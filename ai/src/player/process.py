
import os
import signal
import sys

class Process:
    name: str
    pid: int | None
    _func: callable
    _args: list
    _exit_status: int | None

    def __init__(self, name: str, function: callable, args: list):
        self.name = name
        self._func = function
        self._args = args
        self.pid = None
        self._exit_status = None

    def start(self) -> None:
        try:
            pid = os.fork()
        except Exception as e:
            raise RuntimeError(f"fork() failed: {e}.")

        if pid == 0:  # child
            # Reset SIGCHLD to default so grandchildren are reaped normally
            signal.signal(signal.SIGCHLD, signal.SIG_DFL)
            try:
                self._func(*self._args)
            except Exception:
                pass
            sys.exit(0)
        else:  # parent
            self.pid = pid

    def wait(self) -> int:
        if self.pid is None or self.pid == -1:
            raise RuntimeError(f"Invalid pid {self.pid}.")
        if self._exit_status is not None:
            return self._exit_status
        _, status = os.waitpid(self.pid, 0)
        self._exit_status = os.waitstatus_to_exitcode(status)
        return self._exit_status

    def terminate(self) -> None:
        if self.pid:
            try:
                os.kill(self.pid, signal.SIGTERM)
            except OSError:
                pass
# end of Process class
