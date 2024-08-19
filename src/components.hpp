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
    };

    struct GlobalTransform : Transform {
        GlobalTransform(Vector2 position, Vector2 scale, double rotation): Transform(position, scale, rotation) {};
    };

    struct Mesh {
        std::vector<Vector2> vertices;
        std::vector<unsigned> indices;

        VkBuffer vertexBuffer = nullptr;
        VkBuffer indexBuffer = nullptr;

        Mesh(std::vector<Vector2> vertices, std::vector<unsigned> indices) : vertices(vertices), indices(indices) {};
        ~Mesh();

        void createBuffers(Device& device);

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        private:
            VkDeviceMemory vertexBufferMemory = nullptr;
            VkDeviceMemory indexBufferMemory = nullptr;
            Device* _device = nullptr;
    };

    struct Camera {
        bool main;

        Camera(bool main) : main(main) {};

        static glm::mat4 getProjectionMatrix(const Transform& transform, IVector2 window_size);
    };
}