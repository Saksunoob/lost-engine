#pragma once

#include <vector>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include "utils.hpp"
#include "buffer.hpp"
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

        Entity getEntity(unsigned index) {
            return {filter[index]+1, scene};
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

    template <typename B, typename T>
    struct RenderData {
        const int id;
        const int item_count;

        RenderData(std::vector<T> buffer_data) : id(id_counter++), item_count(buffer_data.size()) {
            data = std::make_shared<std::vector<T>>(buffer_data);
            buffer = std::make_shared<B>(sizeof(T), true);
            buffer->setVector(buffer_data.data(), buffer_data.size());
        }

        B* getBuffer() {return buffer.get();}
        const std::vector<T>& getData() { return *data.get(); }

        private:
            static int id_counter;
            std::shared_ptr<std::vector<T>> data;
            std::shared_ptr<B> buffer = nullptr;
    };

    template<typename B, typename T>
    int RenderData<B, T>::id_counter = 0;

    struct Vertices : public RenderData<VertexBuffer, Vector2> {
        Vertices(std::vector<Vector2> vertices) : RenderData(vertices) {};
    };

    struct Indices : public RenderData<IndexBuffer, unsigned> {
        Indices(std::vector<unsigned> indices) : RenderData(indices) {};
    };

    struct UVs : public RenderData<VertexBuffer, Vector2> {
        UVs(std::vector<Vector2> uvs) : RenderData(uvs) {};
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

        VkImageView getImageView() { return imageView; }
        VkImageLayout getImageLayout() { return imageLayout; }
        int getMipLevels() { return mipLevels; }

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
        VkFormat imageFormat;
        VkImageLayout imageLayout;
    };

    struct Texture {
    public:
        enum Filter {
            LINEAR = VK_FILTER_LINEAR,
            NEAREST = VK_FILTER_NEAREST
        };
        enum AddressMode {
            REPEAT = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            REPEAT_MIRRORED = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
            CLAMP_TO_EDGE = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            CLAMP_TO_BORDER = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
        };

        Texture(const std::string &filepath, Filter filter = Filter::LINEAR, AddressMode address_mode = AddressMode::REPEAT, bool mipmaps = true, Filter mipmap_filter = Filter::LINEAR);
        Texture(const void* data, IVector2 size, TextureFormat format, Filter filter = Filter::LINEAR, AddressMode address_mode = AddressMode::REPEAT, bool mipmaps = true, Filter mipmap_filter = Filter::LINEAR);

        ~Texture();

        TextureData& getData() { return *data.get(); }
        VkSampler getSampler() { return *sampler.get(); }
        IVector2 getSize() { return size; }
    private:
        void createSampler(Filter filter, AddressMode address_mode, bool mipmaps, Filter mimap_filter);

        std::shared_ptr<VkSampler> sampler;
        std::shared_ptr<TextureData> data;
        IVector2 size;
        static std::unordered_map<std::string, std::shared_ptr<TextureData>> texture_files;
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

    struct TextureIndex {
        unsigned index;
    };

    enum UnitType {
        UNIT_PERCENT,
        UNIT_PIXELS,
    };

    enum PointAxis {
        POINT_LEFT,
        POINT_MIDDLE,
        POINT_RIGHT,
        POINT_TOP=POINT_LEFT,
        POINT_BOTTOM=POINT_RIGHT
    };

    struct Point {
        PointAxis horizontal, vertical;

        static inline Point topLeft() {return {POINT_LEFT, POINT_TOP};};
        static inline Point top() {return {POINT_MIDDLE, POINT_TOP};};
        static inline Point topRight() {return {POINT_RIGHT, POINT_TOP};};
        static inline Point left() {return {POINT_LEFT, POINT_MIDDLE};};
        static inline Point center() {return {POINT_MIDDLE, POINT_MIDDLE};};
        static inline Point right() {return {POINT_RIGHT, POINT_MIDDLE};};
        static inline Point bottomLeft() {return {POINT_LEFT, POINT_BOTTOM};};
        static inline Point bottom() {return {POINT_MIDDLE, POINT_BOTTOM};};
        static inline Point bottomRight() {return {POINT_RIGHT, POINT_BOTTOM};};
    };

    struct UITransform {
        Point origin, anchor;
        UnitType position_type, size_type;
        Vector2 position, size;
        float rotation;

        UITransform(Point origin, Point anchor, UnitType position_type, Vector2 position, UnitType size_type, Vector2 size, float rotation) : 
            origin(origin), anchor(anchor),
            position_type(position_type), size_type(size_type),
            position(position), size(size), rotation(rotation), 
            absolute(position, size, rotation) {};

        Transform getAbsoute();
        void calculateAbsolute(IVector2 window_size, Entity entity);

        private:
            Transform absolute;
    };

    struct SlicedTexture {
        std::array<float,4> borders;
        Vector2 pixel_size;
    };

    enum ColliderType {
        SQUARE,
        CIRCLE,
        MESH
    };

    struct UICollider {
        ColliderType type;
        union Data {
            AABB square;
            float radius;
        } data;

        static UICollider square(AABB square) {
            return {SQUARE, Data{square: square}};
        };
        static UICollider circle(float radius) {
            return {CIRCLE, Data{radius: radius}};
        }

        bool collidesWithPoint(Vector2 point, UITransform& transform, Vertices* = nullptr, Indices* = nullptr);
    };
}