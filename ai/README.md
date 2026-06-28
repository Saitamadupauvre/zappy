# zappy_ai

Autonomous AI client for the Zappy network game. Written in Python, managed with [`uv`](https://github.com/astral-sh/uv).

> For architecture, strategy, and testing details, see [`docs/ai_documentation.md`](../../docs/ai_documentation.md).

---

## Requirements

- Python 3.10+
- `uv` - install once with:

```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```

---

## Build

### Via Makefile (recommended)

From the project root:

```bash
make zappy_ai
```

### Via uv directly

```bash
uv sync          # install dependencies into .venv
```

This produces the `zappy_ai` binary (or prepares the `uv run` environment).

---

## Usage

``` helper
./zappy_ai -p <port> -n <team> [-h <host>] [--no-encrypt] [-d]

  -p port       Server port (required)
  -n name       Team name (required) - can pass multiple names
  -h machine    Server hostname (default: localhost)
  --no-encrypt  Disable broadcast encryption (useful for debugging)
  -d / --debug  Enable verbose debug output
```

---

## Quick Start

Open three terminals:

### 1 - Server

```bash
./zappy_server -p 4242 -x 20 -y 20 -n MY_TEAM -c 6 -f 100
```

### 2 - GUI *(optional)*

```bash
./zappy_gui -p 4242 -h localhost
```

### 3 - AI

```bash
# compiled binary
./zappy_ai -p 4242 -n MY_TEAM -h localhost

# or via uv (no compile step needed)
uv run ai/main.py -p 4242 -n MY_TEAM -h localhost
```

The AI is fully autonomous from this point. One instance acts as **Queen**, the rest become **Followers** automatically.

---

## Debug

```bash
# flag
./zappy_ai -p 4242 -n MY_TEAM -d

# or env var
ZAPPY_AI_DEBUG=1 ./zappy_ai -p 4242 -n MY_TEAM
```

Logs (CSV) are written to `ai/logs/run_<timestamp>/` on each run.

---

## Tests

```bash
uv run pytest            # all tests
uv run pytest -v         # verbose
uv run pytest --cov=ai   # with coverage
```
