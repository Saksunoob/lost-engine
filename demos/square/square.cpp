#include "square.hpp"

using namespace engine;

static int size = 512;
static int seed = 0;
static float freq = 0.01;
static bool changed = false;

void updateNoise(Scene& scene) {
    Input& input = scene.getResource<Input>();
    if (input.getKey(SDLK_RIGHT)) {
        freq *= 1.01;
        changed = true;
    }
    if (input.getKey(SDLK_LEFT)) {
        freq /= 1.01;
        changed = true;
    }
    if (input.getKeyJustPressed(SDLK_SPACE)) {
        seed += 1;
        changed = true;
    }


    if (changed) {
        Components textures = scene.GetWithComponents<Texture>();
        std::vector<u_char> noise = PerlinNoise::generate_char(size, freq, seed);
        textures[0].Get<Texture>()->getData().update(noise.data());
        changed = false;
    }
}

int main() {
    Engine::init("Square", IVector2(800, 600));
    Scene& main_scene = Engine::addScene("main", true);

    main_scene.getStage("render")->addSystem(updateNoise);

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
    std::vector<u_char> noise = PerlinNoise::generate_char(size, freq, seed);
    Texture texture(noise.data(), {size, size}, TextureFormat::Srgb(1));

    Entity square2 = main_scene.createEntity();
    square2.addComponent(Mesh(vertices, indices));
    square2.addComponent(UVs(uvs));
    square2.addComponent(GlobalTransform(Vector2(0, 0), Vector2(500, 500), 0));
    square2.addComponent(texture);
    square2.addComponent(ZLayer(0, 0.1));
    

    Engine::run();
}