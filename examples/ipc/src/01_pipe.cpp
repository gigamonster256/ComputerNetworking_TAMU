/* 01_pipe.cpp — anonymous pipes: the simplest parent<->child channel.
 *
 * Build:  make 01_pipe
 * Run:    ./01_pipe
 *
 * pipe(fds) creates two fds: fds[0] = read end, fds[1] = write end.
 * Bytes written to fds[1] come out of fds[0] in order (a byte stream —
 * NO message boundaries). The pipe buffer is finite (often 64 KiB on
 * Linux); a writer blocks when it is full, a reader blocks when empty.
 *
 * Rules demonstrated here:
 *  1. Create the pipe BEFORE fork() so both processes inherit the fds.
 *     (Hence "anonymous": no name, so only related processes — i.e.
 *     forked children — can use it. For unrelated processes use a FIFO,
 *     mq, shm, or socket; see 02/03/04/06.)
 *  2. Close the ends you don't use in each process. This is not tidiness:
 *     the reader only sees EOF once EVERY write end is closed, so a
 *     parent that forgets to close its copy of fds[1] hangs forever.
 *  3. Writes of <= PIPE_BUF bytes (4096 on Linux) are atomic: they never
 *     interleave with other writers. Larger writes may split.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  int fds[2]; /* fds[0] = read, fds[1] = write */
  if (pipe(fds) < 0) {
    perror("pipe");
    return 1;
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    /* ---- child: writer ---- */
    close(fds[0]); /* we never read: close the read end */
    const char* msgs[] = {"hello", "from", "the child"};
    for (size_t i = 0; i < sizeof(msgs) / sizeof(msgs[0]); i++) {
      size_t len = strlen(msgs[i]) + 1; /* send NUL too: our own framing */
      ssize_t n = write(fds[1], msgs[i], len);
      if (n < 0) {
        perror("write");
        _exit(1);
      }
      printf("child: sent \"%s\"\n", msgs[i]);
      fflush(stdout);
    }
    close(fds[1]); /* reader's read() returns 0 (EOF) only after this */
    _exit(0);
  }

  /* ---- parent: reader ---- */
  close(fds[1]); /* we never write: close the write end (else no EOF!) */

  char buf[128];
  ssize_t n;
  while ((n = read(fds[0], buf, sizeof(buf))) > 0) {
    /* Our protocol: NUL-terminated strings, so print chunk by chunk. */
    for (ssize_t off = 0; off < n;) {
      printf("parent: got \"%s\"\n", buf + off);
      off += (ssize_t)strlen(buf + off) + 1;
    }
    fflush(stdout);
  }
  if (n < 0) {
    perror("read");
    return 1;
  }
  printf("parent: EOF (child closed its write end)\n");
  close(fds[0]);

  int status = 0;
  if (waitpid(pid, &status, 0) < 0) {
    perror("waitpid");
    return 1;
  }
  printf("parent: child exited with code %d\n", WEXITSTATUS(status));
  return 0;
}
