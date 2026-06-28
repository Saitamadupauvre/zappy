
import argparse
from dataclasses import dataclass

_MIN_PORT = 1
_MAX_PORT = 65535
_RESERVED_NAMES = ["GRAPHIC"]

@dataclass
class ZappyArgs:
    """Parsed and validated CLI arguments for zappy_ai."""

    port: int
    names: list[str]
    host: str
    no_encrypt: bool
    debug: bool = False

    # --- Constructors --- #
    @classmethod
    def from_argv(cls, argv: list[str] | None = None) -> "ZappyArgs":
        """Parse *argv* (or sys.argv[1:] when None) and return a ZappyArgs."""

        parser = cls._build_parser()
        namespace = parser.parse_args(argv)
        return cls._from_namespace(namespace, parser)

    @classmethod
    def _from_namespace(
        cls,
        args: argparse.Namespace,
        parser: argparse.ArgumentParser,
    ) -> "ZappyArgs":

        port = args.port
        names = [n.strip() for n in args.names]
        host = args.host.strip()
        debug = args.debug

        errors: list[str] = []

        if not (_MIN_PORT <= port <= _MAX_PORT):
            errors.append(f"Invalid port '{port}': must be between {_MIN_PORT} and {_MAX_PORT}.")
        if not names:
            errors.append("Team name cannot be empty.")
        
        for name in names:
            if not name:
                errors.append("Team name cannot be empty.")
            elif name in _RESERVED_NAMES:
                errors.append(f"Team name '{name}' is reserved for the GUI.")

        if not host:
            errors.append("Hostname cannot be empty.")

        if errors:
            parser.error("\n  ".join(errors))

        return cls(port=port, names=names, host=host, no_encrypt=args.no_encrypt, debug=debug)

    # --- Factory --- #
    @staticmethod
    def _build_parser() -> argparse.ArgumentParser:
        # add_help=False because -h is taken for --host.
        parser = argparse.ArgumentParser(prog="zappy_ai", add_help=False)

        parser.add_argument(
            "--help",
            action="help",
            help="USAGE: ./zappy_ai -p port -n name1 name2 -h machine",
        )
        parser.add_argument(
            "-p",
            dest="port",
            type=int,
            required=True,
            metavar="port",
            help="Port number of the zappy server",
        )
        parser.add_argument(
            "-n",
            dest="names",
            type=str,
            nargs="+",
            required=True,
            metavar="name",
            help="Team name to join",
        )
        parser.add_argument(
            "-h",
            dest="host",
            type=str,
            default="localhost",
            metavar="machine",
            help="Hostname of the server (default: localhost)",
        )
        parser.add_argument(
            "--no-encrypt",
            dest="no_encrypt",
            action="store_true",
            default=False,
            help="Disable the communication encryption system (broadcast)"
        )

        parser.add_argument(
            "-d", "--debug",
            dest="debug",
            action="store_true",
            help="Enable verbose debug printing",
        )
        return parser

    # --- Helpers --- #
    def address(self) -> str:
        """Return 'host:port' as a convenience string."""
        return f"{self.host}:{self.port}"

    def __str__(self) -> str:
        return (f"ZappyArgs(host={self.host!r}, port={self.port}, name={self.name!r})")
# End of ZappyArgs class

def parse_args(argv: list[str] | None = None) -> ZappyArgs:
    """Module-level shortcut — keeps the old call-site signature intact."""

    return ZappyArgs.from_argv(argv)
