# Trained models (`ai/runs`)

Trained reinforcement-learning policies for the Zappy AI, produced by the sibling
`Zappy-RL` project (JAX/Flax recurrent MAPPO) and deployed against the reference
server via the vendored adapter in [`ai/rl`](../rl). Launch them with
[`scripts/zappy_run.sh model`](../../scripts/zappy_run.sh).

Each run directory holds:

| File | What |
|------|------|
| `params.msgpack` | Trained actor weights (Flax params, msgpack-serialized) |
| `config.json` | Training/deploy config — map size, `freq`, `n_agents`, `role_dim`, `inv_every`, `hidden` |
| `eval.json` | Training throughput summary |
| `strategy_eval.console.json` | Greedy-policy evaluation summary (milestones, survival) |
| `strategy_eval@…@greedy.json` | Full eval record (conditions encoded in filename) |

## Models

| Key | Run | Map / freq | Result (greedy eval, 512 episodes) |
|-----|-----|-----------|-------------------------------------|
| `10x10` | `strategy-v2-s21-10x10-f100-ov10-ds0.85` | 10×10, freq 100, 6 agents | Campaign-v2 overall best — 0.996 all-6 → L8, ~3536-tick median |
| `30x30` | `strategy-v2-s11-30x30-f20-ov8-ds0.85` | 30×30, freq 20, 6 agents | Best 30×30 — `t_all6_l8` rate **0.977**, death 0.010, violation 0.014 |

[`models.json`](./models.json) maps these keys to run directories; the run tool
uses it to resolve `--model 10x10` / `--model 30x30`.

## Usage

```bash
# from the Zappy project root
scripts/zappy_run.sh setup                 # one-time: create ai/.venv with jax+flax
scripts/zappy_run.sh model --model 10x10   # server + GUI + 6 trained agents
scripts/zappy_run.sh model --model 30x30 --no-gui --duration 120
```

The server's map/frequency/agent count are read from the run's `config.json`, so
each model launches against the map it was trained on.
