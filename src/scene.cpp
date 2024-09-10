#include "scene.hpp"
#include "components.hpp"

using namespace engine;

void Entity::addChild(Entity child) {
    scene.addChild(*this, child);
}
Entity Entity::getParent() {
    return scene.getParent(*this);
}
std::vector<Entity> Entity::getChildren() {
    return scene.getChildren(*this);
}

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
    return createEntity(Entity{0, *this});
}
Entity Scene::createEntity(Entity parent){
    Entity entity = {0, *this};
    if (empty_entity_ids.size() == 0) {
        entity_vector_length += 1;
        entity.id = entity_vector_length;
        for (unsigned i = 0; i < components.size(); i++) {
            components[i].push_back(std::unique_ptr<std::any>());
        }
        entity_hierarchy.push_back({});
    } else {
        entity.id = empty_entity_ids.back();
        empty_entity_ids.pop_back();
        for (unsigned i = 0; i < components.size(); i++) {
            components[i][entity] = std::unique_ptr<std::any>();
        }
    }
    entity_hierarchy[entity].parent = parent.id;
    if (!parent.isNull()) {
        entity_hierarchy[parent].children.push_back(entity.id);
    }
    return entity;
}
void Scene::destroyEntity(Entity entity){
    for (unsigned i = 0; i < components.size(); i++) {
        components[i][entity] = std::unique_ptr<std::any>();
        empty_entity_ids.push_back(entity.id);
    }
}

void Scene::addChild(Entity parent, Entity child) {
    if (!getParent(child).isNull()) {
        std::vector<unsigned>& children = entity_hierarchy[getParent(child)].children;
        children.erase(std::remove(children.begin(), children.end(), child.id), children.end());
    }
    entity_hierarchy[parent].children.push_back(child.id);
    entity_hierarchy[child].parent = parent.id;
}
Entity Scene::getParent(Entity entity) {
    return Entity{entity_hierarchy[entity].parent, *this};
}
std::vector<Entity> Scene::getChildren(Entity entity) {
    std::vector<Entity> children;
    for (unsigned id : entity_hierarchy[entity].children) {
        children.push_back({id, *this});
    }
    return children;
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

Entites Scene::GetEntities() {
    return Entites(*this);
}