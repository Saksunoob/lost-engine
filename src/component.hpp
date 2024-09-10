#pragma once

#include <vector>
#include <memory>
#include <any>

namespace engine {
    template <typename C>
    class Components {
        std::vector<std::unique_ptr<std::any>>* components;
        std::vector<unsigned> filter;

        public:
            Components() : components(nullptr) {};
            Components(std::vector<std::unique_ptr<std::any>>& components) : components(&components) {};

            Components<C>& withFilter(const std::vector<unsigned>& new_filter) {
                filter = new_filter;
                return *this;
            }

            C* operator[](unsigned index) {
                if (index >= filter.size()) {
                    return nullptr;
                }
                index = filter[index];
                return getUnfiltered(index);
            }

            C* getUnfiltered(unsigned index) {
                if (!components) {
                    return nullptr;
                }
                if (index >= components->size() || components->at(index) == nullptr) {
                    return nullptr;
                }
                std::any* component = components->at(index).get();
                return std::any_cast<C>(component);
            }

            bool hasEntity(unsigned entity) {
                if (!components || entity >= components->size()) {
                    return false;
                }
                return components->at(entity).get();
            }

            unsigned size() {
                if (!components) {
                    return 0;
                }
                return filter.size();
            }
    };
}