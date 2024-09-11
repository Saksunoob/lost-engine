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
        template <typename C>
        C* getComponent();
        void addBundle(Bundle bundle);

        void addChild(Entity child);
        Entity getParent();
        std::vector<Entity> getChildren();

        bool isNull() {return id==0;};

        operator unsigned() const{
            return id-1;
        }
    };

    class Entites;

    class Scene {
        friend Entites;

        std::string name;
        std::vector<Stage> stages = std::vector<Stage>();
        std::vector<unsigned> empty_entity_ids = std::vector<unsigned>();
        std::unordered_map<std::type_index, unsigned> component_mapping = std::unordered_map<std::type_index, unsigned>();
        std::unordered_map<std::type_index, std::any> resources{};

        protected:
        unsigned entity_vector_length = 0;
        std::vector<std::vector<std::unique_ptr<std::any>>> components = std::vector<std::vector<std::unique_ptr<std::any>>>();

        struct Hierarchy {
            unsigned parent;
            std::vector<unsigned> children;
        };

        std::vector<Hierarchy> entity_hierarchy;

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
            Entity createEntity(Entity parent);
            void destroyEntity(Entity entity);

            void addChild(Entity parent, Entity child);
            Entity getParent(Entity);
            std::vector<Entity> getChildren(Entity);

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

            void addBundle(Entity entity, Bundle bundle);
            
            template<typename C>
            Components<C> GetComponent() {
                auto it = component_mapping.find(std::type_index(typeid(C)));
                if (it == component_mapping.end()) {
                    return Components<C>();
                }
                unsigned index = it->second;
                return Components<C>(components[index]);
            }

            Entites GetEntities();

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
        return scene.addComponent(*this, std::move(component));
    }

    template <typename C>
    C* Entity::getComponent() {
        if (isNull()) {
            return nullptr;
        }
        return scene.GetComponent<C>().getUnfiltered(id-1);
    }
}