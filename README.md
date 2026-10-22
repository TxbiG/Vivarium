# Vivarium
Vivarium provides simulated worlds in which agents can observe, act, and be measured. It is designed to run headless at high speed for training, and with a renderer for inspection and debugging. Every run is reproducible from a fixed timestep and a seed, so results can be compared across agents, algorithms, and versions.

> *Note* Vivarium is in early development. The API is not stable and may change between releases.

## Features

- Deterministic simulation with a fixed timestep and seeded randomness
- Headless mode for fast, large-scale training runs
- Rigid body physics with collision detection and configurable world parameters
- Gym-style environment interface (`reset`, `step`, observations, rewards)
- Structured observations: object state, sensor rays, and optional rendered frames
- Episode recording and deterministic replay
- Randomizable physics parameters (gravity, friction, mass) for generalization testing

Some features listed above are in development. See [Status](#status).

| Component | State |
|---|---|
| Core simulation loop | In development |
| Headless mode | Planned |
| Gym-style interface | Planned |
| Python bindings | Planned |
| Episode recording and replay | Planned |
| Benchmark task suite | Planned |

## Getting Started

### Requirements

- A C++17 compatible compiler
- CMake 3.20 or later
- Python 3.9 or later (for the Python bindings, when available)

### Build

```bash
git clone https://github.com/<owner>/Vivarium.git
cd Vivarium
cmake -B build
cmake --build build
```

### Run the tests

```bash
ctest --test-dir build
```

## Usage

The example below shows the intended interface for the Python bindings.

```python
import vivarium

env = vivarium.make("BallPush-v0", seed=42, headless=True)
obs = env.reset()

done = False
while not done:
    action = env.action_space.sample()
    obs, reward, done, info = env.step(action)

env.close()
```

## Environments

Planned initial environments:

| Name | Description |
|---|---|
| BallPush | Push a ball to a target location |
| PoleBalance | Keep an inverted pendulum upright |
| StackBlocks | Stack rigid bodies to a target height |
| Navigate | Reach a goal in a procedurally generated space |

## Project Layout

```
Vivarium/
  include/     Public headers
  src/         Simulation core
  envs/        Environment definitions
  bindings/    Language bindings
  examples/    Runnable examples
  tests/       Unit and determinism tests
  docs/        Documentation
```

## Determinism

Vivarium aims for bit-exact reproducibility on a given platform and build configuration. Given the same version, seed, and action sequence, an episode produces identical results. Cross-platform floating-point differences may cause divergence and are tracked as known limitations.

## Contributing

Contributions are welcome. Please read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request. For bugs and feature requests, open an issue with a minimal reproduction where possible.

## License

Released under the [MIT License](LICENSE). Replace this section if you choose a different license.
