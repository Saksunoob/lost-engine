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
    texture.addBundle(Bundles::UITransformBundle(UITransform(Point::center(), Point::center(), UNIT_PIXELS, {0, 0}, UNIT_PERCENT, {50, 50}, 0)));
    texture.addBundle(Bundles::quadMeshBundle());
    texture.addComponent(Texture("../src/textures/test.png", Texture::Filter::NEAREST, Texture::AddressMode::CLAMP_TO_EDGE));
    texture.addComponent(SlicedTexture({{64, 64, 64, 64}}, IVector2(1, 1)));
    texture.addComponent(Collider::square(AABB({-0.5, -0.5}, {0.5, 0.5})));

    Engine::run();
}