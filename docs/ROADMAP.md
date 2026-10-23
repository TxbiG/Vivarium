# Roadmap

## Scene-authoring beta
- [x] Scene document and JSON save/load
- [x] Moss debug-draw viewport with grid and object overlays
- [x] Place roads, terrain proxies, ramps, walls, obstacles, robot starts, goals and sensors
- [x] Hierarchy selection and editable transform / physics properties
- [x] Sample road/terrain robot-test scene
- [ ] Camera orbit/pan/zoom and robust selection ray casting across elevated geometry
- [ ] Transform gizmos and undo/redo
- [ ] Terrain heightmap sculpting and actual terrain mesh generation
- [ ] Convert scene descriptors into backend collision objects

## Core simulation reliability
- [ ] Integrate/evaluate a mature physics backend
- [ ] Episode JSON parsing, action replay and first-divergence diagnostics
- [ ] Gymnasium registration/checker integration
- [ ] Native Python wheel package and CI
- [ ] Throughput and memory benchmarks
- [ ] Structured sensor observations, frame capture and vectorized rollouts
- [ ] 3D dynamics backend and cross-platform regression matrix
