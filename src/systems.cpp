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
    Components colorMeshes = scene.GetComponents().With<GlobalTransform, ZLayer, Vertices, Indices, Color>();

    glm::mat4 proj = cameras[main_camera]->getProjectionMatrix(validCameras.Get<GlobalTransform>()[main_camera], Engine::getWindowSize());

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    std::unordered_map<long, std::vector<EntityComponents>> meshes{};

    for (unsigned i = 0; i < colorMeshes.size(); i++) {
        long vertex_id = colorMeshes[i].Get<Vertices>()->id;
        long index_id = colorMeshes[i].Get<Indices>()->id;
        meshes[vertex_id << 32 | index_id].push_back(colorMeshes[i]);
    }

    shader.bind();
    for (auto& [id, components] : meshes) {
        int arrays = (components.size()-1)/ARRAY_SIZE+1;

        for (int a = 0; a < arrays; a++) {
            components[0].Get<Vertices>()->getBuffer()->bind();
            components[0].Get<Indices>()->getBuffer()->bind();

            Data data;
            for (int i = 0; i < std::min(static_cast<int>(components.size()-a*ARRAY_SIZE),ARRAY_SIZE); i++) {
                data.matrix[i] = proj * components[a*ARRAY_SIZE+i].Get<GlobalTransform>()->getTransformationMatrix(components[a*ARRAY_SIZE+i].Get<ZLayer>()->getZ());
                data.color[i] = *components[a*ARRAY_SIZE+i].Get<Color>();
            }
            shader.writeUniformBinding(0, 0, &data);
            shader.bindSet(0);
            
            vkCmdDrawIndexed(cmdBuffer, components[0].Get<Indices>()->item_count, components.size()-a*ARRAY_SIZE, 0, 0, 0);
        }
    }
}

