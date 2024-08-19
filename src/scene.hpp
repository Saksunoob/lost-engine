#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>

#include <SDL2/SDL.h>

#include "stage.hpp"
#include "components.hpp"

namespace engine {

    class Scene;

    struct Entity {
        unsigned id;
        Scene& scene;

        template <typename C>
        void addComponent(C component);

        operator unsigned() const{
            return id;
        }
    };

    class Scene {
        std::string name;
        std::vector<Stage> stages = std::vector<Stage>();
        unsigned entity_vector_length = 0;
        std::vector<unsigned> empty_entity_ids = std::vector<unsigned>();
        std::vector<std::vector<std::unique_ptr<std::any>>> components = std::vector<std::vector<std::unique_ptr<std::any>>>();
        std::unordered_map<std::type_index, unsigned> component_mapping = std::unordered_map<std::type_index, unsigned>();
        std::unordered_map<std::type_index, std::any> resources{};

        template<typename... C>
        std::vector<unsigned> FilterValidEntities(std::tuple<Component<C>...>& componentTuples) {
            std::vector<unsigned> validIndices;

            for (unsigned i = 0; i < entity_vector_length; ++i) {
                bool isValid = (std::get<Component<C>>(componentTuples)[i] && ...);
                if (isValid) {
                    validIndices.push_back(i);
                }
            }
            return validIndices;
        }

        template<typename... C>
        Components<C...> CreateFilteredComponents(std::tuple<Component<C>...>& componentTuples, const std::vector<unsigned>& validIndices) {
            return Components<C...>(Component<C>(std::get<Component<C>>(componentTuples), validIndices)...);
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
            void addComponent(Entity entity, C component) {
                std::type_index type = std::type_index(typeid(C));
                if (component_mapping.find(type) == component_mapping.end()) {
                    components.push_back(std::vector<std::unique_ptr<std::any>>(entity_vector_length));
                    component_mapping[type] = components.size() - 1;
                }
                components[component_mapping[type]][entity] = std::make_unique<std::any>(std::move(component));
            }
            template <typename C>
            void addComponent() {
                std::type_index type = std::type_index(typeid(C));
                if (component_mapping.find(type) == component_mapping.end()) {
                    components.push_back(std::vector<std::unique_ptr<std::any>>(entity_vector_length));
                    component_mapping[type] = components.size() - 1;
                }
            }
            template<typename C>
            Component<C> GetComponent() {
                auto it = component_mapping.find(std::type_index(typeid(C)));
                if (it == component_mapping.end()) {
                    Logger::logError(std::string("Getting component that doesn't exist [") + std::string(typeid(C).name()) + std::string("]"));
                    std::vector<std::unique_ptr<std::any>> empty(entity_vector_length);
                    return Component<C>(empty);
                }
                unsigned index = it->second;
                return Component<C>(components[index]);
            }

            template<typename... C>
            Components<C...> GetWithComponents() {
                std::tuple<Component<C>...> componentTuples = std::make_tuple(GetComponent<C>()...);
                std::vector<unsigned> validIndices = FilterValidEntities<C...>(componentTuples);
                return CreateFilteredComponents<C...>(componentTuples, validIndices);
            }

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
    void Entity::addComponent(C component) {
        scene.addComponent(this, component);
    }
}