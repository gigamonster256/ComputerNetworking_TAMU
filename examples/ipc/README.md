# Tutorial: POSIX IPC — Pipes, FIFOs, Message Queues, Shared Memory, Semaphores

The `examples/fork/` tutorial taught you how processes come to life and how they
die. This one answers the next question: **once you have two processes,
how do they talk?** Every example is a self-contained C++ file in `src/`
using `fork()` (so it runs with one command), but each mechanism's
"who can talk" notes explain the unrelated-processes case too.

## 0. Setup

```sh
cd examples/ipc
make          # builds 01_pipe 02_fifo 03_mq 04_shm 05_sem 06_socketpair
```

Suggested 50-minute tutorial plan:

| Time | Do | File |
|------|----|------|
| 0–8 | Pipes: inherit, close unused ends, EOF, `PIPE_BUF` | `01_pipe` |
| 8–15 | FIFOs: named pipes, blocking-open rendezvous, `unlink` | `02_fifo` |
| 15–25 | Message queues: boundaries, priorities, persistence | `03_mq` |
| 25–35 | Shared memory: zero-copy speed, zero safety | `04_shm` |
| 35–45 | Semaphores: mutex-protect the counter, break it on purpose | `05_sem` |
| 45–50 | socketpair: full-duplex + the bridge to networking | `06_socketpair` |

---

## 1. The big picture: what varies between mechanisms

Pick IPC by answering four questions:

1. **Who talks?** Related (forked) processes only, or unrelated ones too?
2. **What shape is the data?** Byte stream (you frame it) vs messages
   (kernel preserves boundaries) vs raw memory (you build everything).
3. **Who blocks when?** Empty/full behavior, priorities, notification.
4. **Who cleans up?** Some objects vanish with the process; some
   **persist in the kernel** until unlinked — leak those and the next
   run (or the grader) trips over them.

| Mechanism | Who can talk | Data shape | Direction | Kernel persists? | Example |
|-----------|--------------|------------|-----------|------------------|---------|
| Anonymous pipe | related only (inherited fds) | byte stream | one-way (use 2 for dialogue) | no | `01_pipe` |
| FIFO (named pipe) | anyone knowing the path | byte stream | one-way | yes (filesystem node) | `02_fifo` |
| POSIX message queue | anyone knowing the `/name` | messages + priorities | both ways, N↔M | yes (`/dev/mqueue`) | `03_mq` |
| POSIX shared memory | anyone knowing the `/name` | raw memory | both ways, N↔M | yes (kernel object) | `04_shm` |
| Semaphore (on its own) | anyone knowing the `/name` | a counter (signaling only) | N/A | yes (kernel object) | `05_sem` |
| Unix socketpair | related only (like pipe) | stream or datagram | **full-duplex** | no | `06_socketpair` |

(Unix-domain *named* sockets — `bind()` a path — lift socketpair's
"related only" limit; that plus fd-passing is why they're the "etc"
worth knowing. See section 7.)

---

## 2. Anonymous pipes (`01_pipe`)

`pipe(fds)` → `fds[0]` reads what `fds[1]` is written, in order.

Strengths:

- Simplest thing that works: two fds, no names, no cleanup, gone when
  closed. Perfect for parent↔child (`prog1 | prog2` is literally this).
- Flow control is free: readers block when empty, writers block when
  full — no polling loops.
- Writes ≤ `PIPE_BUF` (4096 on Linux) are atomic across writers.

Weaknesses:

- **Related processes only** — fds are inherited across `fork()`, so
  strangers can't join. (FIFOs/mq/shm fix this with names.)
- **One direction.** A dialogue needs two pipes; a chat room needs many.
- **Byte stream, no boundaries.** `read()` may return half your struct —
  you supply framing (length prefixes, delimiters) and partial-read
  loops, exactly like TCP. Good practice for this course, extra work
  for quick scripts.
- Finite buffer: two processes writing to *each other* on full pipes
  without reading deadlock. (`01_pipe`'s strict writer→reader protocol
  sidesteps this.)

Key lines: create **before** `fork()`; **close unused ends** in each
process — the reader gets EOF only after *every* write end closes, and
the classic hang is a parent still holding `fds[1]`.

## 3. FIFOs / named pipes (`02_fifo`)

Same bytes-in-bytes-out semantics as a pipe, plus a filesystem path
(`mkfifo()`), so **unrelated processes** can meet there.

Strengths:

- Everything pipes give you, minus the fork requirement. Great for
  shell pipelines across terminals and quick demos
  (`mkfifo f; cat f` in one terminal, `echo hi > f` in another).