void engine::renderUVMeshes(Scene& scene) {
    const int ARRAY_SIZE = 10000;

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
    Components uvMeshes = scene.GetComponents().With<GlobalTransform, ZLayer, Vertices, Indices, UVs, Texture>();

    glm::mat4 proj = cameras[main_camera]->getProjectionMatrix(validCameras.Get<GlobalTransform>()[main_camera], Engine::getWindowSize());

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    std::unordered_map<std::tuple<int, int, int, TextureData*, VkSampler>, std::vector<EntityComponents>, tuple_hash<int, int, int, TextureData*, VkSampler>> meshes;

    for (unsigned i = 0; i < uvMeshes.size(); i++) {
        int vertex_id = uvMeshes[i].Get<Vertices>()->id;
        int index_id = uvMeshes[i].Get<Indices>()->id;
        int uv_id = uvMeshes[i].Get<UVs>()->id;
        meshes[{vertex_id, index_id, uv_id, &uvMeshes[i].Get<Texture>()->getData(), uvMeshes[i].Get<Texture>()->getSampler()}].push_back(uvMeshes[i]);
    }

    shader.bind();
    for (auto& [key, components] : meshes) {
        int arrays = (components.size()-1)/ARRAY_SIZE+1;
        Texture& texture = *components[0].Get<Texture>();
        std::vector<engine::Buffer *> vertex_buffers = {components[0].Get<Vertices>()->getBuffer(), components[0].Get<UVs>()->getBuffer()};

        for (int a = 0; a < arrays; a++) {
            shader.bindVertexBuffers(vertex_buffers);
            components[0].Get<Indices>()->getBuffer()->bind();

            Data data[ARRAY_SIZE];
            for (int i = 0; i < std::min(static_cast<int>(components.size()-a*ARRAY_SIZE),ARRAY_SIZE); i++) {
                data[i] = Data {
                    proj * components[a*ARRAY_SIZE+i].Get<GlobalTransform>()->getTransformationMatrix(components[a*ARRAY_SIZE+i].Get<ZLayer>()->getZ())
                };
            }
            shader.writeUniformBinding(0, 0, &data);
            shader.writeSamplerBinding(0, 1, texture);
            shader.bindSet(0);
            
            vkCmdDrawIndexed(cmdBuffer, components[0].Get<Indices>()->item_count, components.size()-a*ARRAY_SIZE, 0, 0, 0);
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

    Components tilemaps = scene.GetComponents().With<GlobalTransform, ZLayer, Vertices, Indices, UVs, TileMap, TextureAtlas>();
    shader.bind();
    for (int i = 0; i < tilemaps.size(); i++) {
        shader.bindVertexBuffers({tilemaps[i].Get<Vertices>()->getBuffer(), tilemaps[i].Get<UVs>()->getBuffer()});
        tilemaps[i].Get<Indices>()->getBuffer()->bind();

        Data data {
            proj * tilemaps[i].Get<GlobalTransform>()->getTransformationMatrix(tilemaps[i].Get<ZLayer>()->getZ()),
            glm::ivec2(tilemaps[i].Get<TileMap>()->size.x, tilemaps[i].Get<TileMap>()->size.y),
            glm::ivec2(tilemaps[i].Get<TextureAtlas>()->size.x, tilemaps[i].Get<TextureAtlas>()->size.y)
        };

        memcpy(&data.tilemap, tilemaps[i].Get<TileMap>()->tiles.data(), tilemaps[i].Get<TileMap>()->tiles.size()*sizeof(int)*4);

        shader.writeUniformBinding(0, 0, &data);
        shader.writeSamplerBinding(0, 1, tilemaps[i].Get<TextureAtlas>()->texture);
        shader.bindSet(0);
        vkCmdDrawIndexed(cmdBuffer, tilemaps[i].Get<Indices>()->item_count, 1, 0, 0, 0);
    }
}

void engine::renderIndexedTextures(Scene& scene) {
    const int ARRAY_SIZE = 10000;

    struct Data {
        glm::mat4 matrix[10000];
        unsigned indices[10000];
        glm::ivec2 atlas_size;
    };

    static Shader shader("shaders/IndexedTexture", ShaderVariables({{VAR_VEC2}, {VAR_VEC2}}), 0, {Binding::Uniform(sizeof(Data)), Binding::Sampler()});
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
    Components uvMeshes = scene.GetComponents().With<GlobalTransform, ZLayer, Vertices, Indices, UVs, TextureAtlas, TextureIndex>();

    glm::mat4 proj = cameras[main_camera]->getProjectionMatrix(validCameras.Get<GlobalTransform>()[main_camera], Engine::getWindowSize());

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    std::unordered_map<std::tuple<int, int, int, TextureData*, VkSampler>, std::vector<EntityComponents>, tuple_hash<int, int, int, TextureData*, VkSampler>> meshes;

    for (unsigned i = 0; i < uvMeshes.size(); i++) {
        int vertex_id = uvMeshes[i].Get<Vertices>()->id;
        int index_id = uvMeshes[i].Get<Indices>()->id;
        int uv_id = uvMeshes[i].Get<UVs>()->id;
        Texture& texture = uvMeshes[i].Get<TextureAtlas>()->texture;
        meshes[{vertex_id, index_id, uv_id, &texture.getData(), texture.getSampler()}].push_back(uvMeshes[i]);
    }

    shader.bind();
    for (auto& [key, components] : meshes) {
        int arrays = (components.size()-1)/ARRAY_SIZE+1;
        Texture& texture = components[0].Get<TextureAtlas>()->texture;
        std::vector<engine::Buffer *> vertex_buffers = {components[0].Get<Vertices>()->getBuffer(), components[0].Get<UVs>()->getBuffer()};
        IVector2 atlas_size = components[0].Get<TextureAtlas>()->size;

        for (int a = 0; a < arrays; a++) {
            shader.bindVertexBuffers(vertex_buffers);
            components[0].Get<Indices>()->getBuffer()->bind();

            Data data;
            data.atlas_size = glm::ivec2(atlas_size.x, atlas_size.y);
            for (int i = 0; i < std::min(static_cast<int>(components.size()-a*ARRAY_SIZE),ARRAY_SIZE); i++) {
                data.matrix[i] = proj * components[a*ARRAY_SIZE+i].Get<GlobalTransform>()->getTransformationMatrix(components[a*ARRAY_SIZE+i].Get<ZLayer>()->getZ());
                data.indices[i] = components[a*ARRAY_SIZE+i].Get<TextureIndex>()->index;
            }
            shader.writeUniformBinding(0, 0, &data);
            shader.writeSamplerBinding(0, 1, texture);
            shader.bindSet(0);
            
            vkCmdDrawIndexed(cmdBuffer, components[0].Get<Indices>()->item_count, components.size()-a*ARRAY_SIZE, 0, 0, 0);
        }
    }
}

void engine::updateUITransforms(Scene& scene) {
    IVector2 window_size = Engine::getWindowSize();

    std::function<void(UITransform*, Entity entity)> update_recursively;

    update_recursively = [window_size, &update_recursively](UITransform* transform, Entity entity) {
        transform->calculateAbsolute(window_size, entity);
        std::vector<Entity> children = entity.getChildren();
        for (Entity& child : children) {
            UITransform* child_transform = child.getComponent<UITransform>();
            if (child_transform != nullptr) {
                update_recursively(child_transform, child);
            }
        }
    };

    Components transforms = scene.GetComponents().With<UITransform>();
    Component<UITransform> transform = transforms.Get<UITransform>();
    for (int i = 0; i < transform.size(); i++) {
        if (transforms.getEntity(i).getParent().isNull()) {
            update_recursively(transform[i], transforms.getEntity(i));
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

    Components colorMeshes = scene.GetComponents().With<UITransform, Vertices, Indices, Color>();

    glm::mat4 proj = Camera::getProjectionMatrix(nullptr, Engine::getWindowSize());

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    std::unordered_map<long, std::vector<EntityComponents>> meshes{};

    for (unsigned i = 0; i < colorMeshes.size(); i++) {
        long vertex_id = colorMeshes[i].Get<Vertices>()->id;
        long index_id = colorMeshes[i].Get<Indices>()->id;
        meshes[vertex_id << 32 | index_id].push_back(colorMeshes[i]);
    }


    shader.bind();
    for (auto& [id, components] : meshes) {
        int arrays = (components.size()-1)/ARRAY_SIZE+1;

        for (int a = 0; a < arrays; a++) {
            components[0].Get<Vertices>()->getBuffer()->bind();
            components[0].Get<Indices>()->getBuffer()->bind();

            Data data;
            for (int i = 0; i < std::min(static_cast<int>(components.size()-a*ARRAY_SIZE),ARRAY_SIZE); i++) {
                float z = 0;
                if (components[a*ARRAY_SIZE+i].Get<ZLayer>()) {
                    z = components[a*ARRAY_SIZE+i].Get<ZLayer>()->getZ();
                }
                data.matrix[i] = proj * components[a*ARRAY_SIZE+i].Get<UITransform>()->getAbsoute().getTransformationMatrix(z);
                data.color[i] = *components[a*ARRAY_SIZE+i].Get<Color>();
            }
            shader.writeUniformBinding(0, 0, &data);
            shader.bindSet(0);
            
            vkCmdDrawIndexed(cmdBuffer, components[0].Get<Indices>()->item_count, components.size()-a*ARRAY_SIZE, 0, 0, 0);
        }
    }
}

void engine::renderTextureUI(Scene& scene) {

    const int ARRAY_SIZE = 10000;

    struct Data {
        glm::mat4 matrix;
    };

    static Shader shader("shaders/UVMesh", ShaderVariables({{VAR_VEC2}, {VAR_VEC2}}), 0, {Binding::Uniform(sizeof(Data)*ARRAY_SIZE), Binding::Sampler()});

    Components uvMeshes = scene.GetComponents().With<UITransform, Vertices, Indices, UVs, Texture>().Without<SlicedTexture>();

    glm::mat4 proj = Camera::getProjectionMatrix(nullptr, Engine::getWindowSize());

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    std::unordered_map<std::tuple<int, int, int, TextureData*, VkSampler>, std::vector<EntityComponents>, tuple_hash<int, int, int, TextureData*, VkSampler>> meshes;

    for (unsigned i = 0; i < uvMeshes.size(); i++) {
        int vertex_id = uvMeshes[i].Get<Vertices>()->id;
        int index_id = uvMeshes[i].Get<Indices>()->id;
        int uv_id = uvMeshes[i].Get<UVs>()->id;
        meshes[{vertex_id, index_id, uv_id, &uvMeshes[i].Get<Texture>()->getData(), uvMeshes[i].Get<Texture>()->getSampler()}].push_back(uvMeshes[i]);
    }

    shader.bind();
    for (auto& [key, components] : meshes) {
        int arrays = (components.size()-1)/ARRAY_SIZE+1;
        Texture& texture = *components[0].Get<Texture>();
        std::vector<engine::Buffer *> vertex_buffers = {components[0].Get<Vertices>()->getBuffer(), components[0].Get<UVs>()->getBuffer()};

        for (int a = 0; a < arrays; a++) {
            shader.bindVertexBuffers(vertex_buffers);
            components[0].Get<Indices>()->getBuffer()->bind();

            Data data[ARRAY_SIZE];
            for (int i = 0; i < std::min(static_cast<int>(components.size()-a*ARRAY_SIZE),ARRAY_SIZE); i++) {
                float z = 0;
                if (components[a*ARRAY_SIZE+i].Get<ZLayer>()) {
                    z = components[a*ARRAY_SIZE+i].Get<ZLayer>()->getZ();
                }
                data[i] = Data {
                    proj * components[a*ARRAY_SIZE+i].Get<UITransform>()->getAbsoute().getTransformationMatrix(z)
                };
            }
            shader.writeUniformBinding(0, 0, &data);
            shader.writeSamplerBinding(0, 1, texture);
            shader.bindSet(0);
            
            vkCmdDrawIndexed(cmdBuffer, components[0].Get<Indices>()->item_count, components.size()-a*ARRAY_SIZE, 0, 0, 0);
        }
    }
}

void engine::renderSlicedTextures(Scene& scene) {
    struct Data {
        glm::mat4 matrix;
        glm::mat4 proj;
        Vector2 slice_scale;
        Vector2 teture_size;
        std::array<float, 4> borders;
    };

    static Shader shader("shaders/9SliceTexture", ShaderVariables({{VAR_VEC2}, {VAR_VEC2}}), 0, {Binding::Uniform(sizeof(Data)), Binding::Sampler()});

    glm::mat4 proj = Camera::getProjectionMatrix(nullptr, Engine::getWindowSize());

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    Components textures = scene.GetComponents().With<UITransform, ZLayer, Vertices, Indices, UVs, Texture, SlicedTexture>();
    shader.bind();
    for (int i = 0; i < textures.size(); i++) {
        shader.bindVertexBuffers({textures[i].Get<Vertices>()->getBuffer(), textures[i].Get<UVs>()->getBuffer()});
        textures[i].Get<Indices>()->getBuffer()->bind();

        Data data {
            textures[i].Get<UITransform>()->getAbsoute().getTransformationMatrix(textures[i].Get<ZLayer>()->getZ()),
            proj,
            textures[i].Get<SlicedTexture>()->pixel_size,
            textures[i].Get<Texture>()->getSize(),
            textures[i].Get<SlicedTexture>()->borders
        };

        shader.writeUniformBinding(0, 0, &data);
        shader.writeSamplerBinding(0, 1, *textures[i].Get<Texture>());
        shader.bindSet(0);
        vkCmdDrawIndexed(cmdBuffer, textures[i].Get<Indices>()->item_count, 1, 0, 0, 0);
    }
}

void engine::timeSystem(Scene& scene) {
    Time& time = scene.getResource<Time>();
    time.newFrame();
    //Logger::log("FPS: " + std::to_string(1/time.deltaTime()));
}

void engine::pollSDLEvents(Scene& scene) {
    Input& input = scene.getResource<Input>();
    input.newFrame();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        input.handleKeyEvent(event);
        switch (event.type) {
            case (SDL_QUIT):
                Engine::quit();
                break;
        }
    }
}