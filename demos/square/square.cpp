#include "square.hpp"

using namespace engine;

static std::vector<u_char> image = {0, 64, 128, 192};
static std::vector<float> f_image = {0, 64, 128, 192};

void updateTexture(Scene& scene) {
    Components textures = scene.GetWithComponents<Texture>();
    for (int i = 0; i < f_image.size(); i++) {
        f_image[i] += 0.1;
        image[i] = static_cast<u_char>(f_image[i]);
    }
    textures[0].Get<Texture>()->getData().update(image.data());
}

int main() {
    Engine::init("Square", IVector2(800, 600));
    Scene& main_scene = Engine::addScene("main", true);

    main_scene.getStage("render")->addSystem(updateTexture);

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
    Texture texture(image.data(), {2, 2}, TextureFormat::Srgb(1));

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