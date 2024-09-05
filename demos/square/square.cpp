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
    texture.addComponent(GlobalTransform({-50, 0}, {100, 100}, 0));
    texture.addComponent(ZLayer(0, 0.1));
    texture.addBundle(Bundles::quadMeshBundle());
    texture.addComponent(TextureAtlas({2, 2},Texture("../src/textures/test.png", Texture::Filter::NEAREST)));
    texture.addComponent(TextureIndex(0));

    Entity texture2 = main_scene.createEntity();
    texture2.addComponent(GlobalTransform({50, 0}, {100, 100}, 0));
    texture2.addComponent(ZLayer(0, 0.1));
    texture2.addBundle(Bundles::quadMeshBundle());
    texture2.addComponent(TextureAtlas({2, 2},Texture("../src/textures/test.png", Texture::Filter::LINEAR)));
    texture2.addComponent(TextureIndex(1));

    Engine::run();
}