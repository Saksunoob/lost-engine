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
    Entites validCameras = scene.GetEntities().With<Camera, GlobalTransform>();
    Components<Camera> cameras = validCameras.Get<Camera>();
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
    Entites colorMeshes = scene.GetEntities().With<GlobalTransform, ZLayer, Vertices, Indices, Color>();

    glm::mat4 proj = cameras[main_camera]->getProjectionMatrix(validCameras.Get<GlobalTransform>()[main_camera]);

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    std::unordered_map<long, std::vector<Entity>> meshes{};

    for (unsigned i = 0; i < colorMeshes.size(); i++) {
        long vertex_id = colorMeshes[i].getComponent<Vertices>()->id;
        long index_id = colorMeshes[i].getComponent<Indices>()->id;
        meshes[vertex_id << 32 | index_id].push_back(colorMeshes[i]);
    }

    shader.bind();
    for (auto& [id, components] : meshes) {
        int arrays = (components.size()-1)/ARRAY_SIZE+1;

        for (int a = 0; a < arrays; a++) {
            components[0].getComponent<Vertices>()->getBuffer()->bind();
            components[0].getComponent<Indices>()->getBuffer()->bind();

            Data data;
            for (int i = 0; i < std::min(static_cast<int>(components.size()-a*ARRAY_SIZE),ARRAY_SIZE); i++) {
                data.matrix[i] = proj * components[a*ARRAY_SIZE+i].getComponent<GlobalTransform>()->getTransformationMatrix(components[a*ARRAY_SIZE+i].getComponent<ZLayer>()->getZ());
                data.color[i] = *components[a*ARRAY_SIZE+i].getComponent<Color>();
            }
            shader.writeUniformBinding(0, 0, &data);
            shader.bindSet(0);
            
            vkCmdDrawIndexed(cmdBuffer, components[0].getComponent<Indices>()->item_count, components.size()-a*ARRAY_SIZE, 0, 0, 0);
        }
    }
}