- Still auto-clean in the data plane: no messages pile up anywhere.

Weaknesses:

- **The blocking-open rendezvous**: `open(O_RDONLY)` waits for a writer
  and `open(O_WRONLY)` waits for a reader. Open the ends in the wrong
  combination and both sides sleep forever — the #1 FIFO debugging
  session. (`O_RDWR` or `O_NONBLOCK` dodge it, with their own caveats.)
- Still a byte stream, still one direction per FIFO.
- **Cleanup is manual**: the path node survives crashes — `unlink()` it
  (all examples here unlink first *and* last for exactly this reason).
- Filesystem permission/path management that anonymous pipes never need.

## 4. POSIX message queues (`03_mq`)

`mq_open/mq_send/mq_receive`: a kernel-held queue of discrete messages,
each with a priority. Highest priority is received first, regardless of
send order (`03_mq` sends prio 1, 9, 5 and receives 9, 5, 1).

Strengths:

- **Message boundaries preserved** — one receive = one message. All the
  framing/partial-read pain of pipes disappears.
- **Priorities built in** — urgent control messages jump data messages
  with zero application code.
- Named (`/name`) → unrelated processes; many senders
  and receivers can share one queue.
- `mq_notify()` lets a process request a signal/thread on arrival
  instead of blocking — handy for event loops.

Weaknesses:

- **Persist until `mq_unlink()`** — visible under `/dev/mqueue`, with
  system caps (`/proc/sys/fs/mqueue/msg_max`, often 10). Forgetting
  cleanup breaks the *next* run, not this one — the nastiest kind of bug
  in autograded assignments.
- Fixed max message size/count set at creation; big payloads don't fit
  (send a pointer-via-shm or a filename instead — the standard hybrid).
- Lower throughput than shared memory (every byte crosses the kernel).
- Portability footnote: POSIX but historically patchy on macOS; on
  Linux needs nothing special these days.

## 5. Shared memory (`04_shm`)

`shm_open() + ftruncate() + mmap(MAP_SHARED)`: the same physical pages
in two address spaces. A store is instantly visible — no syscalls, no
copies. Fastest IPC there is.

Strengths:

- **Throughput**: pointer-speed data sharing; the only sane way to move
  megabytes between processes (frame buffers, ring buffers, datasets).
- Flexible: build any structure you like on top (arrays, queues, hash
  tables).

Weaknesses (all of them are *"you* build it"):

- **No synchronization** — concurrent read-modify-write loses updates
  (`05_sem` demos exactly this). Pair with a semaphore or a mutex
  placed in the segment (`PTHREAD_PROCESS_SHARED`).
- **No signaling** — "is the data ready?" needs a flag + semaphore, a
  companion pipe/socket, or `futex`es. `waitpid()` as a ready-signal
  (as in `04_shm`) only works for one-shot parent/child patterns.
- **No boundaries, no blocking** — it is memory, not a channel.
- **Persists until `shm_unlink()`** — stale segments squat in `/dev/shm`
  and (worse) a re-run can map yesterday's bytes. Unlink first and last.
- Same-machine only, always — and pointers inside the segment are only
  valid if both sides map it at compatible addresses (offsets, not
  absolute pointers, in portable code).

## 6. Semaphores (`05_sem`)

