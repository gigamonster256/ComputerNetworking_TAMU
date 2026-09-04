/* 01_basic.cpp — what fork() actually does.
 *
 * Build:  make 01_basic
 * Run:    ./01_basic
 *
 * fork() creates a new process by duplicating the calling process.
 *  - On success it returns TWICE: 0 in the child, the child's pid in the
 * parent.
 *  - On failure it returns -1 (once, in the parent) and sets errno.
 *  - Both processes continue execution at the instruction AFTER fork().
 *
 * Key ideas demonstrated here:
 *  1. Always check for -1 first.
 *  2. Branch on pid == 0 (child) vs pid > 0 (parent).
 *  3. The parent reaps the child with waitpid() (more on that in 02/03).
 *  4. fflush(stdout) before fork: stdio buffers are duplicated too, so any
 *     buffered-but-unflushed output would print TWICE without this.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void decode_status(int status) {
  if (WIFEXITED(status)) {
    printf("parent: child exited normally, code=%d\n", WEXITSTATUS(status));
  } else if (WIFSIGNALED(status)) {
    printf("parent: child killed by signal %d\n", WTERMSIG(status));
  } else {
    printf("parent: child stopped/continued (unexpected here)\n");
  }
}

int main(void) {
  printf("parent: pid=%d, about to fork...\n", (int)getpid());
  fflush(stdout); /* duplicated buffers print twice — flush first! */

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    /* ---- child ---- */
    printf("child:  pid=%d, ppid=%d (parent pid)\n", (int)getpid(),
           (int)getppid());
    printf("child:  doing some work...\n");
    sleep(1);
    printf("child:  done, exiting with code 42\n");
    fflush(stdout);
    _exit(
        42); /* _exit(): no stdio flush / atexit handlers re-run (see README) */
  }

  /* ---- parent ---- */
  printf("parent: forked child pid=%d\n", (int)pid);

  int status = 0;
  if (waitpid(pid, &status, 0) <
      0) { /* 0 = block until THIS child changes state */
    perror("waitpid");
    return 1;
  }
  decode_status(status);
  printf("parent: exiting\n");
  return 0;
}
