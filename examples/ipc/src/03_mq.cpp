/* 03_mq.cpp — POSIX message queues: prioritized datagrams, no framing code.
 *
 * Build:  make 03_mq
 * Run:    ./03_mq
 *
 * mq_open/mq_send/mq_receive give you what pipes don't:
 *  - MESSAGE BOUNDARIES: each mq_receive() returns exactly one message.
 *  - PRIORITIES: higher-priority messages jump the queue (received first).
 *  - Named (/some_name) + O_CREAT: unrelated processes can rendezvous.
 *  - Notification: a process can ask for a signal (or thread) when a
 *    message arrives instead of blocking (see mq_notify, not shown here).
 *
 * Rules demonstrated here:
 *  1. Queue names start with '/' and need no filesystem path.
 *  2. mq_attr sets mq_maxmsg / mq_msgsize at CREATION time (Linux caps
 *     mq_maxmsg, often at 10 — stay at or under it).
 *  3. mq_send(q, buf, len, prio): higher prio number = received first.
 *  4. mq_unlink() when done; unlike pipes, queues PERSIST in the kernel
 *     (/dev/mqueue) after all processes exit, until unlinked.
 */
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MQ_NAME "/ipc_tutorial_mq"
#define MAXMSG 10
#define MSGSIZE 128

int main(void) {
  struct mq_attr attr;
  memset(&attr, 0, sizeof(attr));
  attr.mq_maxmsg = MAXMSG;
  attr.mq_msgsize = MSGSIZE;

  mq_unlink(MQ_NAME); /* drop any stale queue from a crashed run */
  mqd_t mq = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0600, &attr);
  if (mq == (mqd_t)-1) {
    perror("mq_open");
    return 1;
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    mq_close(mq);
    mq_unlink(MQ_NAME);
    return 1;
  }

  if (pid == 0) {
    /* ---- child: sender (low prio first, to prove reordering) ---- */
    struct {
      const char* text;
      unsigned prio;
    } out[] = {{"low-priority job", 1}, {"URGENT job", 9}, {"normal job", 5}};
    for (size_t i = 0; i < sizeof(out) / sizeof(out[0]); i++) {
      if (mq_send(mq, out[i].text, strlen(out[i].text) + 1, out[i].prio) < 0) {
        perror("mq_send");
        mq_close(mq);
        _exit(1);
      }
      printf("child: sent \"%s\" (prio %u)\n", out[i].text, out[i].prio);
      fflush(stdout);
    }
    mq_close(mq);
    _exit(0);
  }

  /* ---- parent: receiver ---- */
  /* Wait for the sender so all 3 messages are queued before we receive:
   * then the arrival order (1,9,5) vs receive order (9,5,1) proves
   * priority reordering. */
  int status = 0;
  if (waitpid(pid, &status, 0) < 0) {
    perror("waitpid");
    mq_close(mq);
    mq_unlink(MQ_NAME);
    return 1;
  }

  char buf[MSGSIZE];
  for (int i = 0; i < 3; i++) {
    unsigned prio = 0;
    ssize_t n = mq_receive(mq, buf, sizeof(buf), &prio);
    if (n < 0) {
      perror("mq_receive");
      mq_close(mq);
      mq_unlink(MQ_NAME);
      return 1;
    }
    printf("parent: received \"%s\" (prio %u)\n", buf, prio);
    fflush(stdout);
  }

  mq_close(mq);
  mq_unlink(MQ_NAME); /* queues outlive processes — always unlink */
  printf("parent: queue removed. Note receive order 9,5,1 != send order.\n");
  return 0;
}
