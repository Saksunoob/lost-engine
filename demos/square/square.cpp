#include "square.hpp"

int main() {
    engine::Engine::init("Square", engine::IVector2(800, 600));
    engine::Scene& main_scene = engine::Engine::addScene("main", true);

    engine::Entity camera = main_scene.createEntity();
    main_scene.addComponent(camera, engine::Camera(true));
    main_scene.addComponent(camera, engine::GlobalTransform(engine::Vector2(0, 0), engine::Vector2(1, 1), 0));

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
    main_scene.addComponent(square, engine::Mesh(vertices, indices));
    main_scene.addComponent(square, engine::Color(1, 0, 0));
    main_scene.addComponent(square, engine::GlobalTransform(engine::Vector2(100, 0), engine::Vector2(200, 200), 1));
    main_scene.addComponent(square, engine::Texture{"../src/textures/test.png"});

    engine::Entity square2 = main_scene.createEntity();
    main_scene.addComponent(square2, engine::Mesh(vertices, indices));
    main_scene.addComponent(square2, engine::UVs(uvs));
    main_scene.addComponent(square2, engine::GlobalTransform(engine::Vector2(-100, 0), engine::Vector2(100, 100), 0));
    main_scene.addComponent(square2, engine::Texture{"../src/textures/test.png"});

    engine::Engine::run();
}