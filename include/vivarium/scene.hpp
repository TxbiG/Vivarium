#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace vivarium {
struct Vec3 { double x{0.0}, y{0.0}, z{0.0}; };
enum class SceneObjectType { Ground, Road, Terrain, Ramp, Wall, Obstacle, RobotSpawn, Goal, Sensor };
struct SceneObject {
    std::uint64_t id{0}; SceneObjectType type{SceneObjectType::Obstacle}; std::string name{"Object"};
    Vec3 position{0,0,0}, size{1,1,1}; double mass{1.0}, friction{0.6}, restitution{0.0};
    bool fixed{true}, collision_enabled{true};
};
struct SceneDocument {
    std::uint32_t schema_version{1}; double world_width{100.0}, world_depth{100.0}, gravity_y{-9.81};
    std::vector<SceneObject> objects; std::uint64_t next_id{1};
    std::size_t add(SceneObjectType type, const std::string& name);
    bool remove(std::size_t index); std::size_t duplicate(std::size_t index);
    bool validate(std::string* error=nullptr) const;
    bool save_json(const std::string& path, std::string* error=nullptr) const;
    bool load_json(const std::string& path, std::string* error=nullptr);
};
const char* to_string(SceneObjectType type) noexcept;
bool scene_object_type_from_string(const std::string& name, SceneObjectType& type) noexcept;
SceneDocument make_default_scene();
}
