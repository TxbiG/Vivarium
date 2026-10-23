# Scene authoring

Vivarium scenes are stored in schema-versioned JSON. Each `SceneObject` has an ID, type, name, centre position `(x, y, z)`, dimensions `(width, height, depth)`, mass, friction, restitution, fixed flag and collision-enabled flag.

## First supported object types

- `ground`: base plane proxy
- `road`: road slab with lane markings in the viewport
- `terrain`: generated wireframe height preview on a rectangular footprint
- `ramp`: ramp proxy with slope guide lines
- `wall` and `obstacle`: collision/placement proxy boxes
- `robot_spawn`: robot start marker and heading line
- `goal`: goal marker
- `sensor`: sensor marker with illustrative ray lines

## Controls

1. Pick a placement tool in the Environment Builder panel.
2. Click inside the main viewport to place an instance.
3. Select items through the viewport or hierarchy list.
4. Edit position, dimensions and physics properties in the Object Inspector.
5. Save the scene JSON to `vivarium_scene.json` or load that file.

## Semantics

The current geometry is for scene authoring and visual inspection. It does not claim to create a fully watertight terrain mesh, road network graph, navmesh, or rigid-body collision representation. Those are planned integration steps between the scene document, collision backend and robot task definitions.
