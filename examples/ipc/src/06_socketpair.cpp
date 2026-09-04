/* 06_socketpair.cpp — Unix-domain socketpair: bidirectional pipes + networking.
 *
 * Build:  make 06_socketpair
 * Run:    ./06_socketpair
 *
 * socketpair(AF_UNIX, SOCK_STREAM, 0, sv) creates two CONNECTED sockets:
 * sv[0] and sv[1]. Anything written to one is readable from the other —
 * in BOTH directions over the SAME pair (full-duplex). Compare:
 *  - pipe(): one direction only (need 2 pipes for a dialogue).
 *  - socketpair: dialogue over one pair. After fork(), parent keeps sv[0]
 *    and closes sv[1]; child keeps sv[1] and closes sv[0].
 *
 * Why this matters for THIS course: these are real sockets — the same
 * send()/recv() API as your TCP code — so every framing, partial-read,
 * and shutdown pattern transfers directly to MP1–MP4. Bonus powers pipes
 * lack: SOCK_DGRAM mode for message boundaries, SCM_RIGHTS to PASS FILE
 * DESCRIPTORS between processes (e.g. hand an accepted client socket to
 * a worker), and getpeerid-style credential passing.
 *
 * Demo: parent and child ping/pong three rounds over one pair.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define NROUNDS 3

/* Send a NUL-terminated message; loop since stream sockets may partial-write.
 */
static int send_msg(int fd, const char* msg) {
  size_t left = strlen(msg) + 1;
  const char* p = msg;
  while (left > 0) {
    ssize_t n = write(fd, p, left);
    if (n <= 0) {
      return -1;
    }
    p += n;
    left -= (size_t)n;
  }
  return 0;
}

int main(void) {
  int sv[2]; /* sv[0]: parent's end, sv[1]: child's end */
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
    perror("socketpair");
    return 1;
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    /* ---- child ---- */
    close(sv[0]);
    char buf[128];
    for (int i = 0; i < NROUNDS; i++) {
      ssize_t n = read(sv[1], buf, sizeof(buf)); /* wait for ping */
      if (n <= 0) {
        perror("child read");
        close(sv[1]);
        _exit(1);
      }
      printf("child: got \"%s\", replying...\n", buf);
      fflush(stdout);
      char reply[128];
      snprintf(reply, sizeof(reply), "pong %d", i);
      if (send_msg(sv[1], reply) < 0) {
        perror("child write");
        close(sv[1]);
        _exit(1);
      }
    }
    close(sv[1]);
    _exit(0);
  }

  /* ---- parent ---- */
  close(sv[1]);
  char buf[128];
  for (int i = 0; i < NROUNDS; i++) {
    char ping[128];
    snprintf(ping, sizeof(ping), "ping %d", i);
    if (send_msg(sv[0], ping) < 0) {
      perror("parent write");
      return 1;
    }
    ssize_t n = read(sv[0], buf, sizeof(buf)); /* wait for pong */
    if (n <= 0) {
      perror("parent read");
      return 1;
    }
    printf("parent: got \"%s\"\n", buf);
    fflush(stdout);
  }
  close(sv[0]);

  int status = 0;
  if (waitpid(pid, &status, 0) < 0) {
    perror("waitpid");
    return 1;
  }
  printf(
      "parent: dialogue done over ONE fd pair (a pipe would need two). "
      "Child exit %d.\n",
      WEXITSTATUS(status));
  return 0;
}
