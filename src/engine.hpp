#include <unordered_map>
#include <string>
#include <iostream>

#include <SDL2/SDL.h>
#include <glad/glad.h>

#include "logger.hpp"
#include "vectors.hpp"
#include "scene.hpp"

namespace engine {
    class Engine {
        static std::unordered_map<std::string, Scene> scenes;
        static Scene* active_scene;
        static SDL_Window* window;
        static SDL_GLContext context;

        public:
            static void init(const char* title, Vector2 window_size);
            static void run();
            static Scene& addScene(std::string name);
            static void changeScene(std::string scene_name);
            static SDL_Window* getWindow();
    };
}