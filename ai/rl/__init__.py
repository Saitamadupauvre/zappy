"""Vendored Zappy-RL inference stack.

Self-contained copy of the deploy path from the sibling ``Zappy-RL`` project:
``rl.deploy.zappy_ai_adapter`` loads a trained Flax policy (``params.msgpack``)
and drives it as a Zappy AI client against the reference server.

Sourced verbatim from ``Zappy-RL/zappy_rl`` (env + algo/networks + deploy);
package layout is preserved so the original relative imports resolve unchanged.
Run with ``Zappy/ai`` on PYTHONPATH:

    python -m rl.deploy.zappy_ai_adapter --params runs/<run>/params.msgpack ...

Requires numpy + jax + flax (see ``rl/requirements.txt``).
"""
