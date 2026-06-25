
import signal
import sys

from parser.args import parse_args
from player.process import Process
from player.zappyAI import ZappyAI, _reap_children

def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    signal.signal(signal.SIGCHLD, _reap_children)

    bots = []
    for name in args.names:
        bot = Process(
            name=f"AI_{name}_1",
            function=ZappyAI.launch_bot,
            args=[name, args.host, args.port, True]
        )
        bot.start()
        bots.append(bot)

    try:
        for bot in bots:
            bot.wait()
        return 0
    except KeyboardInterrupt:
        for bot in bots:
            bot.terminate()
        return 0
    except Exception:
        return 84

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
