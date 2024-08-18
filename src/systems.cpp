#include "systems.hpp"
static engine::Shader* shader;
static bool shader_initialized = false;
static std::vector<VkBuffer> uniformBuffers;
static std::vector<VkDeviceMemory> uniformBuffersMemory;
static std::vector<void*> uniformBuffersMapped;

static VkDescriptorPool descriptorPool = nullptr;
static VkDescriptorSetLayout descriptorSetLayout;
std::vector<VkDescriptorSet> descriptorSets;

void engine::renderMeshes(Scene& scene) {
    struct Push {
        glm::mat4 matrix;
        Color color;
    };

    if (!shader_initialized) {
        shader = new Shader("shaders/Mesh", {{VAR_VEC2}}, sizeof(Push));
        shader_initialized = true;
    }

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
    Components colors = scene.getComponent<Color>();

    glm::mat4 proj = cameras[main_camera]->getProjectionMatrix(*transforms[main_camera], Engine::getWindowSize());

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();
    shader->bind();
    for (unsigned i = 0; i < transforms.size(); i++) {
        if (transforms[i] != nullptr && meshes[i] != nullptr && colors[i] != nullptr) {
            Mesh* mesh = meshes[i];
            shader->vertexBuffer().bind(mesh->vertices);
            shader->indexBuffer().bind(mesh->indices);

            glm::mat4 transform = transforms[i]->getTransformationMatrix();
            Push push {
                proj * transform,
                *colors[i]
            };
            shader->pushConstant(&push, sizeof(Push));

            vkCmdDrawIndexed(cmdBuffer, mesh->indices.size(), 1, 0, 0, 0);
        }
    }
}