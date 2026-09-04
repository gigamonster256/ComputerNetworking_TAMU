/* 05_sem.cpp — POSIX named semaphores: making shared memory CORRECT.
 *
 * Build:  make 05_sem
 * Run:    ./05_sem
 *
 * 04_shm showed shared memory with one writer. With N writers doing
 * counter++ (read-modify-write), increments interleave and updates get
 * LOST — try it: comment out the sem_wait/sem_post below and the final
 * count will come up SHORT (nondeterministically).
 *
 * A semaphore is just a kernel counter with two atomic ops:
 *  - sem_wait(): decrement; BLOCK if already 0 until someone posts.
 *  - sem_post(): increment; wake one waiter.
 * A semaphore starting at 1 is a MUTEX: whoever holds it (between wait
 * and post) owns the critical section. Named (sem_open("/name", ...)) so
 * unrelated processes can share it; it persists until sem_unlink().
 *
 * Demo: 4 children x 25000 increments of a shm counter, mutex-protected.
 * Expected final value: 100000. Without the semaphore it won't be.
 */
#include <fcntl.h>
#include <sched.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHM_NAME "/ipc_tutorial_sem_shm"
#define SEM_NAME "/ipc_tutorial_sem"
#define NCHILD 4
#define NINCR 25000

int main(void) {
  shm_unlink(SHM_NAME);
  sem_unlink(SEM_NAME);
  int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
  if (fd < 0) {
    perror("shm_open");
    return 1;
  }
  if (ftruncate(fd, sizeof(int)) < 0) {
    perror("ftruncate");
    close(fd);
    shm_unlink(SHM_NAME);
    return 1;
  }
  /* volatile: force a real load + store EVERY iteration. Without it,
   * -O2 folds the whole loop into a single `*counter += NINCR` and the
   * race this program demonstrates silently disappears. Note volatile
   * does NOT make ++ atomic — that missing atomicity IS the demo. */
  volatile int* counter = static_cast<volatile int*>(
      mmap(NULL, sizeof(*counter), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
  if (counter == MAP_FAILED) {
    perror("mmap");
    close(fd);
    shm_unlink(SHM_NAME);
    return 1;
  }
  close(fd);
  *counter = 0;

  /* Named semaphore, initial value 1 = mutex. O_EXCL so a stale sem from
   * a crashed run fails loudly instead of silently reusing value 0. */
  sem_t* mutex = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0600, 1);
  if (mutex == SEM_FAILED) {
    perror("sem_open");
    munmap(const_cast<int*>(counter), sizeof(*counter));
    shm_unlink(SHM_NAME);
    return 1;
  }

  for (int i = 0; i < NCHILD; i++) {
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      return 1;
    }
    if (pid == 0) {
      for (int k = 0; k < NINCR; k++) {
        if (sem_wait(mutex) < 0) { /* lock ... */
          perror("sem_wait");
          _exit(1);
        }
        /* ... critical section: load / modify / store spelled out, with a
         * yield in the middle. The yield widens the race window on purpose
         * so the lost-update bug shows up nearly every run once the lock
         * is removed (exercise 4). Without it the bug hides 9 runs in 10
         * — which is itself the lesson about heisenbugs. */
        int tmp = *counter;
        sched_yield();
        *counter = tmp + 1;
        if (sem_post(mutex) < 0) { /* ... unlock */
          perror("sem_post");
          _exit(1);
        }
      }
      munmap(const_cast<int*>(counter), sizeof(*counter));
      sem_close(mutex);
      _exit(0);
    }
  }

  int status = 0;
  pid_t w;
  while ((w = waitpid(-1, &status, 0)) > 0) {
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
      printf("parent: child %d failed!\n", (int)w);
      fflush(stdout);
    }
  }

  int expected = NCHILD * NINCR;
  int final = *counter;
  printf("parent: counter=%d, expected=%d -> %s\n", final, expected,
         final == expected ? "CORRECT (mutex worked)"
                           : "RACE (lost "
                             "updates!)");
  fflush(stdout);

  munmap(const_cast<int*>(counter), sizeof(*counter));
  sem_close(mutex);
  shm_unlink(SHM_NAME);
  sem_unlink(SEM_NAME); /* named sems persist — always unlink */
  return final == expected ? 0 : 1;
}
