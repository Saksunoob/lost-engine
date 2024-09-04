#include "square.hpp"
#include "bundles.hpp"

using namespace engine;

int main() {
    Engine::init("Square", IVector2(800, 600));
    Scene& main_scene = Engine::addScene("main", true);

    Entity camera = main_scene.createEntity();
    camera.addComponent(Camera(true));
    camera.addComponent(GlobalTransform(Vector2(0, 0), Vector2(1, 1), 0));

    Entity texture = main_scene.createEntity();
    texture.addComponent(GlobalTransform({0, 0}, {100, 100}, 0));
    texture.addComponent(ZLayer(0, 0.1));
    texture.addBundle(Bundles::quadMeshBundle());
    texture.addComponent(TextureAtlas({2, 2},Texture("../src/textures/test.png")));
    texture.addComponent(TextureIndex(0));

    Engine::run();
}