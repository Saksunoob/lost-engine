#include "stage.hpp"

using namespace engine;

void Stage::addSystem(System system) {
    systems.push_back(system);
}
void Stage::removeSystem(System system) {
    for (unsigned i = 0; i < systems.size(); i++) {
        if (system == systems[i]) {
            systems.erase(systems.begin() + i);
            return;
        }
    }
    Logger::logWarning("Trying to remove a nonexisting system");
}

void Stage::execute(Scene& scene) {
    for (System system : systems) {
        system(scene);
    }
}