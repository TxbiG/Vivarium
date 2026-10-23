#include "MetricsPanel.hpp"
#include "../EditorApplication.hpp"
#include <Moss/Moss_GUI.h>

namespace vivarium::editor {
void DrawMetricsPanel(EditorApplication& app) {
    if (BeginWindow("Vivarium | Episode Metrics", nullptr, 0)) {
        Text("EXPERIMENT METRICS");
        Text("Environment: %s", app.environmentId().c_str());
        Text("Seed: %d", app.seed());
        Text("Steps: %d", static_cast<int>(app.stepCount()));
        Text("Last reward: %.8f", app.lastReward());
        Text("Episode return: %.8f", app.episodeReturn());
        Text("Running: %s", app.running() ? "yes" : "no");
        Text("Completed: %s", app.episodeDone() ? "yes" : "no");
        Text("Recording: enabled in this editor session");
        Text("Save writes vivarium_episode.json in the working directory.");
    }
    EndWindow();
}
}
