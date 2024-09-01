#pragma once

#include <vector>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include "utils.hpp"
#include "scene.hpp"
#include "component.hpp"

namespace engine {
    class Device;

    class EntityComponents;

    class Components {
        Scene &scene;
        std::vector<unsigned> filter;

        unsigned index_to_filtered(unsigned index) {
            if (index >= filter.size()) {
                Logger::logError("Invalid index for components");
                return 0;
            }
            return filter[index];
        }

        public:

        Components(Scene& scene) : scene(scene), filter(scene.entity_vector_length) {
            for (unsigned i = 0; i < filter.size(); i++) {
                filter[i]=i;
            }
        };

        EntityComponents operator[](unsigned index);

        template<typename... W>
        Components With() {
            for (auto it = filter.begin(); it != filter.end();) {
                bool has = (scene.GetComponent<W>().hasEntity(*it) && ...);
                if (!has) {
                    filter.erase(it);
                    continue;
                }
                it++;
            }
            
            return *this;
        }

        template<typename... W>
        Components& Without() {
            for (auto it = filter.begin(); it != filter.end();) {
                bool has = (scene.GetComponent<W>().hasEntity(*it) && ...);
                if (has) {
                    filter.erase(it);
                    continue;
                }
                it++;
            }
            
            return *this;
        }

        template<typename T>
        Component<T> Get() {
            return scene.GetComponent<T>().withFilter(filter);
        }

        unsigned size() {
            return filter.size();
        }
    };

    class EntityComponents {
        Components& components;
        unsigned entity;

    public:
        EntityComponents(Components& comps, unsigned entity) : components(comps), entity(entity) {}

        template<typename T>
        T* Get() {
            return components.Get<T>()[entity];
        }
    };

    struct Transform {
        Vector2 position, scale;
        double rotation;

        Transform(Vector2 position, Vector2 scale, double rotation) : position(position), scale(scale), rotation(rotation) {};

        glm::mat4 getTransformationMatrix() const;
        glm::mat4 getTransformationMatrix(float z) const;
    };

    struct GlobalTransform : Transform {
        GlobalTransform(Vector2 position, Vector2 scale, double rotation): Transform(position, scale, rotation) {};
    };

    /// Determines entity's depth value. Lower is rendered on top.
    struct ZLayer {
        float order;

        ZLayer(int layer, float order);
        void setLayer(int new_layer);

        float getZ();

        private:
        int layer;
        static int min_layer;
        static int max_layer;
    };

    class VertexBuffer;
    class IndexBuffer;

    struct Mesh {
        const std::vector<Vector2> vertices;
        const std::vector<unsigned> indices;
        const int mesh_id;

        std::shared_ptr<VertexBuffer> vertexBuffer = nullptr;
        std::shared_ptr<IndexBuffer> indexBuffer = nullptr;

        Mesh(std::vector<Vector2> vertices, std::vector<unsigned> indices);

        private:
            static int mesh_id_counter;
    };

    struct UVs {
        std::vector<Vector2> uvs;

        VertexBuffer* vertexBuffer = nullptr;

        UVs(std::vector<Vector2> uvs);

        UVs(const UVs&);
        UVs(UVs&&);

        ~UVs();
    };

    struct Camera {
        bool main;

        Camera(bool main) : main(main) {};

        static glm::mat4 getProjectionMatrix(const Transform* transform, IVector2 window_size);
    };

    struct TextureFormat {
        uint channels;
        uint type;
        static TextureFormat Unorm(uint channels) { return {channels, 9}; }
        static TextureFormat Snorm(uint channels) { return {channels, 10}; }
        static TextureFormat Uscaled(uint channels) { return {channels, 11}; }
        static TextureFormat Sscaled(uint channels) { return {channels, 12}; }
        static TextureFormat Uint(uint channels) { return {channels, 13}; }
        static TextureFormat Sint(uint channels) { return {channels, 14}; }
        static TextureFormat Srgb(uint channels) { return {channels, 15}; }
        VkFormat getFormat() {
            uint mul = channels;
            if (mul == 4) {
                mul++;
            }
            return static_cast<VkFormat>(type+7*(mul-1));
        }
    };    

    struct TextureData {
    public:
        TextureData(const void* data, IVector2 size, TextureFormat format);
        ~TextureData();

        TextureData(const TextureData &) = delete;
        TextureData(TextureData &&);

        VkSampler getSampler() { return sampler; }
        VkImageView getImageView() { return imageView; }
        VkImageLayout getImageLayout() { return imageLayout; }

        void update(const void* data);
    private:
        void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void generateMipmaps();

        IVector2 size;
        TextureFormat format;
        int mipLevels;

        VkImage image;
        VkDeviceMemory imageMemory;
        VkImageView imageView;
        VkSampler sampler;
        VkFormat imageFormat;
        VkImageLayout imageLayout;
    };

    struct Texture {
    public:
        Texture(const std::string &filepath);
        Texture(const void* data, IVector2 size, TextureFormat format);

        TextureData& getData() { return *data.get(); }
    private:
        void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void generateMipmaps();

        std::shared_ptr<TextureData> data;
    };

    struct TileMap {
        IVector2 size;
        std::vector<int> tiles;

        TileMap(IVector2 size);

        void setTile(IVector2 pos, int value);
    };

    struct TextureAtlas {
        IVector2 size;
        Texture texture;
    };

    enum UnitType {
        UNIT_PERCENT,
        UNIT_PIXELS,
    };

    struct UITransform {
        UnitType position_type, size_type;
        Vector2 position, size;
        float rotation;

        UITransform(UnitType position_type, Vector2 position, UnitType size_type, Vector2 size, float rotation) : 
            position_type(position_type), size_type(size_type), position(position), size(size), rotation(rotation), absolute(position, size, rotation), parent(nullptr) {};

        void addChild(UITransform&);
        void removeChild(UITransform&);
        std::vector<UITransform*> getChildren();
        UITransform* getParent();
        Transform getAbsoute();
        void calculateAbsolute(IVector2 window_size);

        private:
            Transform absolute;
            UITransform* parent;
            std::vector<UITransform*> children;
    };
}