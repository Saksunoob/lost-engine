#include "square.hpp"
#include "bundles.hpp"

using namespace engine;

void test(Scene& scene) {
    Input& input = scene.getResource<Input>();
    Components sliced = scene.GetComponents().With<SlicedTexture>();
    if (input.getMouseScroll() != IVector2(0, 0)) {
        sliced[0].Get<SlicedTexture>()->pixel_size = sliced[0].Get<SlicedTexture>()->pixel_size + Vector2(input.getMouseScroll().y, input.getMouseScroll().y)/10.;
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
    texture.addComponent(UITransform(Point::center(), Point::center(), UNIT_PIXELS, {0, 0}, UNIT_PERCENT, {90, 90}, 0));
    texture.addComponent(ZLayer(0, 0.1));
    texture.addBundle(Bundles::quadMeshBundle());
    texture.addComponent(Texture("../src/textures/test.png", Texture::Filter::NEAREST, Texture::AddressMode::CLAMP_TO_EDGE));
    texture.addComponent(SlicedTexture({{64, 64, 64, 64}}, IVector2(1, 1)));

    Engine::run();
}