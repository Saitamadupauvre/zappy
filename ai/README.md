# Zappy - AI Part

This project uses **`uv`**, an extremely fast Python package installer and resolver written in Rust. It replaces standard tools like `pip`, `pip-tools`, and `virtualenv`, making environment management and execution seamless.

---

## What is `uv`?

`uv` is designed to be a drop-in replacement for common Python development workflows. It automatically manages virtual environments and dependencies behind the scenes without requiring you to manually activate an environment.

### Key Benefits:

- **Blazing Fast**: Up to 10-100x faster than `pip`.
- **Automatic Environment Management**: `uv run` handles virtual environments automatically.
- **Single Binary**: No Python dependency required to install `uv` itself.

---

## Setup & Installation

### 1. Install `uv`

If you don't have `uv` installed on your machine, run the following command:

```bash
curl -LsSf [https://astral.sh/uv/install.sh](https://astral.sh/uv/install.sh) | sh
```

### 2. Project Initialization (Optional)

If dependencies are listed in a `pyproject.toml`, uv will automatically install them on the first run.

To manually sync or install dependencies:

```bash
uv sync
```

---

## Launching the Project

First you need to start the server. Open a terminal and run:

```bash
./binaries/zappy_server -p 4242 -x 10 -y 10 -n TEST -c 5
```

For the Python client, you do **not need** to manually run `source .venv/bin/activate`. `uv` takes care of executing your script inside the correct environment.

To start the application, open a new terminal and simply run:

```bash
uv run src/main.py -p 4242 -n TEST
```

> Note: If the virtual environment does not exist yet, `uv run` will automatically create it and install the required dependencies before executing the script.
