#include "SceneBuilderPanel.hpp"
#include "../EditorApplication.hpp"
#include <Moss/Moss_GUI.h>
#include <algorithm>
#include <string>

namespace vivarium::editor {
void DrawSceneBuilderPanel(EditorApplication& app) {
    if (BeginWindow("Vivarium | Environment Builder", nullptr, 0)) {
        auto& scene = app.scene();
        Text("SCENE TOOLS");
        Text("Choose a placement tool, then click inside the 3D viewport.");
        if (RadioButton("Select / inspect", !app.placementMode())) app.cancelPlacement();
        if (RadioButton("Place: Road", app.placementMode() && app.placementType()==SceneObjectType::Road)) app.beginPlacement(SceneObjectType::Road);
        if (RadioButton("Place: Terrain tile", app.placementMode() && app.placementType()==SceneObjectType::Terrain)) app.beginPlacement(SceneObjectType::Terrain);
        if (RadioButton("Place: Ramp", app.placementMode() && app.placementType()==SceneObjectType::Ramp)) app.beginPlacement(SceneObjectType::Ramp);
        if (RadioButton("Place: Wall", app.placementMode() && app.placementType()==SceneObjectType::Wall)) app.beginPlacement(SceneObjectType::Wall);
        if (RadioButton("Place: Obstacle", app.placementMode() && app.placementType()==SceneObjectType::Obstacle)) app.beginPlacement(SceneObjectType::Obstacle);
        if (RadioButton("Place: Robot spawn", app.placementMode() && app.placementType()==SceneObjectType::RobotSpawn)) app.beginPlacement(SceneObjectType::RobotSpawn);
        if (RadioButton("Place: Goal", app.placementMode() && app.placementType()==SceneObjectType::Goal)) app.beginPlacement(SceneObjectType::Goal);
        if (RadioButton("Place: Sensor", app.placementMode() && app.placementType()==SceneObjectType::Sensor)) app.beginPlacement(SceneObjectType::Sensor);
        if (app.placementMode()) Text("PLACEMENT ACTIVE: %s — click in viewport", to_string(app.placementType()));

        Text("QUICK ACTIONS");
        if (RadioButton("Add road at centre", false)) app.addSceneObject(SceneObjectType::Road);
        if (RadioButton("Add terrain at centre", false)) app.addSceneObject(SceneObjectType::Terrain);
        if (RadioButton("Add robot spawn", false)) app.addSceneObject(SceneObjectType::RobotSpawn);
        if (RadioButton("Duplicate selected", false)) app.duplicateSelectedObject();
        if (RadioButton("Delete selected", false)) app.removeSelectedObject();
        if (RadioButton("Save scene JSON", false)) app.saveScene();
        if (RadioButton("Load scene JSON", false)) app.loadScene();

        Text("WORLD SETTINGS");
        double old_width=scene.world_width, old_depth=scene.world_depth;
        InputDouble("World width", &scene.world_width, 1.0, 10.0, "%.1f");
        InputDouble("World depth", &scene.world_depth, 1.0, 10.0, "%.1f");
        InputDouble("Gravity Y", &scene.gravity_y, .1, 1.0, "%.3f");
        scene.world_width=std::clamp(scene.world_width, 10.0, 10000.0);
        scene.world_depth=std::clamp(scene.world_depth, 10.0, 10000.0);
        if (scene.world_width!=old_width || scene.world_depth!=old_depth) app.refreshViewportCamera();
        if (RadioButton("Show grid", app.showGrid())) app.toggleGrid();
        if (RadioButton("Show collision overlay", app.showCollisionOverlay())) app.toggleCollisionOverlay();

        Text("SCENE HIERARCHY (%d objects)", static_cast<int>(scene.objects.size()));
        for (std::size_t i=0;i<scene.objects.size();++i) {
            const auto& object=scene.objects[i];
            const std::string label=std::to_string(object.id)+" | "+object.name;
            if (RadioButton(label.c_str(), app.selectedObjectIndex()==i)) app.selectObject(i);
        }
        Text("Scene format: versioned JSON (vivarium_scene.json)");
        Text("Status: %s", app.statusMessage().c_str());
    }
    EndWindow();
}
} // namespace vivarium::editor
