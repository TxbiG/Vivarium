#pragma once

#include "vivarium/recorder.hpp"
#include "vivarium/scene.hpp"
#include "vivarium/vivarium.hpp"

#include <chrono>
#include <memory>
#include <string>

struct Moss_Window;
struct Moss_Renderer;
struct Moss_Camera;
struct Moss_DebugDrawList;

namespace vivarium::editor {

class EditorApplication {
public:
    EditorApplication();
    ~EditorApplication();

    int run();

    const std::string& environmentId() const noexcept { return environment_id_; }
    void selectEnvironment(const std::string& id);
    int seed() const noexcept { return seed_; }
    int& seedValue() noexcept { return seed_; }
    double& actionX() noexcept { return action_x_; }
    double& actionY() noexcept { return action_y_; }
    bool running() const noexcept { return running_; }
    void setRunning(bool value) noexcept { running_ = value && !episode_done_; }
    bool episodeDone() const noexcept { return episode_done_; }
    std::size_t stepCount() const noexcept { return step_count_; }
    double episodeReturn() const noexcept { return episode_return_; }
    double lastReward() const noexcept { return last_reward_; }
    const Observation& observation() const noexcept { return observation_; }
    EnvironmentSpec spec() const { return environment_ ? environment_->spec() : EnvironmentSpec{}; }
    const std::string& statusMessage() const noexcept { return status_message_; }

    void resetEpisode();
    void stepOnce();
    void update(double elapsed_seconds);
    bool saveEpisode();
    SceneDocument& scene() noexcept { return scene_; }
    const SceneDocument& scene() const noexcept { return scene_; }
    std::size_t selectedObjectIndex() const noexcept { return selected_object_index_; }
    void selectObject(std::size_t index) noexcept;
    SceneObject* selectedObject() noexcept;
    const SceneObject* selectedObject() const noexcept;
    std::size_t addSceneObject(SceneObjectType type);
    void removeSelectedObject();
    void duplicateSelectedObject();
    bool saveScene();
    bool loadScene();
    void toggleGrid() noexcept { show_grid_ = !show_grid_; }
    void refreshViewportCamera() noexcept { last_window_width_ = last_window_height_ = 0; }
    void beginPlacement(SceneObjectType type) noexcept;
    void cancelPlacement() noexcept;
    bool placementMode() const noexcept { return placement_mode_; }
    SceneObjectType placementType() const noexcept { return placement_type_; }
    void toggleCollisionOverlay() noexcept { show_collision_overlay_ = !show_collision_overlay_; }
    bool showGrid() const noexcept { return show_grid_; }
    bool showCollisionOverlay() const noexcept { return show_collision_overlay_; }

private:
    bool initializeMoss();
    void shutdownMoss() noexcept;
    void drawFrame();
    void advanceOneStep();
    void drawViewport();
    void drawGrid();
    void drawSceneObject(const SceneObject& object, bool selected);
    void updateViewportCamera();
    void handleViewportPicking();

    Moss_Window* window_{nullptr};
    Moss_Renderer* renderer_{nullptr};
    Moss_Camera* camera_{nullptr};
    Moss_DebugDrawList* debug_draw_{nullptr};
    SceneDocument scene_{make_default_scene()};
    std::size_t selected_object_index_{6};
    bool show_grid_{true};
    bool placement_mode_{false};
    SceneObjectType placement_type_{SceneObjectType::Obstacle};
    bool show_collision_overlay_{true};
    int last_window_width_{0};
    int last_window_height_{0};
    std::shared_ptr<Environment> environment_;
    std::unique_ptr<EpisodeRecorder> recorder_;
    std::string environment_id_{"BallPush-v0"};
    int seed_{42};
    double action_x_{0.5};
    double action_y_{0.0};
    bool running_{false};
    bool episode_done_{false};
    bool single_step_requested_{false};
    std::size_t step_count_{0};
    double episode_return_{0.0};
    double last_reward_{0.0};
    double accumulator_{0.0};
    Observation observation_;
    std::string status_message_{"Ready"};
};

} // namespace vivarium::editor
