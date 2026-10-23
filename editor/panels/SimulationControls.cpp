#include "SimulationControls.hpp"
#include "../EditorApplication.hpp"
#include <Moss/Moss_GUI.h>

namespace vivarium::editor {
void DrawSimulationControls(EditorApplication& app) {
    if (BeginWindow("Vivarium | Simulation Controls", nullptr, 0)) {
        Text("SIMULATION");
        Text("Choose an environment:");
        if (RadioButton("BallPush", app.environmentId() == "BallPush-v0")) app.selectEnvironment("BallPush-v0");
        if (RadioButton("Navigate", app.environmentId() == "Navigate-v0")) app.selectEnvironment("Navigate-v0");
        if (RadioButton("PoleBalance", app.environmentId() == "PoleBalance-v0")) app.selectEnvironment("PoleBalance-v0");
        if (RadioButton("StackBlocks", app.environmentId() == "StackBlocks-v0")) app.selectEnvironment("StackBlocks-v0");

        Text("Experiment seed (used at reset)");
        InputInt("Seed", &app.seedValue(), 1, 100);
        if (RadioButton("Reset Episode", false)) app.resetEpisode();
        if (RadioButton("Run", app.running())) app.setRunning(true);
        if (RadioButton("Pause", !app.running())) app.setRunning(false);
        if (RadioButton("Step One Tick", false)) app.stepOnce();
        if (RadioButton("Save Episode JSON", false)) app.saveEpisode();

        Text("Action vector (clipped by environment to [-1, 1])");
        InputDouble("Action X", &app.actionX(), 0.05, 0.25, "%.2f");
        InputDouble("Action Y", &app.actionY(), 0.05, 0.25, "%.2f");
        Text("Simulation step: fixed 1/60 second");
        Text("Headless training remains available via C++ / Python APIs.");
        Text("Status: %s", app.statusMessage().c_str());
    }
    EndWindow();
}
}
