#include "square.hpp"

using namespace engine;

int main() {
    Engine::init("Square", IVector2(800, 600));
    Scene& main_scene = Engine::addScene("main", true);

    Entity camera = main_scene.createEntity();
    camera.addComponent(Camera(true));
    camera.addComponent(GlobalTransform(Vector2(0, 0), Vector2(1, 1), 0));

    std::vector<Vector2> vertices = {
        {0.5, 0.5},
        {-0.5, -0.5},
        {0.5, -0.5},
        {-0.5, 0.5}
    };
    std::vector<Vector2> uvs {
        {1, 1},
        {0, 0},
        {1, 0},
        {0, 1}
    };
    std::vector<unsigned> indices = {
        0, 1, 2, 0, 3, 1
    };

    Entity square = main_scene.createEntity();
    square.addComponent(Mesh(vertices, indices));
    square.addComponent(Color(1, 0, 0));
    square.addComponent(GlobalTransform(Vector2(50, 0), Vector2(200, 200), 1));
    square.addComponent(ZLayer(0, 0.2));
    Texture texture("../src/textures/test.png");

    for (int i = 0; i < 1; i++) {
        Entity square2 = main_scene.createEntity();
        square2.addComponent(Mesh(vertices, indices));
        square2.addComponent(UVs(uvs));
        square2.addComponent(GlobalTransform(Vector2(-50, 0), Vector2(100, 100), 0));
        square2.addComponent(texture);
        square2.addComponent(ZLayer(0, 0.1));
    }
    

    Engine::run();
}