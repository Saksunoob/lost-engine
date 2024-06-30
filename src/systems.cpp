#include "systems.hpp"

static std::unique_ptr<engine::Pipeline> pipeline{};

void engine::renderMeshes(Scene& scene) {
    if (!pipeline) {
        auto pipelineConfig = PipelineConfig::defaultConfig(Engine::getSwapChain()->width(), Engine::getSwapChain()->height());
        pipelineConfig.renderPass = Engine::getSwapChain()->getRenderPass();

        pipeline = std::make_unique<Pipeline>(Engine::getDevice(), "src/shaders/UVMesh", pipelineConfig);
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
    
    Components meshes = scene.getComponent<UVMesh>();
    Components textures = scene.getComponent<Texture>();

    for (unsigned i = 0; i < transforms.size(); i++) {
        if (transforms[i] != nullptr && meshes[i] != nullptr && textures[i] != nullptr) {
            UVMesh* mesh = meshes[i];

            if (mesh->vertexBuffer == nullptr) {
                Device& device = Engine::getDevice();

                mesh->vertexCount = static_cast<unsigned>(mesh->vertices.size());

                VkDeviceSize bufferSize = sizeof(mesh->vertices[0]) * mesh->vertexCount;
                device.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    mesh->vertexBuffer, mesh->vertexBufferMemory);

                void* data;
                vkMapMemory(device.device(), mesh->vertexBufferMemory, 0, bufferSize, 0, &data);
                memcpy(data, mesh->vertices.data(), static_cast<size_t>(bufferSize));
                vkUnmapMemory(device.device(), mesh->vertexBufferMemory);
            }

            VkBuffer buffers[] = {mesh->vertexBuffer};
            VkDeviceSize offset[] = {0};
            vkCmdBindVertexBuffers(Engine::getCurrentCommandBuffer(), 0, 1, buffers, offset);
            vkCmdDraw(Engine::getCurrentCommandBuffer(), mesh->vertexCount, 1, 0, 0);
        }
    }
}