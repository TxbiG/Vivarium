# Architecture

`Environment` defines reset/step/observe/spec. The four starter tasks own their reward and termination logic. `PhysicsWorld2D` uses fixed-step integration and simple circle collision response. The Python facade loads `_vivarium` if available, otherwise runs an educational Python fallback. Recorder stores actions, observations, rewards and terminal flags as JSON.

Limits: no CCD, joint constraints, frictional contact solver, broadphase tree, or 3D. PoleBalance is a simplified pedagogical model. StackBlocks uses procedural placement rather than the rigid-body world. Determinism claims are scoped to same-build repeatability tests.


## Moss editor integration

The desktop editor is a separate executable target, not part of `vivarium` itself. When `VIVARIUM_BUILD_MOSS_EDITOR=ON`, CMake adds the developer-supplied Moss checkout with `add_subdirectory`, links against the public `Moss` target, and compiles the editor's application loop and GUI panels. The editor uses `Moss_CreateWindow`, `Moss_CreateRenderer`, event polling, renderer frame calls, the GUI window/text/input APIs, and matching resource teardown.

The editor currently exposes an observation-driven diagnostic grid for the 2D tasks rather than drawing native scene meshes. The simulation logic still lives in the headless environment library. The selected seed/action sequence and the simulation's fixed-step progression are independent of the rendering rate, while real-time Run mode uses a capped wall-clock accumulator to schedule the fixed 1/60-second ticks.

Moss is an external checkout, not vendored code. Its current branch and graphics backend should be pinned and tested in the integration environment; the base archive's default build deliberately leaves the editor disabled.
