#include "square.hpp"

class Test : public engine::Component {
    public:
        int index;
        Test(int i): index(i) {};
};

int main() {
    engine::Engine::init("Square", engine::Vector2(800, 600));
    engine::Scene& main_scene = engine::Engine::addScene("main");
    engine::Entity entity = main_scene.createEntity();
    main_scene.addComponent(entity, Test(1));
    engine::Components<Test> test = main_scene.getComponent<Test>();
    main_scene.addStageAt("update", 0);
    main_scene.getStage("update").addSystem(testSystem);
    engine::Engine::run();
}

void testSystem() {
    engine::Logger::log("works");
}