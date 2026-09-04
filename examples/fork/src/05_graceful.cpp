/* 05_graceful.cpp — Ctrl+C cleanup: parent forwards the signal, children exit
 * cleanly.
 *
 * Build:  make 05_graceful
 * Run:    ./05_graceful        then press Ctrl+C  (or: kill -TERM <parent-pid>)
 *
 * The scenario: a forking server (like MP1/MP2). The parent accepts
 * connections and forks a worker child per connection. Now the operator
 * hits Ctrl+C (SIGINT) or runs `kill` (SIGTERM). Goal: NO orphans holding
 * the port open, NO zombies, workers get a chance to flush/close state.
 *
 * Protocol implemented here:
 *  1. Parent installs SIGINT/SIGTERM handler that only sets a flag
 *     (volatile sig_atomic_t). The main loop notices and starts shutdown.
 *  2. Each worker installs a SIGTERM handler that sets its own flag, so it
 *     can break out of its work loop, run cleanup (close fds, flush logs),
 *     and _exit() with a status.
 *  3. Parent shutdown sequence: kill(child, SIGTERM) for every worker,
 *     then waitpid() each one (reaps -> no zombies), then exit itself.
 *
 * Why forward explicitly instead of relying on Ctrl+C reaching everyone?
 *  - Ctrl+C DOES go to the whole foreground process group, but `kill <pid>`
 *    and systemd stop send SIGTERM to the parent ONLY. Forwarding covers
 *    both cases, and is the pattern graders/production code expect.
 *  - Alternative: put children in their own process group (setpgid) and
 *    kill(-pgid, SIGTERM). Same idea, one syscall. See README section 6.
 *
 * Demo output: press Ctrl+C and watch each worker print cleanup + parent
 * report each reap. Afterwards `ps` shows nothing left behind.
 */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define NWORKERS 2

static volatile sig_atomic_t g_shutdown =
    0;                                   /* set by parent's INT/TERM handler */
static volatile sig_atomic_t g_stop = 0; /* set by worker's TERM handler */

static void parent_on_term(int sig) {
  (void)sig;
  g_shutdown = 1;
}

static void worker_on_term(int sig) {
  (void)sig;
  g_stop = 1;
}

static void install(int signum, void (*fn)(int)) {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = fn;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags =
      0; /* no SA_RESTART: sleep() returns early so loops notice flags fast */
  if (sigaction(signum, &sa, NULL) < 0) {
    perror("sigaction");
    exit(1);
  }
}

/* Simulated connection-worker: loop until told to stop, then clean up. */
static void worker_main(int id) {
  install(SIGTERM, worker_on_term);
  install(SIGINT, worker_on_term); /* Ctrl+C reaches our group too; exit cleanly
                                      either way */

  printf("worker %d: pid=%d started (parent %d)\n", id, (int)getpid(),
         (int)getppid());
  fflush(stdout);

  int ticks = 0;
  while (!g_stop) {
    printf("worker %d: handling fake connection (tick %d)\n", id, ++ticks);
    fflush(stdout);
    sleep(2); /* stand-in for blocking recv()/send() work */
  }

  /* ---- cleanup section: close fds, flush logs, release locks ---- */
  printf(
      "worker %d: got shutdown signal, cleaning up (closing fds, "
      "flushing)...\n",
      id);
  fflush(stdout);
  sleep(1); /* pretend cleanup takes a moment */
  printf("worker %d: cleanup done, exiting.\n", id);
  fflush(stdout);
  _exit(0);
}

int main(void) {
  install(SIGINT, parent_on_term);  /* Ctrl+C */
  install(SIGTERM, parent_on_term); /* kill <pid>, systemd stop */

  pid_t workers[NWORKERS];

  printf("parent: pid=%d starting %d workers. Press Ctrl+C to stop me.\n",
         (int)getpid(), NWORKERS);
  fflush(stdout);

  for (int i = 0; i < NWORKERS; i++) {
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      return 1;
    }
    if (pid == 0) {
      worker_main(i); /* never returns */
    }
    workers[i] = pid;
  }

  /* Parent's "server loop": wait for the shutdown flag. */
  while (!g_shutdown) {
    /* Reap any worker that dies on its own (crash) so it never zombifies.
     * WNOHANG => never blocks the server loop. */
    pid_t pid = waitpid(-1, NULL, WNOHANG);
    if (pid > 0) {
      printf("parent: worker %d exited early (will not restart it here)\n",
             (int)pid);
      fflush(stdout);
      for (int i = 0; i < NWORKERS; i++) {
        if (workers[i] == pid) {
          workers[i] = -1;
        }
      }
    }
    sleep(1);
  }

  printf("\nparent: shutdown requested, signalling %d workers...\n", NWORKERS);
  fflush(stdout);

  for (int i = 0; i < NWORKERS; i++) {
    if (workers[i] > 0) {
      if (kill(workers[i], SIGTERM) < 0 && errno != ESRCH) {
        perror("kill");
      }
    }
  }

  /* Reap every worker (blocks until each exits) => no zombies. */
  for (int i = 0; i < NWORKERS; i++) {
    if (workers[i] <= 0) {
      continue;
    }
    int status = 0;
    for (;;) {
      pid_t got = waitpid(workers[i], &status, 0);
      if (got < 0) {
        if (errno == EINTR) {
          continue;
        }
        if (errno == ECHILD) {
          break; /* already reaped in the loop above */
        }
        perror("waitpid");
        break;
      }
      if (WIFEXITED(status)) {
        printf("parent: worker %d reaped, exit code %d\n", (int)got,
               WEXITSTATUS(status));
      } else if (WIFSIGNALED(status)) {
        printf("parent: worker %d reaped, killed by signal %d\n", (int)got,
               WTERMSIG(status));
      }
      fflush(stdout);
      break;
    }
  }

  printf("parent: all workers gone, no zombies, no orphans. Goodbye.\n");
  return 0;
}
