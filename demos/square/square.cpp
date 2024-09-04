#include "square.hpp"
#include "bundles.hpp"

using namespace engine;

int main() {
    Engine::init("Square", IVector2(800, 600));
    Scene& main_scene = Engine::addScene("main", true);

    Entity camera = main_scene.createEntity();
    camera.addComponent(Camera(true));
    camera.addComponent(GlobalTransform(Vector2(0, 0), Vector2(1, 1), 0));

    Entity ui1 = main_scene.createEntity();
    ui1.addComponent(UITransform(Point::topLeft(), Point::topLeft(), UNIT_PERCENT, Vector2(0, 0), UNIT_PERCENT, Vector2(50, 50), 0));
    ui1.addComponent(ZLayer(0, 0.1));
    ui1.addBundle(Bundles::quadMeshBundle());
    ui1.addComponent(Texture("../src/textures/test.png"));

    Entity ui2 = main_scene.createEntity(ui1);
    ui2.addComponent(UITransform(Point::right(), Point::bottomRight(), UNIT_PERCENT, Vector2(0, 0), UNIT_PERCENT, Vector2(50, 50), 0));
    ui2.addBundle(Bundles::quadMeshBundle());
    ui2.addComponent(Color(0, 0, 1));

    Engine::run();
}