#include "EditorApplication.hpp"

#include "panels/EnvironmentInspector.hpp"
#include "panels/MetricsPanel.hpp"
#include "panels/SimulationControls.hpp"
#include "panels/SceneBuilderPanel.hpp"

#include <Moss/Moss_GUI.h>
#include <Moss/Moss_Platform.h>
#include <Moss/Moss_Renderer.h>

#include <algorithm>
#include <cmath>
#include <sstream>
#include <utility>
#include <cstdio>

namespace vivarium::editor {
namespace {
constexpr double kFixedStep = 1.0 / 60.0;
constexpr float kCameraHeight = 132.0f;

Float3 make_f3(double x, double y, double z) {
    return Float3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}
Color scene_color(SceneObjectType type) {
    switch (type) {
    case SceneObjectType::Ground: return Color(.30f,.34f,.38f,1.f);
    case SceneObjectType::Road: return Color(.25f,.28f,.32f,1.f);
    case SceneObjectType::Terrain: return Color(.22f,.62f,.34f,1.f);
    case SceneObjectType::Ramp: return Color(.82f,.59f,.22f,1.f);
    case SceneObjectType::Wall: return Color(.70f,.73f,.77f,1.f);
    case SceneObjectType::Obstacle: return Color(.80f,.31f,.27f,1.f);
    case SceneObjectType::RobotSpawn: return Color(.20f,.62f,.95f,1.f);
    case SceneObjectType::Goal: return Color(.25f,.90f,.52f,1.f);
    case SceneObjectType::Sensor: return Color(.96f,.80f,.22f,1.f);
    }
    return Color(.8f,.8f,.8f,1.f);
}

}

EditorApplication::EditorApplication() : scene_(make_default_scene()) {}

EditorApplication::~EditorApplication() { shutdownMoss(); }

bool EditorApplication::initializeMoss() {
    // These lifecycle/window/renderer calls are from the public TxbiG/Moss API.
    // In the current Moss headers Moss_Init/Moss_Terminate are commented out;
    // the editor therefore uses the supported window and renderer lifecycle.
    window_ = Moss_CreateWindow("Vivarium Editor | Moss", 1440, 900, nullptr, nullptr);
    if (window_ == nullptr) {
        status_message_ = "Moss could not create a window. Check the selected graphics backend and platform dependencies.";
        return false;
    }
    renderer_ = Moss_CreateRenderer(window_);
    if (renderer_ == nullptr) {
        status_message_ = "Moss could not create a renderer. Check your Moss graphics backend build options.";
        Moss_TerminateWindow(window_);
        window_ = nullptr;
        return false;
    }
    debug_draw_ = Moss_RendererCreateDebugDrawList(renderer_, 32768);
    if (debug_draw_ == nullptr) {
        status_message_ = "Moss debug-draw creation failed.";
        shutdownMoss();
        return false;
    }
    updateViewportCamera();
    return true;
}

void EditorApplication::shutdownMoss() noexcept {
    if (debug_draw_ != nullptr && renderer_ != nullptr) {
        Moss_RendererDestroyDebugDrawList(renderer_, debug_draw_);
        debug_draw_ = nullptr;
    }
    if (camera_ != nullptr) {
        Moss_CameraDestroy(camera_);
        camera_ = nullptr;
    }
    if (renderer_ != nullptr) {
        Moss_RendererDestroy(renderer_);
        renderer_ = nullptr;
    }
    if (window_ != nullptr) {
        Moss_TerminateWindow(window_);
        window_ = nullptr;
    }
}

int EditorApplication::run() {
    resetEpisode();
    if (!initializeMoss()) return 1;

    auto previous = std::chrono::steady_clock::now();
    while (!Moss_ShouldWindowClose(window_)) {
        Moss_PollEvents();
        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<double> elapsed = now - previous;
        previous = now;
        update(std::min(elapsed.count(), 0.25));
        handleViewportPicking();
        updateViewportCamera();

        Moss_RendererBeginFrame(renderer_);
        drawViewport();
        drawFrame();
        Moss_RendererEndFrame(renderer_);
    }
    shutdownMoss();
    return 0;
}

void EditorApplication::drawFrame() {
    DrawSimulationControls(*this);
    DrawSceneBuilderPanel(*this);
    DrawEnvironmentInspector(*this);
    DrawMetricsPanel(*this);
}

void EditorApplication::selectEnvironment(const std::string& id) {
    if (id == environment_id_) return;
    environment_id_ = id;
    resetEpisode();
}

void EditorApplication::resetEpisode() {
    try {
        environment_ = make_environment(environment_id_);
        observation_ = environment_->reset(static_cast<std::uint64_t>(std::max(seed_, 0)));
        recorder_ = std::make_unique<EpisodeRecorder>(environment_id_, static_cast<std::uint64_t>(std::max(seed_, 0)));
        running_ = false;
        episode_done_ = false;
        single_step_requested_ = false;
        step_count_ = 0;
        episode_return_ = 0.0;
        last_reward_ = 0.0;
        accumulator_ = 0.0;
        status_message_ = "Episode reset with seed " + std::to_string(seed_);
    } catch (const std::exception& ex) {
        running_ = false;
        episode_done_ = true;
        status_message_ = std::string("Could not reset environment: ") + ex.what();
    }
}

void EditorApplication::stepOnce() {
    if (!episode_done_) single_step_requested_ = true;
}

void EditorApplication::update(double elapsed_seconds) {
    if (single_step_requested_) {
        single_step_requested_ = false;
        advanceOneStep();
        return;
    }
    if (!running_ || episode_done_) return;
    accumulator_ += std::max(0.0, elapsed_seconds);
    // Clamp work per UI frame to keep input responsive after a long stall.
    int steps_this_frame = 0;
    while (accumulator_ >= kFixedStep && steps_this_frame < 8 && running_ && !episode_done_) {
        advanceOneStep();
        accumulator_ -= kFixedStep;
        ++steps_this_frame;
    }
}

void EditorApplication::advanceOneStep() {
    if (!environment_ || episode_done_) return;
    const Observation before = observation_;
    const auto action_size = environment_->spec().action_size;
    std::vector<double> action;
    if (action_size == 1) action = {action_x_};
    else action = {action_x_, action_y_};
    try {
        const StepResult result = environment_->step(action);
        if (recorder_) recorder_->record(before, action, result);
        observation_ = result.observation;
        last_reward_ = result.reward;
        episode_return_ += result.reward;
        ++step_count_;
        episode_done_ = result.done();
        if (episode_done_) {
            running_ = false;
            status_message_ = result.terminated ? "Episode terminated" : "Episode time limit reached";
        }
    } catch (const std::exception& ex) {
        running_ = false;
        status_message_ = std::string("Step failed: ") + ex.what();
    }
}

bool EditorApplication::saveEpisode() {
    if (!recorder_) {
        status_message_ = "No episode recording is available yet";
        return false;
    }
    std::string error;
    if (!recorder_->save_json("vivarium_episode.json", &error)) {
        status_message_ = "Could not save episode: " + error;
        return false;
    }
    status_message_ = "Saved recording to vivarium_episode.json";
    return true;
}



void EditorApplication::updateViewportCamera() {
    if (!renderer_) return;
    const int width = std::max(Moss_GetWindowWidth(), 1);
    const int height = std::max(Moss_GetWindowHeight(), 1);
    if (camera_ && width == last_window_width_ && height == last_window_height_) return;
    if (camera_) { Moss_CameraDestroy(camera_); camera_ = nullptr; }
    Moss_Camera3DDesc desc;
    desc.position = make_f3(scene_.world_width * .5 + scene_.world_width * .28, kCameraHeight, scene_.world_depth * .5 + scene_.world_depth * .36);
    desc.target = make_f3(scene_.world_width * .5, 0.0, scene_.world_depth * .5);
    desc.up = Float3(0.0f, 1.0f, 0.0f);
    desc.fov_degrees = 45.0f;
    desc.aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    desc.near_plane = .1f; desc.far_plane = 1000.0f;
    camera_ = Moss_CameraCreate3D(&desc);
    if (camera_) Moss_RendererSetCamera(renderer_, camera_);
    last_window_width_ = width; last_window_height_ = height;
}

void EditorApplication::drawViewport() {
    if (!renderer_ || !debug_draw_) return;
    Moss_DebugDrawListClear(debug_draw_);
    if (show_grid_) drawGrid();
    for (std::size_t i = 0; i < scene_.objects.size(); ++i)
        drawSceneObject(scene_.objects[i], i == selected_object_index_);

    // Runtime markers remain visible while task episodes are paused or stepping.
    if (environment_id_ == "BallPush-v0" && observation_.size() >= 10) {
        Moss_DebugSphereDesc agent; agent.center = make_f3(observation_[0], .8, observation_[1]);
        agent.radius = .7f; agent.color = Color(.10f,.65f,1.f,1.f); Moss_DebugDrawSphere(debug_draw_, &agent);
        Moss_DebugSphereDesc ball; ball.center = make_f3(observation_[4], .55, observation_[5]);
        ball.radius = .55f; ball.color = Color(.92f,.44f,.24f,1.f); Moss_DebugDrawSphere(debug_draw_, &ball);
        Moss_DebugSphereDesc goal; goal.center = make_f3(observation_[8], .2, observation_[9]);
        goal.radius = .65f; goal.color = Color(.2f,1.f,.45f,1.f); Moss_DebugDrawSphere(debug_draw_, &goal);
    } else if (environment_id_ == "Navigate-v0" && observation_.size() >= 4) {
        Moss_DebugSphereDesc agent; agent.center = make_f3(observation_[0], .5, observation_[1]);
        agent.radius = .5f; agent.color = Color(.10f,.65f,1.f,1.f); Moss_DebugDrawSphere(debug_draw_, &agent);
        Moss_DebugSphereDesc goal; goal.center = make_f3(observation_[2], .35, observation_[3]);
        goal.radius = .4f; goal.color = Color(.2f,1.f,.45f,1.f); Moss_DebugDrawSphere(debug_draw_, &goal);
    }
    Moss_RendererDrawDebugList(renderer_, debug_draw_);
}

void EditorApplication::drawGrid() {
    const Color minor(.20f,.24f,.29f,1.f), major(.32f,.39f,.46f,1.f);
    for (double x = 0; x <= scene_.world_width; x += 5.0) {
        Moss_DebugLineDesc line; line.from = make_f3(x, .005, 0); line.to = make_f3(x, .005, scene_.world_depth);
        line.color = (std::fmod(x, 10.0) < .01) ? major : minor; Moss_DebugDrawLine(debug_draw_, &line);
    }
    for (double z = 0; z <= scene_.world_depth; z += 5.0) {
        Moss_DebugLineDesc line; line.from = make_f3(0, .005, z); line.to = make_f3(scene_.world_width, .005, z);
        line.color = (std::fmod(z, 10.0) < .01) ? major : minor; Moss_DebugDrawLine(debug_draw_, &line);
    }
}

void EditorApplication::drawSceneObject(const SceneObject& object, bool selected) {
    const auto p = object.position; const auto s = object.size;
    const Color color = selected ? Color(1.f,.72f,.15f,1.f) : scene_color(object.type);
    Moss_DebugBoxDesc box; box.center = make_f3(p.x,p.y,p.z); box.size = make_f3(s.x,s.y,s.z); box.color = color;
    if (object.type == SceneObjectType::Goal || object.type == SceneObjectType::RobotSpawn || object.type == SceneObjectType::Sensor) {
        Moss_DebugSphereDesc sphere; sphere.center = make_f3(p.x,p.y,p.z); sphere.radius = static_cast<float>(std::max({s.x,s.y,s.z})*.5);
        sphere.color = color; sphere.segments = 20; Moss_DebugDrawSphere(debug_draw_, &sphere);
        if (object.type == SceneObjectType::RobotSpawn) {
            Moss_DebugBoxDesc chassis = box; chassis.size = make_f3(s.x*.85,s.y*.45,s.z*.65); Moss_DebugDrawBox(debug_draw_, &chassis);
            Moss_DebugLineDesc heading; heading.from=make_f3(p.x,p.y+.2,p.z); heading.to=make_f3(p.x,p.y+.2,p.z-s.z*1.8); heading.color=color; heading.thickness=3; Moss_DebugDrawLine(debug_draw_,&heading);
        }
        if (object.type == SceneObjectType::Sensor) {
            for (double offset=-1.0; offset<=1.01; offset+=.5) { Moss_DebugLineDesc ray; ray.from=make_f3(p.x,p.y,p.z); ray.to=make_f3(p.x+offset*5.0,p.y-.25,p.z-8.0); ray.color=Color(.98f,.82f,.22f,.95f); ray.thickness=1.5f; Moss_DebugDrawLine(debug_draw_,&ray); }
        }
    } else {
        Moss_DebugDrawBox(debug_draw_, &box);
        if (object.type == SceneObjectType::Road) {
            const double road_y=p.y+s.y*.55;
            // Dashed centre line follows the road's long X axis; solid edge lines
            // define the drivable width. Roads can be rotated in the data model later.
            const double dash_len=std::max(1.0,std::min(3.0,s.x*.045));
            const double dash_gap=std::max(1.0,dash_len*.7);
            for(double x=p.x-s.x*.45;x<p.x+s.x*.45;x+=dash_len+dash_gap) {
                Moss_DebugLineDesc dash; dash.from=make_f3(x,road_y,p.z); dash.to=make_f3(std::min(x+dash_len,p.x+s.x*.45),road_y,p.z);
                dash.color=Color(.96f,.89f,.64f,1.f); dash.thickness=2.2f; Moss_DebugDrawLine(debug_draw_,&dash);
            }
            for(double side : {-1.0,1.0}) {
                Moss_DebugLineDesc edge; edge.from=make_f3(p.x-s.x*.5,road_y,p.z+side*s.z*.42); edge.to=make_f3(p.x+s.x*.5,road_y,p.z+side*s.z*.42);
                edge.color=Color(.77f,.80f,.82f,1.f); edge.thickness=1.3f; Moss_DebugDrawLine(debug_draw_,&edge);
            }
        } else if (object.type == SceneObjectType::Terrain) {
            // A generated wireframe height-field preview makes terrain visibly editable
            // before the later heightmap/mesh collision backend is introduced.
            constexpr int samples = 16;
            auto height_at = [&](double u, double v) {
                const double hill = std::cos(u * 3.14159265358979323846) * std::cos(v * 3.14159265358979323846);
                return p.y + hill * s.y * 0.5;
            };
            for (int j=0;j<=samples;++j) {
                const double v=static_cast<double>(j)/samples;
                const double z=p.z-s.z*.5+v*s.z;
                for (int i=0;i<samples;++i) {
                    const double u0=static_cast<double>(i)/samples, u1=static_cast<double>(i+1)/samples;
                    Moss_DebugLineDesc wire;
                    wire.from=make_f3(p.x-s.x*.5+u0*s.x,height_at(u0*2.0-1.0,v*2.0-1.0),z);
                    wire.to=make_f3(p.x-s.x*.5+u1*s.x,height_at(u1*2.0-1.0,v*2.0-1.0),z);
                    wire.color=Color(.38f,.82f,.48f,1.f); wire.thickness=.9f;
                    Moss_DebugDrawLine(debug_draw_,&wire);
                }
            }
            for (int i=0;i<=samples;++i) {
                const double u=static_cast<double>(i)/samples;
                const double x=p.x-s.x*.5+u*s.x;
                for (int j=0;j<samples;++j) {
                    const double v0=static_cast<double>(j)/samples, v1=static_cast<double>(j+1)/samples;
                    Moss_DebugLineDesc wire;
                    wire.from=make_f3(x,height_at(u*2.0-1.0,v0*2.0-1.0),p.z-s.z*.5+v0*s.z);
                    wire.to=make_f3(x,height_at(u*2.0-1.0,v1*2.0-1.0),p.z-s.z*.5+v1*s.z);
                    wire.color=Color(.30f,.70f,.39f,1.f); wire.thickness=.9f;
                    Moss_DebugDrawLine(debug_draw_,&wire);
                }
            }
        } else if (object.type == SceneObjectType::Ramp) {
            for (int i=0;i<=8;++i) { const double t=static_cast<double>(i)/8.0; Moss_DebugLineDesc cross; cross.from=make_f3(p.x-s.x*.5,p.y-s.y*.5+s.y*t,p.z-s.z*.5); cross.to=make_f3(p.x+s.x*.5,p.y-s.y*.5+s.y*t,p.z+s.z*.5); cross.color=Color(.96f,.71f,.3f,1.f); Moss_DebugDrawLine(debug_draw_,&cross); }
        }
    }
    if (selected || (show_collision_overlay_ && object.collision_enabled)) {
        Moss_DebugBoxDesc bounds; bounds.center=make_f3(p.x,p.y,p.z); bounds.size=make_f3(s.x*1.025,s.y*1.025,s.z*1.025);
        bounds.color=selected?Color(1.f,.78f,.15f,1.f):Color(.96f,.28f,.2f,.8f); Moss_DebugDrawBox(debug_draw_,&bounds);
    }
}

void EditorApplication::handleViewportPicking() {
    if (!Moss_IsMouseJustPressed(Mouse::LEFT)) return;
    int mx=0,my=0; Moss_GetMousePosition(&mx,&my);
    const int width=std::max(Moss_GetWindowWidth(),1),height=std::max(Moss_GetWindowHeight(),1);
    const int left=280, right=width-300, top=42, bottom=height-175;
    if(mx<left||mx>right||my<top||my>bottom||right<=left||bottom<=top)return;

    // Cast the clicked screen pixel into the ground plane using the same fixed
    // perspective camera used by the Moss viewport. This is more accurate than
    // assuming a top-down orthographic view; elevated mesh picking is future work.
    const double pi=3.14159265358979323846;
    const double fov=45.0*pi/180.0;
    const double aspect=static_cast<double>(width)/static_cast<double>(height);
    const double cx=scene_.world_width*.5, cz=scene_.world_depth*.5;
    const double cam_x=cx+scene_.world_width*.28, cam_y=kCameraHeight, cam_z=cz+scene_.world_depth*.36;
    double fx=cx-cam_x, fy=-cam_y, fz=cz-cam_z;
    const double fl=std::sqrt(fx*fx+fy*fy+fz*fz); if(fl<1e-8)return;
    fx/=fl; fy/=fl; fz/=fl;
    double rx=-fz, rz=fx;
    const double rl=std::sqrt(rx*rx+rz*rz); if(rl<1e-8)return;
    rx/=rl; rz/=rl;
    const double ux=-rz*fy, uy=rz*fx-rx*fz, uz=rx*fy;
    const double ndc_x=2.0*static_cast<double>(mx)/static_cast<double>(width)-1.0;
    const double ndc_y=1.0-2.0*static_cast<double>(my)/static_cast<double>(height);
    const double tan_half=std::tan(fov*.5);
    double dx=fx+rx*ndc_x*tan_half*aspect+ux*ndc_y*tan_half;
    double dy=fy+uy*ndc_y*tan_half;
    double dz=fz+rz*ndc_x*tan_half*aspect+uz*ndc_y*tan_half;
    const double dl=std::sqrt(dx*dx+dy*dy+dz*dz); if(dl<1e-8)return;
    dx/=dl; dy/=dl; dz/=dl;
    if(std::abs(dy)<1e-7)return;
    const double t=-cam_y/dy; if(t<=0)return;
    const double wx=std::clamp(cam_x+t*dx,0.0,scene_.world_width);
    const double wz=std::clamp(cam_z+t*dz,0.0,scene_.world_depth);

    if(placement_mode_) {
        const auto index=addSceneObject(placement_type_);
        auto& object=scene_.objects[index];
        object.position.x=std::clamp(wx,object.size.x*.5,std::max(object.size.x*.5,scene_.world_width-object.size.x*.5));
        object.position.z=std::clamp(wz,object.size.z*.5,std::max(object.size.z*.5,scene_.world_depth-object.size.z*.5));
        placement_mode_=false;
        status_message_="Placed " + object.name + " in scene";
        return;
    }

    // Skip the very large floor while testing selectable objects back-to-front.
    for(std::size_t reverse=scene_.objects.size(); reverse>0; --reverse) {
        const std::size_t i=reverse-1;
        const auto& o=scene_.objects[i];
        if(o.type==SceneObjectType::Ground) continue;
        if(std::abs(wx-o.position.x)<=o.size.x*.5 && std::abs(wz-o.position.z)<=o.size.z*.5) {
            selected_object_index_=i; status_message_="Selected: "+o.name; return;
        }
    }
    for(std::size_t i=0;i<scene_.objects.size();++i) {
        const auto& o=scene_.objects[i];
        if(std::abs(wx-o.position.x)<=o.size.x*.5 && std::abs(wz-o.position.z)<=o.size.z*.5) {
            selected_object_index_=i; status_message_="Selected: "+o.name; return;
        }
    }
}

void EditorApplication::selectObject(std::size_t i) noexcept { if(i<scene_.objects.size())selected_object_index_=i; }
SceneObject* EditorApplication::selectedObject() noexcept { return selected_object_index_<scene_.objects.size()?&scene_.objects[selected_object_index_]:nullptr; }
const SceneObject* EditorApplication::selectedObject() const noexcept { return selected_object_index_<scene_.objects.size()?&scene_.objects[selected_object_index_]:nullptr; }
std::size_t EditorApplication::addSceneObject(SceneObjectType t) { const auto i=scene_.add(t,std::string(to_string(t))+"_"+std::to_string(scene_.next_id));selected_object_index_=i;status_message_="Added "+scene_.objects[i].name;return i; }
void EditorApplication::removeSelectedObject() { if(scene_.remove(selected_object_index_)){selected_object_index_=scene_.objects.empty()?0:std::min(selected_object_index_,scene_.objects.size()-1);status_message_="Removed selected object";}else status_message_="No scene object selected"; }
void EditorApplication::duplicateSelectedObject() { const auto i=scene_.duplicate(selected_object_index_);if(i<scene_.objects.size()){selected_object_index_=i;status_message_="Duplicated scene object";}else status_message_="Select an object to duplicate"; }
void EditorApplication::beginPlacement(SceneObjectType type) noexcept {
    placement_type_ = type;
    placement_mode_ = true;
    status_message_ = std::string("Placement tool: ") + to_string(type) + " — click in viewport";
}
void EditorApplication::cancelPlacement() noexcept {
    placement_mode_ = false;
    status_message_ = "Selection tool active";
}
bool EditorApplication::saveScene() { std::string error;if(!scene_.save_json("vivarium_scene.json",&error)){status_message_="Scene save failed: "+error;return false;}status_message_="Saved vivarium_scene.json";return true; }
bool EditorApplication::loadScene() { SceneDocument loaded;std::string error;if(!loaded.load_json("vivarium_scene.json",&error)){status_message_="Scene load failed: "+error;return false;}scene_=std::move(loaded);selected_object_index_=scene_.objects.empty()?0:std::min(selected_object_index_,scene_.objects.size()-1);last_window_width_=last_window_height_=0;updateViewportCamera();status_message_="Loaded vivarium_scene.json";return true; }

} // namespace vivarium::editor
