#include <vector>
#include <memory>
#include <any>

namespace engine {
    template <typename C>
    class Components {
        std::vector<std::unique_ptr<std::any>>* components;

        public:

            Components(std::vector<std::unique_ptr<std::any>>* components) : components(components) {};

            C& operator[](unsigned index) {
                std::any* component = components->operator[](index).get();
                return *std::any_cast<C>(component);
            }

            unsigned size() {
                return components->size();
            }
    };
}