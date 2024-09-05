#include "square.hpp"
#include "bundles.hpp"

using namespace engine;

void test(Scene& scene) {
    Input& input = scene.getResource<Input>();
    if (input.getMouseButtonJustPressed(1)) {
        Logger::log("mouse 1");
    }
    if (input.getMouseScroll() != IVector2(0, 0)) {
        Logger::log(std::format("scroll: {{x: {}, y: {}}}", input.getMouseScroll().x, input.getMouseScroll().y));
    }
    if (input.getMouseDelta() != IVector2(0, 0)) {
        Logger::log(std::format("motion: {{x: {}, y: {}}}", input.getMouseDelta().x, input.getMouseDelta().y));
        Logger::log(std::format("mouse: {{x: {}, y: {}}}", input.getMousePos().x, input.getMousePos().y));
    }
}

int main() {
    Engine::init("Square", IVector2(800, 600));
    Scene& main_scene = Engine::addScene("main", true);

    main_scene.getStage("update")->addSystem(test);

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