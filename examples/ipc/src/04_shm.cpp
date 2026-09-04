/* 04_shm.cpp — POSIX shared memory: the fastest IPC (plain memory access).
 *
 * Build:  make 04_shm
 * Run:    ./04_shm
 *
 * shm_open() + ftruncate() + mmap(..., MAP_SHARED, ...) maps the SAME
 * physical pages into two processes: a store by one is instantly visible
 * to the other. No read()/write() syscalls per message, no kernel copy —
 * just pointer dereference. That is why shm wins every throughput race.
 *
 * The price (see 05_sem for the fix):
 *  - NO synchronization. Two writers (or reader-vs-writer) race unless
 *    YOU add it (semaphore, mutex in shared memory, ...).
 *  - NO message boundaries, NO blocking/wakeup: you must build your own
 *    signaling (a "data ready" flag + sem, a pipe alongside, ...).
 *  - Lifetime: like mqueues, the object persists until shm_unlink().
 *
 * This demo uses ONE writer + waitpid() as the (crude) synchronization so
 * it stays deterministic: child writes a struct, parent waits, then reads.
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHM_NAME "/ipc_tutorial_shm"

struct payload {
  int id;
  char text[64];
};

int main(void) {
  shm_unlink(SHM_NAME); /* drop any stale object from a crashed run */
  int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
  if (fd < 0) {
    perror("shm_open");
    return 1;
  }
  if (ftruncate(fd, sizeof(struct payload)) < 0) { /* set object size */
    perror("ftruncate");
    close(fd);
    shm_unlink(SHM_NAME);
    return 1;
  }

  struct payload* p = static_cast<struct payload*>(
      mmap(NULL, sizeof(*p), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
  if (p == MAP_FAILED) {
    perror("mmap");
    close(fd);
    shm_unlink(SHM_NAME);
    return 1;
  }
  close(fd); /* mapping holds its own reference; fd no longer needed */

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    munmap(p, sizeof(*p));
    shm_unlink(SHM_NAME);
    return 1;
  }

  if (pid == 0) {
    /* ---- child: writer ---- */
    /* (Both processes map the same pages — writes are instantly visible.) */
    p->id = 42;
    snprintf(p->text, sizeof(p->text), "hello from shared memory");
    printf("child: wrote {id=%d, text=\"%s\"} at %p\n", p->id, p->text,
           (void*)p);
    fflush(stdout);
    munmap(p, sizeof(*p));
    _exit(0);
  }

  /* ---- parent: reader (waitpid = our "is it ready yet?" signal) ---- */
  int status = 0;
  if (waitpid(pid, &status, 0) < 0) {
    perror("waitpid");
    munmap(p, sizeof(*p));
    shm_unlink(SHM_NAME);
    return 1;
  }
  printf("parent: read  {id=%d, text=\"%s\"} at %p\n", p->id, p->text,
         (void*)p);
  printf(
      "parent: same address, same physical page, zero syscalls to move "
      "the data.\n");

  munmap(p, sizeof(*p));
  shm_unlink(SHM_NAME); /* shm objects outlive processes — always unlink */
  printf("parent: shm object removed\n");
  return 0;
}
