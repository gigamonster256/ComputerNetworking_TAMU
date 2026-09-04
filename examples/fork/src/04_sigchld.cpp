/* 04_sigchld.cpp — reaping WITHOUT blocking: the SIGCHLD handler.
 *
 * Build:  make 04_sigchld
 * Run:    ./04_sigchld
 *
 * Problem: a server parent cannot sit in waitpid() — it has real work to do
 * (accept() loop, etc.). But if it never waits, children become zombies (02).
 *
 * Solution: the kernel sends the parent SIGCHLD every time a child changes
 * state. Install a handler that reaps with waitpid(-1, ..., WNOHANG):
 *  - WNOHANG makes waitpid() return 0 immediately if no child has exited,
 *    so the handler never blocks.
 *  - Loop the call: several children may have exited before one SIGCHLD
 *    is delivered (signals do not queue), so reap until it returns 0/-1.
 *
 * Handler-safety rules (important!):
 *  - A signal handler may only call async-signal-safe functions. printf(),
 *    malloc(), snprintf() are NOT safe. waitpid() and write() ARE safe.
 *  - So the handler here only reaps and bumps a sig_atomic_t counter;
 *    all printing happens in main().
 *  - Save/restore errno inside the handler so it does not clobber the
 *    errno of whatever main() was doing when interrupted.
 *  - Use sigaction(), never signal(): portable semantics + SA_RESTART so
 *    slow syscalls (like sleep/accept) resume instead of failing EINTR.
 */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t g_reaped = 0; /* # children reaped so far */

static void on_sigchld(int sig) {
  (void)sig;
  int saved_errno = errno; /* preserve main-line errno */
  for (;;) {
    /* WNOHANG: return 0 at once if nothing has exited; never block. */
    pid_t pid = waitpid(-1, NULL, WNOHANG);
    if (pid <= 0) {
      break; /* 0 = none ready; -1 = none left (ECHILD) or error */
    }
    g_reaped++;
  }
  errno = saved_errno;
}

int main(void) {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = on_sigchld;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags =
      SA_RESTART | SA_NOCLDSTOP; /* restart syscalls; only notify on exit */
  if (sigaction(SIGCHLD, &sa, NULL) < 0) {
    perror("sigaction");
    return 1;
  }

  const int total = 5;
  printf("parent: pid=%d, spawning %d short-lived children\n", (int)getpid(),
         total);
  printf("parent: I will NOT call waitpid() in main — the handler reaps.\n");
  fflush(stdout);

  for (int i = 0; i < total; i++) {
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      return 1;
    }
    if (pid == 0) {
      sleep(1); /* simulate a connection handler doing work */
      _exit(i); /* exit code = child index, ignored here */
    }
    printf("parent: spawned child %d\n", (int)pid);
    fflush(stdout);
    sleep(1); /* stagger exits so the handler fires repeatedly */
  }

  /* Simulate a busy server: do work while the handler reaps in background. */
  while (g_reaped < total) {
    printf("parent: working... (reaped %d/%d)\n", g_reaped, total);
    fflush(stdout);
    sleep(1);
  }

  printf(
      "parent: all %d children reaped, no zombies, never blocked. Exiting.\n",
      total);
  return 0;
}
