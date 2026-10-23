#include "EditorApplication.hpp"
#include <exception>
#include <iostream>

int main() {
    try {
        vivarium::editor::EditorApplication app;
        return app.run();
    } catch (const std::exception& ex) {
        std::cerr << "Vivarium Editor failed: " << ex.what() << '\n';
        return 1;
    }
}
