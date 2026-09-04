# Tutorial: `fork()` and the Life of a Process

POSIX `fork()` (from `<unistd.h>`, declared as `pid_t fork(void);`) is the
system call behind every forking network server you will write in this
course (MP1/MP2 style: accept a connection, fork a child to handle it).
This tutorial walks through what `fork()` does, what happens to the child
afterwards, and the two cleanup duties every parent owes its children:

1. **Reap them** so they don't linger as zombies.
2. **Shut them down** when you get Ctrl+C / SIGTERM so they don't linger
   as orphans holding your port open.

## 0. Setup

```sh
cd examples/fork
make          # builds 01_basic 02_zombie 03_wait 04_sigchld 05_graceful
```

Each example is one self-contained C/C++ file in `src/` with a comment header.
Run them in order: each builds on the previous one.

---

## 1. What `fork()` does (`01_basic`)

```c
pid_t pid = fork();
if (pid < 0) { /* error */ }
else if (pid == 0) { /* child */ }
else { /* parent, pid == child's pid */ }
```

- The child is a near-copy of the parent: same code, same variables
  (with the same *values* — but separate memory afterwards), same open
  file descriptors (sockets included!), same signal dispositions.
- The ONLY first-order differences: the return value of `fork()`, and
  `getpid()` / `getppid()`.
- Both processes resume **after** the `fork()` call. There is no
  "start of the child program" — the child *is* the parent, duplicated.
- Scheduling order is unspecified: never assume child-prints-first.

**Demo:** `./01_basic`. The parent prints its pid, forks, the child
prints both pids and exits `42`, the parent `waitpid()`s and decodes it.

**Gotcha — duplicated stdio buffers.** `printf()` without a newline
sits in a userspace buffer that `fork()` duplicates, so it can print
twice (once per process). Fix: `fflush(stdout);` before `fork()`.
(`01_basic` does this; try deleting that line and see.)

**Gotcha — `_exit()` vs `exit()` in the child.** `exit()` flushes stdio
and runs `atexit()` handlers — which the parent *also* will do, so
buffers flush twice and handlers run twice. Rule of thumb: in a forked
child that does not `exec()`, finish with `_exit()` (or `_Exit()`).

## 2. The lifecycle: running → zombie → reaped (`02_zombie`)

```
 parent calls fork()
       |
  +----+----+
  |         |
alive      alive
  |         |
  |      child calls _exit() / returns from main / dies of signal
  |         |
  |      ZOMBIE (state Z) — kernel holds pid + exit status for parent
  |         |
  |      parent calls wait()/waitpid() — status collected, slot freed
  |         |
  v         v
 ...      gone
```

- A **zombie** is already dead: no memory, no fds, no CPU. Just a
  process-table entry (`STAT Z` in `ps`) waiting for the parent to
  collect the exit status.
- Zombies are the parent's bug, not the child's: only `wait()`-family
  calls in the **parent** remove them.
- If the parent dies first, living children become **orphans** and are
  reparented to init/systemd (pid 1), which reaps them. Dead children of
  a dead parent vanish with it. So zombies only accumulate while a
  long-lived parent (your server!) neglects to wait.
- Runaway zombies exhaust the pid space and confuse graders checking
  `ps`. Always reap.

**Demo:** `./02_zombie` sleeps 15 s with an unreaped dead child. In a
second terminal, run the `ps -o pid,ppid,stat,cmd ...` line it prints:
you will see the child with `STAT Z`. After the sleep the parent reaps
it — run `ps` again and the line is gone. Bonus demo: `kill -9` the
parent mid-sleep and watch the zombie disappear with it.

## 3. Reaping N children: the `waitpid(-1)` loop (`03_wait`)

One `waitpid()` call reaps exactly one child, so a parent with N
children loops:

```c
for (;;) {
    pid_t pid = waitpid(-1, &status, 0);
    if (pid < 0) {
        if (errno == EINTR) continue;   /* signal interrupted us: retry */
        if (errno == ECHILD) break;     /* no children left: done */
        /* real error */
    }
    /* decode status ... */
}
```

- `waitpid(pid, ...)` waits for one specific child; `waitpid(-1, ...)`
  takes whichever child exits first. (Plain `wait(&status)` is just
  `waitpid(-1, &status, 0)`.)
- Decode with macros — never inspect `status` raw:
  `WIFEXITED` / `WEXITSTATUS`, `WIFSIGNALED` / `WTERMSIG`
  (and `WCOREDUMP` where available). `./03_wait` forks three children
  that exit 0, exit 3, and die of `SIGABRT` so you see each case.

## 4. Reaping without blocking: `SIGCHLD` (`04_sigchld`)

A server parent can't block in `waitpid()` — it must stay in its
`accept()` loop. Instead, the kernel sends the parent **`SIGCHLD`**
whenever a child changes state. Install a handler that reaps:

```c
static void on_sigchld(int sig) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) { /* reap all ready children */ }
    errno = saved_errno;
}
```

Handler rules (all demonstrated in `04_sigchld`):

- `WNOHANG`: return 0 at once if nothing has exited — the handler must
  never block.
- Loop: signals don't queue, so one `SIGCHLD` can mean several dead
  children. Reap until `waitpid` returns 0 (none ready) or -1
  (`ECHILD` = none left).
