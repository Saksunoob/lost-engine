#include "systems.hpp"
#include "resources.hpp"
#include "buffer.hpp"
#include "components.hpp"
#include <chrono>
#include <functional>

void engine::renderColorMeshes(Scene& scene) {

    const int ARRAY_SIZE = 10000;

    struct Data {
        glm::mat4 matrix[ARRAY_SIZE];
        Color color[ARRAY_SIZE];
    };
    static Shader shader("shaders/ColorMesh", {{{VAR_VEC2}}}, 0, {Binding::Uniform(sizeof(Data))});
    Components validCameras = scene.GetComponents().With<Camera, GlobalTransform>();
    Component<Camera> cameras = validCameras.Get<Camera>();
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
    Components colorMeshes = scene.GetComponents().With<GlobalTransform, ZLayer, Mesh, Color>();

    glm::mat4 proj = cameras[main_camera]->getProjectionMatrix(validCameras.Get<GlobalTransform>()[main_camera], Engine::getWindowSize());

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    std::unordered_map<int, std::vector<EntityComponents>> meshes{};

    for (unsigned i = 0; i < colorMeshes.size(); i++) {
        meshes[colorMeshes[i].Get<Mesh>()->mesh_id].push_back(colorMeshes[i]);
    }

    shader.bind();
    for (auto& [id, components] : meshes) {
        int arrays = (components.size()-1)/ARRAY_SIZE+1;

        for (int a = 0; a < arrays; a++) {
            components[0].Get<Mesh>()->vertexBuffer->bind();
            components[0].Get<Mesh>()->indexBuffer->bind();

            Data data;
            for (int i = 0; i < std::min(static_cast<int>(components.size()-a*ARRAY_SIZE),ARRAY_SIZE); i++) {
                data.matrix[i] = proj * components[a*ARRAY_SIZE+i].Get<GlobalTransform>()->getTransformationMatrix(components[a*ARRAY_SIZE+i].Get<ZLayer>()->getZ());
                data.color[i] = *components[a*ARRAY_SIZE+i].Get<Color>();
            }
            shader.writeUniformBinding(0, 0, &data);
            shader.bindSet(0);
            
            vkCmdDrawIndexed(cmdBuffer, components[0].Get<Mesh>()->indices.size(), components.size()-a*ARRAY_SIZE, 0, 0, 0);
        }
    }
}

void engine::renderUVMeshes(Scene& scene) {
    const int ARRAY_SIZE = 10000;

    struct pair_hash {
        std::size_t operator () (const std::pair<int,TextureData*>& pair) const {
            return std::hash<int>()(pair.first) ^ (std::hash<TextureData*>()(pair.second) << 1);
        }
    };

    struct Data {
        glm::mat4 matrix;
    };

    static Shader shader("shaders/UVMesh", ShaderVariables({{VAR_VEC2}, {VAR_VEC2}}), 0, {Binding::Uniform(sizeof(Data)*ARRAY_SIZE), Binding::Sampler()});
    Components validCameras = scene.GetComponents().With<Camera, GlobalTransform>();
    Component<Camera> cameras = validCameras.Get<Camera>();
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
    Components uvMeshes = scene.GetComponents().With<GlobalTransform, ZLayer, Mesh, UVs, Texture>();

    glm::mat4 proj = cameras[main_camera]->getProjectionMatrix(validCameras.Get<GlobalTransform>()[main_camera], Engine::getWindowSize());

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    std::unordered_map<std::pair<int, TextureData*>, std::vector<EntityComponents>,pair_hash> meshes;

    for (unsigned i = 0; i < uvMeshes.size(); i++) {
        meshes[{uvMeshes[i].Get<Mesh>()->mesh_id, &uvMeshes[i].Get<Texture>()->getData()}].push_back(uvMeshes[i]);
    }

    shader.bind();
    for (auto& [key, components] : meshes) {
        int arrays = (components.size()-1)/ARRAY_SIZE+1;
        TextureData& textureData = *key.second;
        std::vector<engine::Buffer *> vertex_buffers = {components[0].Get<Mesh>()->vertexBuffer.get(), components[0].Get<UVs>()->vertexBuffer};

        for (int a = 0; a < arrays; a++) {
            shader.bindVertexBuffers(vertex_buffers);
            components[0].Get<Mesh>()->indexBuffer->bind();

            Data data[ARRAY_SIZE];
            for (int i = 0; i < std::min(static_cast<int>(components.size()-a*ARRAY_SIZE),ARRAY_SIZE); i++) {
                data[i] = Data {
                    proj * components[a*ARRAY_SIZE+i].Get<GlobalTransform>()->getTransformationMatrix(components[a*ARRAY_SIZE+i].Get<ZLayer>()->getZ())
                };
            }
            shader.writeUniformBinding(0, 0, &data);
            shader.writeSamplerBinding(0, 1, textureData);
            shader.bindSet(0);
            
            vkCmdDrawIndexed(cmdBuffer, components[0].Get<Mesh>()->indices.size(), components.size()-a*ARRAY_SIZE, 0, 0, 0);
        }
    }
}

