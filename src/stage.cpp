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
    Logger::logError("Trying to remove a system that doesn't exist!");
}

void Stage::execute() {
    for (System system : systems) {
        system();
    }
}