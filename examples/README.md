# Examples: POSIX Process & IPC Tutorials

Self-contained C++17 tutorials for ECEN 602, each with runnable examples
and a TA-style walkthrough. Build any folder with `make`.

| Folder | Topic |
|--------|-------|
| [`fork/`](fork/README.md) | `fork()` and the life of a process: return-value split, zombies and reaping (`waitpid`, `SIGCHLD`), graceful Ctrl+C shutdown |
| [`ipc/`](ipc/README.md) | POSIX IPC: pipes, FIFOs, message queues, shared memory, semaphores, socketpair — with strengths/weaknesses of each |

Suggested order: `fork/` first (processes must exist before they can
talk), then `ipc/`.
