#include "systems.hpp"

void engine::renderColorMeshes(Scene& scene) {
    struct Push {
        glm::mat4 matrix;
        Color color;
    };

    static Shader shader("shaders/ColorMesh", {{VAR_VEC2}}, sizeof(Push));

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
    Components colorMeshes = scene.GetWithComponents<GlobalTransform, Mesh, Color>();

    glm::mat4 proj = cameras[main_camera]->getProjectionMatrix(*validCameras.Get<GlobalTransform>()[main_camera], Engine::getWindowSize());

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();
    shader.bind();
    for (unsigned i = 0; i < colorMeshes.size(); i++) {
        EntityComponents colorMesh = colorMeshes[i];
        shader.vertexBuffer().bind(colorMesh.Get<Mesh>()->vertices);
        shader.indexBuffer().bind(colorMesh.Get<Mesh>()->indices);

        Push push {
            proj * colorMesh.Get<GlobalTransform>()->getTransformationMatrix(),
            *colorMesh.Get<Color>()
        };
        shader.pushConstant(&push, sizeof(Push));

        vkCmdDrawIndexed(cmdBuffer, colorMesh.Get<Mesh>()->indices.size(), 1, 0, 0, 0);
    }
}