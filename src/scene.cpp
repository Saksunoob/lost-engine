#include "scene.hpp"
#include "components.hpp"

using namespace engine;

void Entity::addBundle(Bundle& bundle) {
    scene.addBundle(*this, bundle);
};

void Scene::executeStages(){
    for (Stage stage : this->stages) {
        stage.execute(*this);
    }
}

void Scene::addStageAfter(std::string stage, std::string after){
    for (unsigned i=0; i<stages.size(); i++) {
        if (stages[i].name == after) {
            stages.insert(stages.begin() + i + 1, stage);
            return;
        }
    }
}
void Scene::addStageBefore(std::string stage, std::string before){
    for (unsigned i=0; i<stages.size(); i++) {
        if (stages[i].name == before) {
            stages.insert(stages.begin() + i, stage);
            return;
        }
    }
}
void Scene::addStageAt(std::string stage, unsigned index){
    stages.insert(stages.begin() + index, stage);
    return;
}
void Scene::removeStage(std::string stage){
    for (unsigned i=0; i<stages.size(); i++) {
        if (stages[i].name == stage) {
            stages.erase(stages.begin() + i);
            return;
        }
    }
}
engine::Stage* Scene::getStage(std::string stage) {
    for (unsigned i=0; i<stages.size(); i++) {
        if (stages[i].name == stage) {
            return &stages[i];
        }
    }
    Logger::logWarning("Trying to get non existing stage: " + stage);
    return nullptr;
}

Entity Scene::createEntity(){
    if (empty_entity_ids.size() == 0) {
        for (unsigned i = 0; i < components.size(); i++) {
            components[i].push_back(std::unique_ptr<std::any>());
        }
        entity_vector_length += 1;
        return Entity{entity_vector_length - 1, *this};
    } else {
        unsigned entity_id = empty_entity_ids.back();
        empty_entity_ids.pop_back();
        for (unsigned i = 0; i < components.size(); i++) {
            components[i][entity_id] = std::unique_ptr<std::any>();
        }
        return Entity{entity_id, *this};
    }
}
void Scene::destroyEntity(Entity entity){
    for (unsigned i = 0; i < components.size(); i++) {
        components[i][entity] = std::unique_ptr<std::any>();
        empty_entity_ids.push_back(entity);
    }
}

void Scene::addBundle(Entity entity, Bundle& bundle) {
    for (auto& [type, component] : bundle.components) {
        if (component_mapping.find(type) == component_mapping.end()) {
            components.push_back(std::vector<std::unique_ptr<std::any>>(entity_vector_length));
            component_mapping[type] = components.size() - 1;
        }
        components[component_mapping[type]][entity] = std::make_unique<std::any>(*component.get());
    }
}

Components Scene::GetComponents() {
    return Components(*this);
}