- **Async-signal safety**: inside a handler only call safe functions.
  `waitpid()` and `write()` are safe; `printf()`, `malloc()`,
  `snprintf()` are NOT. So the handler only reaps and bumps a
  `volatile sig_atomic_t` counter; all printing happens in `main()`.
- Save/restore `errno` — the handler can interrupt `main()` mid-syscall.
- Use `sigaction()`, not `signal()` (portable semantics), with
  `SA_RESTART` so slow calls like `accept()`/`sleep()` resume, and
  `SA_NOCLDSTOP` so you only hear about exits, not stops/continues.

**Demo:** `./04_sigchld` spawns 5 staggered children while `main()`
does "server work" and never calls `waitpid()` itself. All 5 are
reaped from the handler — no zombies, no blocking.

Common alternative: `signal(SIGCHLD, SIG_IGN)`. On Linux this auto-reaps
children — but the exit codes are lost, it is not portable, and you
learn nothing. Prefer the handler loop.

## 5. Ctrl+C cleanup: forward, clean up, reap (`05_graceful`)

The capstone. A forking server gets SIGINT (Ctrl+C) or SIGTERM
(`kill <pid>`, systemd stop). Correct shutdown has three steps:

```
 1. Parent catches SIGINT/SIGTERM -> sets shutdown flag
 2. Parent forwards SIGTERM to every worker child (kill)
 3. Each worker's SIGTERM handler sets its flag -> worker runs cleanup
    (close fds, flush logs) -> _exit()
 4. Parent waitpid()s every worker -> no zombies -> parent exits
```

Why forward explicitly?

- Ctrl+C reaches the whole *foreground process group*, but
  `kill <parent-pid>` and service managers signal the parent **only**.
  Forwarding covers both — it's the pattern production code expects.
- Otherwise workers keep running as orphans, still bound to your port;
  your "restart" then fails with `EADDRINUSE` and students blame the OS.
- Alternative: `setpgid()` the children into their own group and
  `kill(-pgid, SIGTERM)` once. Same idea; see also `SO_REUSEADDR` on
  your listening socket so a fast restart works regardless.

Handler discipline (both sides):

- Handlers set a `volatile sig_atomic_t` flag and return. Real work
  happens in the main loop, where everything is safe to call.
- Don't use `SA_RESTART` blindly here: you *want* `sleep()`/`accept()`
  to return early (`EINTR`) so the loop notices the flag promptly.
  (`05_graceful` installs with flags `0` for exactly this reason;
  for `accept()` loops, either check the flag after `EINTR` or use
  `sigsuspend()`/`pselect()`.)
- Children handle `SIGTERM` (and `SIGINT`, since Ctrl+C may reach them
  directly) so both delivery paths end in cleanup, not instant death.
  Note the default disposition of both signals is "terminate without
  cleanup" — installing the handler is what buys the worker time.

**Demo:** `./05_graceful`, then press Ctrl+C (or `kill -TERM` the
parent pid from another terminal). Watch each worker print cleanup,
each reap reported, and `ps` afterwards showing nothing left behind.
Compare with the broken version: comment out the `kill()` loop, retry,
and find orphaned `./05_graceful` workers still ticking in `ps`.

## 6. Cheat sheet: the five syscalls

| Call | Purpose | Notes |
|------|---------|-------|
| `fork()` | create child | check `< 0`; `fflush` first; child ends with `_exit()` |
| `waitpid()` | reap, get status | loop to `ECHILD`; retry on `EINTR`; `WNOHANG` in handlers/loops |
| `sigaction()` | catch `SIGCHLD`/`SIGINT`/`SIGTERM` | handler sets flag only; mind `SA_RESTART` |
| `kill(pid, sig)` | signal a child (parent→child) | `kill(-pgid, sig)` for a whole group; `ESRCH` = already gone |
| `_exit()` | end child | no stdio flush / `atexit` re-run, unlike `exit()` |

Status macros: `WIFEXITED/WEXITSTATUS`, `WIFSIGNALED/WTERMSIG`.

## 7. Exercises (good lab / homework fodder)

1. **Count the processes.** `fork(); fork(); fork(); printf("hi\n");`
   How many lines print? (Answer: 8.) Verify, then explain with a tree.
2. **Make a zombie on purpose**, find it in `ps aux` (STAT `Z`), then
   fix the parent to reap it. (`02_zombie` is the worked solution.)
3. **Blocking vs non-blocking.** Replace the `WNOHANG` handler in
   `04_sigchld` with a blocking `waitpid()` in `main()` and describe
   what stalls.
4. **Unsafe handler.** Call `printf()` inside `on_sigchld`, fork 50 fast
   children, and (maybe, intermittently!) watch output garble — then
   revert. Intermittent is the point: async-signal bugs don't reproduce
   on demand.
5. **Graceful vs rude.** In `05_graceful`, replace the `kill()+waitpid()`
   shutdown with `exit(0)` and observe (via `ps`) the orphaned workers
   and the still-bound port.

## 8. Further reading

- `man 2 fork`, `man 2 waitpid`, `man 2 sigaction`, `man 7 signal`
  (the async-signal-safe list lives here), `man 2 kill`, `man 2 _exit`
