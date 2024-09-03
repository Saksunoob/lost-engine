#include "square.hpp"
#include "bundle.hpp"

using namespace engine;

int main() {
    Engine::init("Square", IVector2(800, 600));
    Scene& main_scene = Engine::addScene("main", true);

    Entity camera = main_scene.createEntity();
    camera.addComponent(Camera(true));
    camera.addComponent(GlobalTransform(Vector2(0, 0), Vector2(1, 1), 0));

    Vertices vertices({
        {0.5, 0.5},
        {-0.5, -0.5},
        {0.5, -0.5},
        {-0.5, 0.5}
    });
    UVs uvs({
        {1, 1},
        {0, 0},
        {1, 0},
        {0, 1}
    });
    Indices indices({
        0, 1, 2, 0, 3, 1
    });

    Bundle test{vertices, indices};

    Entity ui1 = main_scene.createEntity();
    UITransform& parent = ui1.addComponent(UITransform(UNIT_PERCENT, Vector2(0, 0), UNIT_PERCENT, Vector2(50, 50), 0));
    ui1.addBundle(test);
    ui1.addComponent(Color(1, 0, 0));

    Entity ui2 = main_scene.createEntity();
    UITransform& child = ui2.addComponent(UITransform(UNIT_PERCENT, Vector2(25, 25), UNIT_PERCENT, Vector2(50, 50), 0));
    parent.addChild(child);
    ui2.addBundle(test);
    ui2.addComponent(Color(0, 0, 1));

    Engine::run();
}