#include "square.hpp"

int main() {
    engine::Engine::init("Square", engine::IVector2(800, 600));
    engine::Scene& main_scene = engine::Engine::addScene("main", true);

    engine::Entity camera = main_scene.createEntity();
    camera.addComponent(engine::Camera(true));
    camera.addComponent(engine::GlobalTransform(engine::Vector2(0, 0), engine::Vector2(1, 1), 0));

    std::vector<engine::Vector2> vertices = {
        {0.5, 0.5},
        {-0.5, -0.5},
        {0.5, -0.5},
        {-0.5, 0.5}
    };
    std::vector<engine::Vector2> uvs {
        {1, 1},
        {0, 0},
        {1, 0},
        {0, 1}
    };
    std::vector<unsigned> indices = {
        0, 1, 2, 0, 3, 1
    };

    engine::Entity square = main_scene.createEntity();
    square.addComponent(engine::Mesh(vertices, indices));
    square.addComponent(engine::Color(1, 0, 0));
    square.addComponent(engine::GlobalTransform(engine::Vector2(100, 0), engine::Vector2(200, 200), 1));
    square.addComponent(engine::Texture{"../src/textures/test.png"});

    engine::Entity square2 = main_scene.createEntity();
    square2.addComponent(engine::Mesh(vertices, indices));
    square2.addComponent(engine::UVs(uvs));
    square2.addComponent(engine::GlobalTransform(engine::Vector2(-100, 0), engine::Vector2(100, 100), 0));
    square2.addComponent(engine::Texture{"../src/textures/test.png"});

    engine::Engine::run();
}