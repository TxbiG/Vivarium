# Moss editor integration

## Design

`vivarium_editor` is an optional desktop editor that consumes the public `TxbiG/Moss` platform, renderer, debug-draw and GUI APIs. The simulation runtime and Python API remain independent of the editor so that training can run headlessly.

## Scene-authoring milestone

- Moss debug-draw list presents a 3D grid, selected object bounds, roads and lane guides, terrain wireframe relief, ramps, walls, obstacles, robot spawn points, goals and sensor-ray markers.
- The scene builder panel selects a placement tool, then places the chosen item at the clicked viewport location.
- Hierarchy list selection, duplicate/delete, JSON save/load and world settings are provided.
- The inspector edits transforms, dimensions, mass, friction, restitution, fixed state and collision-enabled state.
- Episode controls/metrics are a separate authoring workflow, not coupled to the editor renderer.

## Configure

```powershell
cmake -S . -B build-editor `
  -DVIVARIUM_BUILD_MOSS_EDITOR=ON `
  -DVIVARIUM_MOSS_ROOT="C:/path/to/Moss"
cmake --build build-editor --config Release --target vivarium_editor
```

## Important limitations

The scene model currently stores boxes/proxies. Terrain is displayed as a generated wireframe height preview; it is not yet a sculptable heightmap or a terrain collision mesh. Roads and ramps are visible scene objects, but their geometry is not yet exported into a 3D simulation collision backend. Selection is based on a top-view-like plane mapping rather than precise perspective ray casting. Moss debug-draw primitives are the current viewport renderer, deliberately reusing Moss rather than introducing another graphics library.

The archive can test the headless runtime without Moss. The actual editor target must be compiled against the user's local Moss checkout and configured graphics backend before the Moss-linked application can be considered verified.
