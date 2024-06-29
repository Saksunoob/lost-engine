#include "square.hpp"

int main() {
    engine::Engine::init("Square", engine::Vector2(800, 600));
    engine::Scene& main_scene = engine::Engine::addScene("main");

    engine::Entity camera = main_scene.createEntity();
    main_scene.addComponent(camera, engine::Camera(true));
    main_scene.addComponent(camera, engine::GlobalTransform(engine::Vector2(0, 0), engine::Vector2(1, 1), 0));

    engine::Entity square = main_scene.createEntity();
    std::vector<engine::Vector2> vertex_positions = {
        {0.5, 0.5},
        {0.5, -0.5},
        {-0.5, -0.5},
        {-0.5, 0.5},
    };
    std::vector<engine::Vector2> vertex_UVs = {
        {1, 1},
        {1, 0},
        {0, 0},
        {0, 1},
    };
    std::vector<unsigned> indices = {
        0, 1, 3,
        1, 2, 3
    };
    main_scene.addComponent(square, engine::UVMesh(vertex_positions,vertex_UVs, indices));
    main_scene.addComponent(square, engine::Texture(engine::Color(1, 1, 0), engine::IVector2(16, 16)));
    main_scene.addComponent(square, engine::GlobalTransform(engine::Vector2(0, 0), engine::Vector2(400, 400), 0));

    main_scene.addStageAt("render", 0);
    main_scene.getStage("render").addSystem(engine::renderUVMeshes);
    engine::Engine::run();
}