void engine::renderTileMaps(Scene& scene) {
    struct Data {
        glm::mat4 matrix;
        glm::ivec2 tilemap_size;
        glm::ivec2 atlas_size;
        glm::ivec4 tilemap[256*256/4];
    };

    static Shader shader("shaders/TileMap", ShaderVariables({{VAR_VEC2}, {VAR_VEC2}}), 0, {Binding::Uniform(sizeof(Data)), Binding::Sampler()});
    Components validCameras = scene.GetComponents().With<Camera, GlobalTransform>();
    Component<Camera> cameras = validCameras.Get<Camera>();
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

    glm::mat4 proj = cameras[main_camera]->getProjectionMatrix(validCameras.Get<GlobalTransform>()[main_camera], Engine::getWindowSize());

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    Components tilemaps = scene.GetComponents().With<GlobalTransform, ZLayer, Mesh, UVs, TileMap, TextureAtlas>();
    shader.bind();
    for (int i = 0; i < tilemaps.size(); i++) {
        shader.bindVertexBuffers({tilemaps[i].Get<Mesh>()->vertexBuffer.get(), tilemaps[i].Get<UVs>()->vertexBuffer});
        tilemaps[i].Get<Mesh>()->indexBuffer->bind();

        Data data {
            proj * tilemaps[i].Get<GlobalTransform>()->getTransformationMatrix(tilemaps[i].Get<ZLayer>()->getZ()),
            glm::ivec2(tilemaps[i].Get<TileMap>()->size.x, tilemaps[i].Get<TileMap>()->size.y),
            glm::ivec2(tilemaps[i].Get<TextureAtlas>()->size.x, tilemaps[i].Get<TextureAtlas>()->size.y)
        };

        memcpy(&data.tilemap, tilemaps[i].Get<TileMap>()->tiles.data(), tilemaps[i].Get<TileMap>()->tiles.size()*sizeof(int)*4);

        shader.writeUniformBinding(0, 0, &data);
        shader.writeSamplerBinding(0, 1, tilemaps[i].Get<TextureAtlas>()->texture.getData());
        shader.bindSet(0);
        vkCmdDrawIndexed(cmdBuffer, tilemaps[i].Get<Mesh>()->indices.size(), 1, 0, 0, 0);
    }
}

void engine::updateUITransforms(Scene& scene) {
    IVector2 window_size = Engine::getWindowSize();

    std::function<void(UITransform*)> update_recursively;

    update_recursively = [window_size, &update_recursively](UITransform* transform) {
        transform->calculateAbsolute(window_size);
        std::vector<UITransform*> children = transform->getChildren();
        for (UITransform* child : children) {
            update_recursively(child);
        }
    };

    Components transforms = scene.GetComponents().With<UITransform>();
    Component<UITransform> transform = transforms.Get<UITransform>();
    for (int i = 0; i < transform.size(); i++) {
        if (!transform[i]->getParent()) {
            update_recursively(transform[i]);
            
        }
    }
}
void engine::renderColorUI(Scene& scene) {

    const int ARRAY_SIZE = 10000;

    struct Data {
        glm::mat4 matrix[ARRAY_SIZE];
        Color color[ARRAY_SIZE];
    };
    static Shader shader("shaders/ColorMesh", {{{VAR_VEC2}}}, 0, {Binding::Uniform(sizeof(Data))});

    Components colorMeshes = scene.GetComponents().With<UITransform, Mesh, Color>();

    glm::mat4 proj = Camera::getProjectionMatrix(nullptr, Engine::getWindowSize());

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    std::unordered_map<int, std::vector<EntityComponents>> meshes{};

    for (unsigned i = 0; i < colorMeshes.size(); i++) {
        meshes[colorMeshes[i].Get<Mesh>()->mesh_id].push_back(colorMeshes[i]);
    }


    shader.bind();
    for (auto& [id, components] : meshes) {
        int arrays = (components.size()-1)/ARRAY_SIZE+1;

        for (int a = 0; a < arrays; a++) {
            components[0].Get<Mesh>()->vertexBuffer->bind();
            components[0].Get<Mesh>()->indexBuffer->bind();

            Data data;
            for (int i = 0; i < std::min(static_cast<int>(components.size()-a*ARRAY_SIZE),ARRAY_SIZE); i++) {
                data.matrix[i] = proj * components[a*ARRAY_SIZE+i].Get<UITransform>()->getAbsoute().getTransformationMatrix(0.5);
                data.color[i] = *components[a*ARRAY_SIZE+i].Get<Color>();
            }
            shader.writeUniformBinding(0, 0, &data);
            shader.bindSet(0);
            
            vkCmdDrawIndexed(cmdBuffer, components[0].Get<Mesh>()->indices.size(), components.size()-a*ARRAY_SIZE, 0, 0, 0);
        }
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