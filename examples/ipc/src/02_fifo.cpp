/* 02_fifo.cpp — named pipes (FIFOs): pipes for UNRELATED processes.
 *
 * Build:  make 02_fifo
 * Run:    ./02_fifo
 *
 * A FIFO looks like a file (mkfifo(path, mode)) but behaves like a pipe:
 * bytes in -> bytes out, in order, byte stream, blocking when empty/full.
 * Because it has a NAME in the filesystem, any two processes that know the
 * path can talk — no fork() relationship needed. Try it by hand:
 *
 *   Terminal 1:  mkfifo /tmp/demo_fifo && cat /tmp/demo_fifo
 *   Terminal 2:  echo hello > /tmp/demo_fifo
 *
 * The rendezvous rule (the #1 FIFO gotcha), demonstrated below:
 *  - open(path, O_RDONLY) BLOCKS until some process opens for writing.
 *  - open(path, O_WRONLY) BLOCKS until some process opens for reading.
 * Here parent and child open concurrently, so they unblock each other.
 * Break this on purpose (open the read end twice, or open O_WRONLY with
 * no reader) and watch your program hang — then remember this moment.
 *
 * Cleanup: unlink() the path when done, or it lingers in /tmp forever.
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/ipc_tutorial_fifo"

int main(void) {
  unlink(FIFO_PATH); /* drop any stale fifo from a previous crashed run */
  if (mkfifo(FIFO_PATH, 0600) < 0) {
    perror("mkfifo");
    return 1;
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    unlink(FIFO_PATH);
    return 1;
  }

  if (pid == 0) {
    /* ---- child: writer ---- */
    /* Blocks until the parent opens the read end. */
    int wfd = open(FIFO_PATH, O_WRONLY);
    if (wfd < 0) {
      perror("open O_WRONLY");
      _exit(1);
    }
    const char* msg = "hello through the fifo";
    ssize_t n = write(wfd, msg, strlen(msg) + 1);
    if (n < 0) {
      perror("write");
      close(wfd);
      _exit(1);
    }
    printf("child: wrote \"%s\"\n", msg);
    fflush(stdout);
    close(wfd);
    _exit(0);
  }

  /* ---- parent: reader ---- */
  /* Blocks until the child opens the write end. */
  int rfd = open(FIFO_PATH, O_RDONLY);
  if (rfd < 0) {
    perror("open O_RDONLY");
    return 1;
  }
  char buf[128];
  ssize_t n = read(rfd, buf, sizeof(buf));
  if (n < 0) {
    perror("read");
    close(rfd);
    return 1;
  }
  printf("parent: read \"%s\" (%zd bytes)\n", buf, n);
  close(rfd);

  int status = 0;
  if (waitpid(pid, &status, 0) < 0) {
    perror("waitpid");
    unlink(FIFO_PATH);
    return 1;
  }
  printf("parent: child exited with code %d\n", WEXITSTATUS(status));

  unlink(FIFO_PATH); /* FIFOs persist until unlinked — don't litter /tmp */
  printf("parent: fifo removed\n");
  return 0;
}
