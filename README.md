# Vivarium

Vivarium provides simulated worlds in which agents can observe, act, and be measured. This beta foundation contains a C++17 simulation core, simple fixed-step 2D circle physics, four starter tasks, a Python interface, examples, tests, CI, and an **optional desktop editor powered by [TxbiG/Moss](https://github.com/TxbiG/Moss)**.

> **Beta status:** this is an early working foundation, not a validated general-purpose physics engine. `PoleBalance` uses simplified instructional dynamics; `StackBlocks` is a placement task rather than a full rigid-body stack simulation. The Python fallback follows the Gymnasium five-value step convention but is not a `gymnasium.Env` subclass and does not promise numerical parity with C++.

## Included

- C++17 and CMake core, plus CTest tests.
- Fixed-timestep 2D circle physics with gravity, world bounds, and basic collision response.
- Explicit seeded C++ PRNG with same-build repeatability tests.
- `BallPush-v0`, `Navigate-v0`, `PoleBalance-v0`, and `StackBlocks-v0`.
- Python facade: `reset()` returns `(observation, info)`; `step()` returns `(observation, reward, terminated, truncated, info)`.
- Optional pybind11 native module source.
- Episode JSON writer (JSON loading and deterministic replay are not yet implemented).
- Optional Moss-powered editor target with environment selection, run/pause/reset/single-step controls, seed/action inputs, Moss debug-draw 3D scene viewport, object hierarchy, click-to-place workflow, object property inspector, episode metrics, and JSON scene/episode export.

## C++ build and test

Windows PowerShell:

```powershell
cmake -S . -B build -DVIVARIUM_BUILD_TESTS=ON -DVIVARIUM_BUILD_EXAMPLES=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\build\Release\vivarium_random_agent.exe
```

Linux/macOS:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/vivarium_random_agent
```

## Python quick start

```bash
python -m pip install -e ./python
python -m unittest discover -s tests -p 'test_python.py'
python examples/python/random_agent.py
```

```python
import vivarium

env = vivarium.make("BallPush-v0", seed=42, headless=True)
observation, info = env.reset(seed=42)
for _ in range(500):
    observation, reward, terminated, truncated, info = env.step([0.1, 0.0])
    if terminated or truncated:
        break
env.close()
```

The Python fallback runs without a compiled extension. The pybind11 source can be enabled with `-DVIVARIUM_BUILD_PYTHON=ON` when Python development headers and pybind11 are installed; automatic wheel building is not configured in this beta.

## Build the Moss editor

The editor is optional. Headless training and C++ tests do not require Moss or a graphics backend. Use a local checkout of [TxbiG/Moss](https://github.com/TxbiG/Moss), which defines a CMake target named `Moss` and exposes public platform, renderer, and GUI headers.

A simple workspace layout is:

```text
workspace/
  Moss/       # TxbiG/Moss checkout
  Vivarium/   # this repository
```

Configure using the real path to your Moss checkout:

```powershell
cmake -S . -B build-editor `
  -DVIVARIUM_BUILD_MOSS_EDITOR=ON `
  -DVIVARIUM_MOSS_ROOT="C:/path/to/Moss"
cmake --build build-editor --config Release --target vivarium_editor
```

On Linux/macOS use the same CMake options with a Unix path. Build Moss with a supported graphics backend and install its dependencies first (for example, Vulkan SDK when Vulkan is enabled in Moss). The integration adds the supplied Moss checkout with `add_subdirectory` and links to the existing `Moss` target; it does not vendor Moss's source into Vivarium.

The scene-authoring milestone includes:

- Moss window, renderer, debug-draw list, camera and event lifecycle.
- A 3D viewport rendered through Moss debug-draw primitives, with a world grid, highlighted selection, world bounds, road lane markings, terrain wireframe height preview, ramp guides, obstacle bounds, robot spawn, goal and sensor ray markers.
- Click-to-place tools for roads, terrain tiles, ramps, walls, obstacles, robot spawns, goals and sensors.
- Object selection through viewport clicks and the scene hierarchy; transforms, dimensions, mass, friction, restitution, fixed state and collision enablement are editable in the inspector.
- Scene JSON save/load, object duplication/removal, world width/depth/gravity controls, grid toggle and collision-overlay toggle.
- Simulation run/pause/reset/single-step, seed/action controls, metrics and episode JSON export.

**Remaining editor limitations:** Moss debug-draw boxes and lines are visual authoring primitives, not a full shaded mesh/material system. Terrain tiles are a generated wireframe height preview and do not yet generate terrain collision meshes; roads and ramps are editable scene descriptors and their collision geometry is not yet connected to the training physics backend. Click placement casts a perspective ray to the ground plane for the fixed authoring camera. Orbit/pan/zoom controls, transform gizmos, and precise picking against elevated mesh surfaces remain follow-up work. The editor must still be compiled and smoke-tested against the exact local Moss checkout/backend because that repository is not vendored into this archive.

## Known gaps before a stable release

- Integrate and evaluate a mature physics backend such as Box2D; current solver has circles only and no continuous collision detection, contact friction solver, or joints.
- Implement episode JSON loading, action replay, and first-divergence reports.
- Implement a proper Gymnasium subclass/registration and test it with Gymnasium's checker.
- Add ray sensors, structured sensor observations, frame capture, vectorized rollout workers, and benchmarks.
- Build and test the editor against pinned Moss revisions on Windows, Linux, and macOS.
- Add terrain heightmap sculpting and mesh/collision generation.
- Add accurate perspective ray picking, transform gizmos, undo/redo and workspace persistence.
- Connect scene collision shapes to the simulation physics backend and add a 3D dynamics backend.

## Repository layout

```text
include/vivarium/       Public C++ API
src/                    Core simulation and recording
bindings/python/        Optional native Python binding
python/vivarium/        Python facade and fallback
editor/                 Optional Moss-powered editor and panels
examples/               C++ and Python examples
tests/                  Automated tests
docs/                   Architecture and roadmap
.github/workflows/      CI
```

## Determinism

Simulation advances using a fixed time step and explicit seeded randomness. Tests check that the same seed and action sequence produce the same outputs in the same build. Floating-point behavior and compiler/backend differences mean bit-exact cross-platform reproducibility is not promised.

## Contributing

Contributions are welcome. Please read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request. For bugs and feature requests, open an issue with a minimal reproduction where possible.

## License

The project is distributed under the [MIT License](LICENSE).
