#include "systems.hpp"

static std::unique_ptr<engine::Pipeline> pipeline{};

void engine::renderMeshes(Scene& scene) {
    if (!pipeline) {
        auto pipelineConfig = PipelineConfig::defaultConfig(Engine::getSwapChain()->width(), Engine::getSwapChain()->height());
        pipelineConfig.renderPass = Engine::getSwapChain()->getRenderPass();

        pipeline = std::make_unique<Pipeline>(Engine::getDevice(), "src/shaders/Mesh", pipelineConfig);
    }
    pipeline->bind(Engine::getCurrentCommandBuffer());

    Components cameras = scene.getComponent<Camera>();
    Components transforms = scene.getComponent<GlobalTransform>();
    
    Entity main_camera;
    for (unsigned i = 0; i < cameras.size(); i++) {
        if (cameras[i] != nullptr && cameras[i]->main) {
            if (transforms[i] != nullptr) {
                main_camera = i;
                break;
            }
            Logger::logWarning("Main camera has no GlobalTransform component");            
        }
        if (i+1 == cameras.size()) {
            Logger::logWarning("No main camera");
            return;
        }
    }
    
    Components meshes = scene.getComponent<Mesh>();
    Components textures = scene.getComponent<Texture>();

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    for (unsigned i = 0; i < transforms.size(); i++) {
        if (transforms[i] != nullptr && meshes[i] != nullptr && textures[i] != nullptr) {
            Mesh* mesh = meshes[i];

            Device& device = Engine::getDevice();

            if (mesh->vertexBuffer == nullptr) {
                mesh->createBuffers(device);
            }

            VkBuffer buffers[] = {mesh->vertexBuffer};
            VkDeviceSize offset[] = {0};
            vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offset);
            vkCmdBindIndexBuffer(cmdBuffer, mesh->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(Engine::getCurrentCommandBuffer(), mesh->indices.size(), 1, 0, 0, 0);
        }
    }
}