`sem_open/sem_wait/sem_post`: a kernel counter. `wait` = "decrement or
sleep until I can"; `post` = "increment and wake someone". Starting at 1
it is a **mutex**; starting at 0 it is a **signal flag** ("wait until
the data is ready" — the missing piece from section 5).

Strengths:

- Make shared memory (or any shared resource) **correct** with three
  lines: wait → touch → post. Also rate-limiting (start at N for N
  slots) and event signaling across unrelated processes via names.
- Small, fast, well-understood semantics.

Weaknesses:

- **They protect nothing by themselves** — safety comes from *every*
  accessor agreeing to lock. One rogue path that skips `sem_wait`
  corrupts data with no error and no warning.
- Deadlock is one forgotten `sem_post` (or one crash inside the
  critical section) away — prefer short sections, and know about
  `SEM_UNDO`-style (SysV) or robust-mutex recovery for production code.
- **Persist until `sem_unlink()`**, same stale-object trap as mq/shm
  (a leftover value-0 semaphore deadlocks your next run instantly —
  `05_sem` uses `O_EXCL` so that fails loudly instead).
- Debugging races is miserable: they vanish under debuggers and
  `printf`s. Keep critical sections tiny and contention low.

`05_sem` runs 4 children × 25000 mutex-guarded increments and asserts
100000. Break it on purpose (comment out wait/post) to watch the count
come up short — the cheapest possible race-condition demo.

## 7. socketpair, and the bridge to networking (`06_socketpair`)

`socketpair(AF_UNIX, SOCK_STREAM, 0, sv)`: like a pipe, but **one pair
carries both directions**. Parent keeps `sv[0]`, child keeps `sv[1]`,
and they dialogue (`ping`/`pong` in `06_socketpair`).

Strengths:

- Full-duplex over one fd pair; `SOCK_DGRAM` variant adds message
  boundaries; same `read/write/send/recv/select/poll/epoll` API as
  network sockets — patterns transfer 1:1 to MP1–MP4.
- Superpowers: pass file descriptors between processes (`SCM_RIGHTS` —
  e.g. hand an `accept()`ed client socket to a worker), pass
  credentials, and (with *named* Unix sockets) talk between unrelated
  processes over the filesystem.
- No kernel-persistent objects for the anonymous form; nothing to unlink.

Weaknesses:

- Anonymous form is related-processes-only (bind a path for strangers).
- Still kernel-mediated copies (slower than shm for bulk data) and still
  stream-mode framing work for `SOCK_STREAM`.
- `SCM_RIGHTS` fd-passing is powerful but fiddly ancillary-data code —
  worth it for preforked servers, overkill elsewhere.

**Course advice:** when a later MP wants worker processes handling client
sockets, reach for `socketpair` + `SCM_RIGHTS` (or just `fork()` with
the connected fd inherited). When it wants throughput, `shm` + `sem`.
When it wants control messages with priorities, `mq`. Default to pipes
for linear parent↔child data flow.

## 8. Cheat sheet

| Need | Reach for | Remember |
|------|-----------|----------|
| Parent↔child stream | pipe | close unused ends or EOF never comes |
| Unrelated-process stream | FIFO | blocking-open rendezvous; `unlink` |
| Prioritized control messages | POSIX mq | boundaries free; `mq_unlink`; mind caps |
| Bulk data, max speed | POSIX shm | add sem + ready-signal yourself; `shm_unlink` |
| Mutual exclusion / signaling | POSIX sem | everyone must lock; `sem_unlink`; short sections |
| Bidirectional + socket API | socketpair | one pair, both ways; `SCM_RIGHTS` for fds |

Universal rules: unlink named objects first *and* last (crash-safe reruns);
`fork()`-inherited fds are the mechanism behind pipe/socketpair sharing;
status/errno discipline from the `examples/fork/` tutorial applies everywhere here.

## 9. A note on System V vs POSIX IPC

You'll see two families in `man` pages: System V (`msgget/semget/
shmget`, numeric keys via `ftok`) and POSIX (`mq_open/sem_open/
shm_open`, `/names`). They solve the same problems. Prefer **POSIX**:
name-based rendezvous, fd integration (`select/poll` on mq descriptors),
and saner cleanup. Know System V exists because legacy code (and some
textbooks) still use it.

## 10. Exercises

1. **The hanging pipe.** Delete one `close(fds[1])` in `01_pipe` and
   predict the hang; then kill it and explain which `read` never got EOF.
2. **FIFO rendezvous.** Swap the open order in `02_fifo` (reader opens
   twice before any writer) and observe the deadlock; fix with `O_NONBLOCK`
   + `poll()`.
3. **Priority inversion.** Send 10 messages at mixed priorities through
   `03_mq` and log receive order. What happens to a steady stream of
   prio-1 messages while prio-9s keep arriving? (Starvation — how would
   you bound it?)
4. **Race hunting.** Strip the semaphore from `05_sem`, run 5 times,
   record the spread. Re-add it. Explain why `(*counter)++` is three
   machine steps and which interleaving loses an update.
5. **Ready signaling.** Replace `waitpid`-as-signal in `04_shm` with a
   value-0 semaphore the child posts after writing (parent waits before
   reading). Now make it a loop of 100 updates — what else must change
   (hint: the parent must tell the child the read is done)?
6. **fd passing.** Extend `06_socketpair` to send an open file descriptor
   with `SCM_RIGHTS` (`man 7 unix`, `CMSG_*` macros) — the vērified
   preforked-server pattern.

## 11. Further reading

- `man 7 pipe`, `man 7 fifo`, `man 7 mq_overview`, `man 7 shm_overview`,
  `man 7 sem_overview`, `man 7 unix`, `man 2 socketpair`
- Stevens, Fenner & Rudoff, *UNIX Network Programming*, Vol. 2 —
  the IPC volume (pipes, FIFOs, mq, semaphores, shm, doors).
- Kerrisk, *The Linux Programming Interface*, Ch. 44–55.
