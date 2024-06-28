#include "scene.hpp"


using namespace engine;

void Scene::executeStages(){
    for (Stage stage : this->stages) {
        stage.execute();
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
Stage& Scene::getStage(std::string stage){
    for (unsigned i=0; i<stages.size(); i++) {
        if (stages[i].name == stage) {
            return stages[i];
        }
    }
}

unsigned Scene::createEntity(){
    if (empty_entity_ids.size() == 0) {
        for (unsigned i = 0; i < components.size(); i++) {
            components[i].push_back(std::unique_ptr<std::any>());
        }
        entity_vector_length += 1;
        return entity_vector_length - 1;
    } else {
        unsigned entity_id = empty_entity_ids.back();
        empty_entity_ids.pop_back();
        for (unsigned i = 0; i < components.size(); i++) {
            components[i][entity_id] = std::unique_ptr<std::any>();
        }
        return entity_id;
    }
}
void Scene::destroyEntity(unsigned entity){
    for (unsigned i = 0; i < components.size(); i++) {
        components[i][entity] = std::unique_ptr<std::any>();
        empty_entity_ids.push_back(entity);
    }
}