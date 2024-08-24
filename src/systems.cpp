#include "systems.hpp"
#include "resources.hpp"
#include "buffer.hpp"

void engine::renderColorMeshes(Scene& scene) {
    struct Data {
        glm::mat4 matrix;
        Color color;
    };
    static Shader shader("shaders/ColorMesh", {{VAR_VEC2}}, 0, {Binding::Uniform(sizeof(Data)), Binding::Sampler()});
    Components validCameras = scene.GetWithComponents<Camera, GlobalTransform>();
    Component<Camera>& cameras = validCameras.Get<Camera>();
    unsigned main_camera;
    for (int i = 0; i < cameras.size(); i++) {
        if (cameras[i]->main) {
            main_camera = i;
            break;    
        }
        if (i+1 == cameras.size()) {
            Logger::logWarning("No main camera");
            return;
        }
    }
    Components colorMeshes = scene.GetWithComponents<GlobalTransform, Mesh, Color, Texture>();

    glm::mat4 proj = cameras[main_camera]->getProjectionMatrix(*validCameras.Get<GlobalTransform>()[main_camera], Engine::getWindowSize());

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();
    shader.bind();
    for (unsigned i = 0; i < colorMeshes.size(); i++) {
        EntityComponents colorMesh = colorMeshes[i];
        Mesh& mesh = *colorMesh.Get<Mesh>();
        mesh.vertexBuffer->bind();
        mesh.indexBuffer->bind();

        Data data {
            proj * colorMesh.Get<GlobalTransform>()->getTransformationMatrix(),
            *colorMesh.Get<Color>()
        };
        shader.writeUniformBinding(0, 0, &data);
        shader.writeSamplerBinding(0, 1, *colorMesh.Get<Texture>());
        shader.bindSet(0);
        
        vkCmdDrawIndexed(cmdBuffer, colorMesh.Get<Mesh>()->indices.size(), 1, 0, 0, 0);
    }
}

void engine::timeSystem(Scene& scene) {
    Time& time = scene.getResource<Time>();
    time.newFrame();
    Logger::log("FPS: " + std::to_string(1/time.deltaTime()));
}

void engine::pollSDLEvents(Scene& scene) {
    scene.getResource<Input>().newFrame();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case (SDL_QUIT):
                Engine::quit();
                break;
            case (SDL_WINDOWEVENT): {
                break;
            }
            case (SDL_KEYDOWN): case (SDL_KEYUP):
                scene.getResource<Input>().handleKeyEvent(event);
                break;
        }
    }
}