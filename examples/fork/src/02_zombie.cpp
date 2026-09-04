/* 02_zombie.cpp — what a zombie is, and why you must reap your children.
 *
 * Build:  make 02_zombie
 * Run:    ./02_zombie        (then in a SECOND terminal, run the ps command it
 * prints)
 *
 * Lifecycle: running -> terminated-but-unreaped ("zombie", state Z) -> reaped.
 *
 * When a child exits, the kernel keeps its exit status around until the parent
 * calls wait()/waitpid(). A dead-but-unreaped child is a ZOMBIE: it holds no
 * memory or open files, just a pid + exit-status slot in the process table.
 * If the parent NEVER reaps, zombies pile up and pids can eventually run out.
 *
 * This program:
 *  1. Forks a child that exits immediately.
 *  2. Parent sleeps 15 s WITHOUT waiting  -> child is a zombie during this
 * window. Run the printed `ps` command in another terminal to see STAT "Z".
 *  3. Parent then calls waitpid()       -> zombie is reaped, slot freed.
 *
 * Try also: kill -9 the parent during the sleep. The child (already dead) is
 * reaped by init/systemd, and an orphaned RUNNING child would be reparented
 * to init. Dead children of a dead parent never linger.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
  printf("parent: pid=%d\n", (int)getpid());
  fflush(stdout);

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    printf("child:  pid=%d exiting immediately...\n", (int)getpid());
    fflush(stdout);
    _exit(0);
  }

  printf("parent: child pid=%d has exited, but I will NOT wait yet.\n",
         (int)pid);
  printf("parent: >>> in another terminal, run:\n");
  printf("parent: >>>   ps -o pid,ppid,stat,cmd -p %d -p %d\n", (int)pid,
         (int)getpid());
  printf("parent: >>> you should see STAT 'Z' (zombie) on the child.\n");
  fflush(stdout);

  sleep(15); /* window during which the child is a zombie */

  printf("parent: now reaping child %d with waitpid()...\n", (int)pid);
  fflush(stdout);

  int status = 0;
  if (waitpid(pid, &status, 0) < 0) {
    perror("waitpid");
    return 1;
  }
  printf("parent: reaped. Child exited with code %d; zombie is gone.\n",
         WEXITSTATUS(status));
  printf("parent: run the ps command again — the child line has vanished.\n");
  return 0;
}
