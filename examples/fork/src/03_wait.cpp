/* 03_wait.cpp — reaping MANY children: the waitpid(-1) loop.
 *
 * Build:  make 03_wait
 * Run:    ./03_wait
 *
 * Rules for a parent with N children:
 *  - waitpid(pid, ...) reaps ONE specific child.
 *  - waitpid(-1, ...)  reaps ANY one child (whoever exits first).
 *  - Each call reaps exactly ONE child, so loop until you have reaped all N.
 *  - waitpid() returns -1 with errno == ECHILD when no children remain.
 *  - It can also fail with EINTR if a signal arrived: retry the call.
 *
 * This program forks 3 children that exit differently:
 *  child A: exit(0)          — normal success
 *  child B: exit(3)          — normal failure code
 *  child C: killed by signal — abort(), i.e. SIGABRT
 * The parent reaps them in exit order and decodes every status with the
 * WIFEXITED / WEXITSTATUS / WIFSIGNALED macros.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define NCHILDREN 3

static void report(pid_t pid, int status) {
  if (WIFEXITED(status)) {
    printf("parent: child %d exited normally, code=%d\n", (int)pid,
           WEXITSTATUS(status));
  } else if (WIFSIGNALED(status)) {
    printf("parent: child %d killed by signal %d%s\n", (int)pid,
           WTERMSIG(status),
#ifdef WCOREDUMP
           WCOREDUMP(status) ? " (core dumped)" : ""
#else
           ""
#endif
    );
  } else {
    printf("parent: child %d changed state unexpectedly (status=%d)\n",
           (int)pid, status);
  }
  fflush(stdout);
}

int main(void) {
  printf("parent: pid=%d, forking %d children\n", (int)getpid(), NCHILDREN);
  fflush(stdout);

  for (int i = 0; i < NCHILDREN; i++) {
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      return 1;
    }
    if (pid == 0) {
      /* ---- child ---- */
      printf("child %c: pid=%d, sleeping %d s...\n", (char)('A' + i),
             (int)getpid(), i + 1);
      fflush(stdout);
      sleep((unsigned int)(i + 1));
      if (i == 0) {
        _exit(0);
      } else if (i == 1) {
        _exit(3);
      } else {
        abort(); /* dies of SIGABRT so the parent sees WIFSIGNALED */
      }
    }
    /* parent falls through and forks the next child */
  }

  /* ---- parent: reap until ECHILD (no children left) ---- */
  int reaped = 0;
  for (;;) {
    int status = 0;
    pid_t pid = waitpid(-1, &status, 0);
    if (pid < 0) {
      if (errno == EINTR) {
        continue; /* a signal interrupted us — just retry */
      }
      if (errno == ECHILD) {
        break; /* no children left: done */
      }
      perror("waitpid");
      return 1;
    }
    reaped++;
    report(pid, status);
  }

  printf("parent: reaped %d/%d children, no zombies left. Exiting.\n", reaped,
         NCHILDREN);
  return 0;
}
