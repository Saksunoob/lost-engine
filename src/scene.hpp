#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>

#include <SDL2/SDL.h>

#include "stage.hpp"
#include "component.hpp"
#include "bundle.hpp"

namespace engine {

    class Scene;

    struct Entity {
        unsigned id;
        Scene& scene;

        template <typename C>
        C& addComponent(C component);
        void addBundle(Bundle& bundle);

        operator unsigned() const{
            return id;
        }
    };

    class Components;

    class Scene {
        friend Components;

        std::string name;
        std::vector<Stage> stages = std::vector<Stage>();
        std::vector<unsigned> empty_entity_ids = std::vector<unsigned>();
        std::unordered_map<std::type_index, unsigned> component_mapping = std::unordered_map<std::type_index, unsigned>();
        std::unordered_map<std::type_index, std::any> resources{};

        protected:
        unsigned entity_vector_length = 0;
        std::vector<std::vector<std::unique_ptr<std::any>>> components = std::vector<std::vector<std::unique_ptr<std::any>>>();

            template<typename C>
            Component<C> GetComponent() {
                auto it = component_mapping.find(std::type_index(typeid(C)));
                if (it == component_mapping.end()) {
                    return Component<C>();
                }
                unsigned index = it->second;
                return Component<C>(components[index]);
            }

        public:
            Scene(std::string name) : name(name) {}
            Scene() : name("empty_scene") {}

            void executeStages();

            void addStageAfter(std::string stage, std::string after);
            void addStageBefore(std::string stage, std::string before);
            void addStageAt(std::string stage, unsigned index);
            void removeStage(std::string stage);
            Stage* getStage(std::string stage);

            Entity createEntity();
            void destroyEntity(Entity entity);

            template <typename C>
            C& addComponent(Entity entity, C component) {
                std::type_index type = std::type_index(typeid(C));
                if (component_mapping.find(type) == component_mapping.end()) {
                    components.push_back(std::vector<std::unique_ptr<std::any>>(entity_vector_length));
                    component_mapping[type] = components.size() - 1;
                }
                components[component_mapping[type]][entity] = std::make_unique<std::any>(std::move(component));
                return *std::any_cast<C>(components[component_mapping[type]][entity].get());
            }

            void addBundle(Entity entity, Bundle& bundle);

            Components GetComponents();

            template<typename R>
            void addResource(R resource) {
                std::type_index id = typeid(R);
                resources[id] = resource;
            }

            template<typename R>
            R& getResource() {
                std::type_index id = typeid(R);
                return std::any_cast<R&>(resources.at(id));
            }
    };

    template <typename C>
    C& Entity::addComponent(C component) {
        return scene.addComponent(*this, component);
    }
}