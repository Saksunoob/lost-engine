#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>

#include <SDL2/SDL.h>

#include "stage.hpp"
#include "components.hpp"

namespace engine {

    typedef unsigned Entity;

    class Scene {
        std::string name;
        std::vector<Stage> stages = std::vector<Stage>();
        unsigned entity_vector_length = 0;
        std::vector<Entity> empty_entity_ids = std::vector<Entity>();
        std::vector<std::vector<std::unique_ptr<std::any>>> components = std::vector<std::vector<std::unique_ptr<std::any>>>();
        std::unordered_map<std::type_index, unsigned> component_mapping = std::unordered_map<std::type_index, unsigned>();

        public:
            Scene(std::string name) : name(name) {}
            Scene() : name("empty_scene") {}

            void executeStages();

            void addStageAfter(std::string stage, std::string after);
            void addStageBefore(std::string stage, std::string before);
            void addStageAt(std::string stage, unsigned index);
            void removeStage(std::string stage);
            Stage* getStage(std::string stage);

            unsigned createEntity();
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
            Components<C> getComponent() {
                std::type_index type = std::type_index(typeid(C));
                if (component_mapping.find(type) == component_mapping.end()) {
                    Logger::logError(std::string("Getting component that doesn't exist [") + std::string(typeid(C).name()) + std::string("]"));
                    return Components<C>(nullptr);
                }
                return Components<C>(&components[component_mapping[std::type_index(typeid(C))]]);
            }
    };
}