void engine::renderUVMeshes(Scene& scene) {
    const int ARRAY_SIZE = 10000;

    struct Data {
        glm::mat4 matrix;
    };

    static Shader shader("shaders/UVMesh", ShaderVariables({{VAR_VEC2}, {VAR_VEC2}}), 0, {Binding::Uniform(sizeof(Data)*ARRAY_SIZE), Binding::Sampler()});
    Entites validCameras = scene.GetEntities().With<Camera, GlobalTransform>();
    Components<Camera> cameras = validCameras.Get<Camera>();
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
    Entites uvMeshes = scene.GetEntities().With<GlobalTransform, ZLayer, Vertices, Indices, UVs, Texture>().Without<SlicedTexture>();

    glm::mat4 proj = cameras[main_camera]->getProjectionMatrix(validCameras.Get<GlobalTransform>()[main_camera]);

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    std::unordered_map<std::tuple<int, int, int, TextureData*, VkSampler>, std::vector<Entity>, tuple_hash<int, int, int, TextureData*, VkSampler>> meshes;

    for (unsigned i = 0; i < uvMeshes.size(); i++) {
        int vertex_id = uvMeshes[i].getComponent<Vertices>()->id;
        int index_id = uvMeshes[i].getComponent<Indices>()->id;
        int uv_id = uvMeshes[i].getComponent<UVs>()->id;
        meshes[{vertex_id, index_id, uv_id, &uvMeshes[i].getComponent<Texture>()->getData(), uvMeshes[i].getComponent<Texture>()->getSampler()}].push_back(uvMeshes[i]);
    }

    shader.bind();
    for (auto& [key, components] : meshes) {
        int arrays = (components.size()-1)/ARRAY_SIZE+1;
        Texture& texture = *components[0].getComponent<Texture>();
        std::vector<engine::Buffer *> vertex_buffers = {components[0].getComponent<Vertices>()->getBuffer(), components[0].getComponent<UVs>()->getBuffer()};

        for (int a = 0; a < arrays; a++) {
            shader.bindVertexBuffers(vertex_buffers);
            components[0].getComponent<Indices>()->getBuffer()->bind();

            Data data[ARRAY_SIZE];
            for (int i = 0; i < std::min(static_cast<int>(components.size()-a*ARRAY_SIZE),ARRAY_SIZE); i++) {
                data[i] = Data {
                    proj * components[a*ARRAY_SIZE+i].getComponent<GlobalTransform>()->getTransformationMatrix(components[a*ARRAY_SIZE+i].getComponent<ZLayer>()->getZ())
                };
            }
            shader.writeUniformBinding(0, 0, &data);
            shader.writeSamplerBinding(0, 1, texture);
            shader.bindSet(0);
            
            vkCmdDrawIndexed(cmdBuffer, components[0].getComponent<Indices>()->item_count, components.size()-a*ARRAY_SIZE, 0, 0, 0);
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
    Entites validCameras = scene.GetEntities().With<Camera, GlobalTransform>();
    Components<Camera> cameras = validCameras.Get<Camera>();
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

    glm::mat4 proj = cameras[main_camera]->getProjectionMatrix(validCameras.Get<GlobalTransform>()[main_camera]);

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    Entites tilemaps = scene.GetEntities().With<GlobalTransform, ZLayer, Vertices, Indices, UVs, TileMap, TextureAtlas>();
    shader.bind();
    for (int i = 0; i < tilemaps.size(); i++) {
        shader.bindVertexBuffers({tilemaps[i].getComponent<Vertices>()->getBuffer(), tilemaps[i].getComponent<UVs>()->getBuffer()});
        tilemaps[i].getComponent<Indices>()->getBuffer()->bind();

        Data data {
            proj * tilemaps[i].getComponent<GlobalTransform>()->getTransformationMatrix(tilemaps[i].getComponent<ZLayer>()->getZ()),
            glm::ivec2(tilemaps[i].getComponent<TileMap>()->size.x, tilemaps[i].getComponent<TileMap>()->size.y),
            glm::ivec2(tilemaps[i].getComponent<TextureAtlas>()->size.x, tilemaps[i].getComponent<TextureAtlas>()->size.y)
        };

        memcpy(&data.tilemap, tilemaps[i].getComponent<TileMap>()->tiles.data(), tilemaps[i].getComponent<TileMap>()->tiles.size()*sizeof(int)*4);

        shader.writeUniformBinding(0, 0, &data);
        shader.writeSamplerBinding(0, 1, tilemaps[i].getComponent<TextureAtlas>()->texture);
        shader.bindSet(0);
        vkCmdDrawIndexed(cmdBuffer, tilemaps[i].getComponent<Indices>()->item_count, 1, 0, 0, 0);
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
    Entites validCameras = scene.GetEntities().With<Camera, GlobalTransform>();
    Components<Camera> cameras = validCameras.Get<Camera>();
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
    Entites uvMeshes = scene.GetEntities().With<GlobalTransform, ZLayer, Vertices, Indices, UVs, TextureAtlas, TextureIndex>();

    glm::mat4 proj = cameras[main_camera]->getProjectionMatrix(validCameras.Get<GlobalTransform>()[main_camera]);

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();

    std::unordered_map<std::tuple<int, int, int, TextureData*, VkSampler>, std::vector<Entity>, tuple_hash<int, int, int, TextureData*, VkSampler>> meshes;

    for (unsigned i = 0; i < uvMeshes.size(); i++) {
        int vertex_id = uvMeshes[i].getComponent<Vertices>()->id;
        int index_id = uvMeshes[i].getComponent<Indices>()->id;
        int uv_id = uvMeshes[i].getComponent<UVs>()->id;
        Texture& texture = uvMeshes[i].getComponent<TextureAtlas>()->texture;
        meshes[{vertex_id, index_id, uv_id, &texture.getData(), texture.getSampler()}].push_back(uvMeshes[i]);
    }

    shader.bind();
    for (auto& [key, components] : meshes) {
        int arrays = (components.size()-1)/ARRAY_SIZE+1;
        Texture& texture = components[0].getComponent<TextureAtlas>()->texture;
        std::vector<engine::Buffer *> vertex_buffers = {components[0].getComponent<Vertices>()->getBuffer(), components[0].getComponent<UVs>()->getBuffer()};
        IVector2 atlas_size = components[0].getComponent<TextureAtlas>()->size;

        for (int a = 0; a < arrays; a++) {
            shader.bindVertexBuffers(vertex_buffers);
            components[0].getComponent<Indices>()->getBuffer()->bind();

            Data data;
            data.atlas_size = glm::ivec2(atlas_size.x, atlas_size.y);
            for (int i = 0; i < std::min(static_cast<int>(components.size()-a*ARRAY_SIZE),ARRAY_SIZE); i++) {
                data.matrix[i] = proj * components[a*ARRAY_SIZE+i].getComponent<GlobalTransform>()->getTransformationMatrix(components[a*ARRAY_SIZE+i].getComponent<ZLayer>()->getZ());
                data.indices[i] = components[a*ARRAY_SIZE+i].getComponent<TextureIndex>()->index;
            }
            shader.writeUniformBinding(0, 0, &data);
            shader.writeSamplerBinding(0, 1, texture);
            shader.bindSet(0);
            
            vkCmdDrawIndexed(cmdBuffer, components[0].getComponent<Indices>()->item_count, components.size()-a*ARRAY_SIZE, 0, 0, 0);
        }
    }
}

void engine::updateTransforms(Scene& scene) {
    std::function<void(Entity)> update_recursively;

    update_recursively = [&update_recursively](Entity entity) {
        Transform* transform = entity.getComponent<Transform>();
        GlobalTransform* global = entity.getComponent<GlobalTransform>();
        GlobalTransform* parent = entity.getParent().getComponent<GlobalTransform>();

        if (!transform || !global) {
            return;
        }

        if (parent) {
            *global = *transform * *parent;
        } else {
            *global = *transform;
        }

        std::vector<Entity> children = entity.getChildren();
        for (Entity& child : children) {
            update_recursively(child);
        }
    };

    Entites transforms = scene.GetEntities().With<Transform, GlobalTransform>();
    for (int i = 0; i < transforms.size(); i++) {
        if (transforms.getEntity(i).getParent().isNull()) {
            update_recursively(transforms[i]);
        }
    }
}

void engine::updateUITransforms(Scene& scene) {
    IVector2 window_size = Engine::getWindowSize();

    std::function<void(UITransform*, Entity)> update_recursively;

    update_recursively = [window_size, &update_recursively](UITransform* transform, Entity entity) {
        transform->calculateGlobal(window_size, entity);
        std::vector<Entity> children = entity.getChildren();
        for (Entity& child : children) {
            UITransform* child_transform = child.getComponent<UITransform>();
            if (child_transform != nullptr) {
                update_recursively(child_transform, child);
            }
        }
    };

    Entites transforms = scene.GetEntities().With<UITransform>();
    Components<UITransform> transform = transforms.Get<UITransform>();
    for (int i = 0; i < transform.size(); i++) {
        if (transforms.getEntity(i).getParent().isNull()) {
            update_recursively(transform[i], transforms.getEntity(i));
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

    glm::mat4 proj = Camera::getProjectionMatrix(nullptr);

    VkCommandBuffer cmdBuffer = Engine::getCurrentCommandBuffer();
    Entites textures = scene.GetEntities().With<GlobalTransform, ZLayer, Vertices, Indices, UVs, Texture, SlicedTexture>();
    shader.bind();
    for (int i = 0; i < textures.size(); i++) {
        shader.bindVertexBuffers({textures[i].getComponent<Vertices>()->getBuffer(), textures[i].getComponent<UVs>()->getBuffer()});
        textures[i].getComponent<Indices>()->getBuffer()->bind();
        Data data {
            textures[i].getComponent<GlobalTransform>()->getTransformationMatrix(textures[i].getComponent<ZLayer>()->getZ()),
            proj,
            textures[i].getComponent<SlicedTexture>()->pixel_size,
            textures[i].getComponent<Texture>()->getSize(),
            textures[i].getComponent<SlicedTexture>()->borders
        };
        shader.writeUniformBinding(0, 0, &data);
        shader.writeSamplerBinding(0, 1, *textures[i].getComponent<Texture>());
        shader.bindSet(0);
        vkCmdDrawIndexed(cmdBuffer, textures[i].getComponent<Indices>()->item_count, 1, 0, 0, 0);
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