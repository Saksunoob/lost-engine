#pragma once

#include <vector>
#include <memory>
#include <tuple>
#include <any>
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include "utils.hpp"

namespace engine {
    class Device;

    template <typename C>
    class Component {
        std::vector<std::unique_ptr<std::any>>& components;
        bool filtered;
        std::vector<unsigned> filter;

        public:

            Component(std::vector<std::unique_ptr<std::any>>& components) : components(components), filtered(false) {};
            Component(Component& component, const std::vector<unsigned>& filter) : components(component.components), filtered(true), filter(filter) {};

            C* operator[](unsigned index) {
                if (filtered) {
                    index = filter[index];
                }
                if (components[index] == nullptr) {
                    return nullptr;
                }
                std::any* component = components[index].get();
                return std::any_cast<C>(component);
            }

            unsigned size() {
                if (filtered) {
                    return filter.size();
                }
                return components.size();
            }
    };

    template<typename... C>
    class EntityComponents {
        std::tuple<C*...> components;

    public:
        EntityComponents(C*... comps) : components(comps...) {}

        template<typename T>
        T* Get() {
            return std::get<T*>(components);
        }
    };

    template <typename... C>
    class Components {
        std::tuple<Component<C>...> components;

        public:

        Components(std::tuple<Component<C>&&...> components) : components(std::move(components)) {};
        Components(Component<C>&&... components) : components(std::make_tuple(std::move(components)...)) {};

        // Indexing operator to return EntityComponents for a given index
        EntityComponents<C...> operator[](unsigned index) {
            return EntityComponents<C...>(std::get<Component<C>>(components)[index]...);
        }

        template<typename T>
        Component<T>& Get() {
            return std::get<Component<T>>(components);
        }

        unsigned size() {
            return std::get<0>(components).size();
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
        std::vector<Vector2> vertices;
        std::vector<unsigned> indices;

        VertexBuffer* vertexBuffer = nullptr;
        IndexBuffer* indexBuffer = nullptr;

        Mesh(std::vector<Vector2> vertices, std::vector<unsigned> indices);

        Mesh(const Mesh&);
        Mesh(Mesh&&);

        ~Mesh();
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

        static glm::mat4 getProjectionMatrix(const Transform& transform, IVector2 window_size);
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
}