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

    Mesh mesh(vertices, indices);
    Texture atlas("../src/textures/test.png");

    Entity tilemap = main_scene.createEntity();

    TileMap tilemap_c({5,5});
    for (int i = 0; i < 25; i++) {
        tilemap_c.setTile({i%5,i/5},i%4);
    }

    tilemap.addComponent(GlobalTransform({0, 0}, {200, 200}, 0));
    tilemap.addComponent(ZLayer(0,0));
    tilemap.addComponent(mesh);
    tilemap.addComponent(UVs(uvs));
    tilemap.addComponent(tilemap_c);
    tilemap.addComponent(TextureAtlas{{2,2},atlas});

    Engine::run();
}