#include <vector>
#include <string>

#include "logger.hpp"

namespace engine {
    class Scene;

    class Stage {
        typedef void (*System)(Scene&);

        std::vector<System> systems;
        
        public:
            std::string name;

            Stage(std::string name) : name(name), systems(std::vector<System>()){};

            // Move constructor
            Stage(Stage&& other) noexcept
                : name(std::move(other.name)), systems(std::move(other.systems)) {}

            // Copy constructor
            Stage(const Stage& other)
                : name(other.name), systems(other.systems) {}

            // Move assignment operator
            Stage& operator=(Stage&& other) noexcept {
                if (this != &other) {
                    name = std::move(other.name);
                    systems = std::move(other.systems);
                }
                return *this;
            }

            // Copy assignment operator
            Stage& operator=(const Stage& other) {
                if (this != &other) {
                    name = other.name;
                    systems = other.systems;
                }
                return *this;
            }

            
            void addSystem(System system);
            void removeSystem(System system);
            void execute(Scene&);
    };
}