#include "EnvironmentInspector.hpp"
#include "../EditorApplication.hpp"
#include <Moss/Moss_GUI.h>
#include <algorithm>
#include <cmath>

namespace vivarium::editor {
void DrawEnvironmentInspector(EditorApplication& app) {
    if (BeginWindow("Vivarium | Object Inspector", nullptr, 0)) {
        auto* object = app.selectedObject();
        if (object) {
            Text("SELECTED OBJECT");
            Text("ID: %llu", static_cast<unsigned long long>(object->id));
            Text("Name: %s", object->name.c_str());
            Text("Type: %s", to_string(object->type));
            Text("TRANSFORM (world units)");
            InputDouble("Position X", &object->position.x, .25, 1.0, "%.3f");
            InputDouble("Position Y", &object->position.y, .1, .5, "%.3f");
            InputDouble("Position Z", &object->position.z, .25, 1.0, "%.3f");
            Text("SIZE");
            InputDouble("Width", &object->size.x, .25, 1.0, "%.3f");
            InputDouble("Height", &object->size.y, .25, 1.0, "%.3f");
            InputDouble("Depth", &object->size.z, .25, 1.0, "%.3f");
            object->size.x=std::max(object->size.x,.05);
            object->size.y=std::max(object->size.y,.05);
            object->size.z=std::max(object->size.z,.05);
            Text("PHYSICS");
            InputDouble("Mass", &object->mass, .1, 1.0, "%.3f");
            InputDouble("Friction", &object->friction, .05, .25, "%.3f");
            InputDouble("Restitution", &object->restitution, .05, .25, "%.3f");
            object->mass=std::max(object->mass,.001);
            object->friction=std::max(object->friction,0.0);
            object->restitution=std::clamp(object->restitution,0.0,1.0);
            if (RadioButton("Fixed object (click to toggle)", object->fixed)) object->fixed=!object->fixed;
            if (RadioButton("Collision enabled (click to toggle)", object->collision_enabled)) object->collision_enabled=!object->collision_enabled;
        } else {
            Text("No object selected. Add an object or select one in the hierarchy.");
        }
        Text("SIMULATION STATE");
        Text("Environment: %s", app.environmentId().c_str());
        const auto spec=app.spec();
        Text("Action dimensions: %d", static_cast<int>(spec.action_size));
        Text("Observation dimensions: %d", static_cast<int>(spec.observation_size));
        Text("Observation values: %d", static_cast<int>(app.observation().size()));
        const auto& obs=app.observation();
        for(std::size_t i=0;i<obs.size() && i<20;++i) Text("obs[%d] = %.5f",static_cast<int>(i),obs[i]);
    }
    EndWindow();
}
}
