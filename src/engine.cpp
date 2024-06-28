#include <glad.c>

#include "engine.hpp"

using namespace engine;

std::unordered_map<std::string, Scene> Engine::scenes = std::unordered_map<std::string, Scene>();
Scene* Engine::active_scene = nullptr;
SDL_Window* Engine::window = nullptr;
SDL_GLContext Engine::context = nullptr;

void Engine::init(const char* title, Vector2 window_size) {
    // initialize and configure
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError();
        exit(1);
    }

    SDL_GL_LoadLibrary(NULL);

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // window creation
    Engine::window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_size.x, window_size.y, SDL_WINDOW_OPENGL);

    if (Engine::window == NULL)
    {
        std::cout << "SDL window could not initialize! SDL_Error: " << SDL_GetError();
        exit(1);
    }

    
    context = SDL_GL_CreateContext(window);

    if (context == NULL)
    {
        std::cout << "Failed to create OpenGL context";
        exit(1);
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        std::cout << "Failed to initialize GLAD";
        exit(1);
    }

    // Setting blending parameters
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void Engine::run() {
    bool quit = false;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        if (active_scene == nullptr) {
            Logger::logError("No active scene!");
            continue;
        }
        Engine::active_scene->executeStages();
    }

    SDL_Quit();
}
Scene& Engine::addScene(std::string name) {
    scenes[name] =Scene(name);
    if (active_scene == nullptr) {
        active_scene = &scenes[name];
    }
    return scenes[name];
}
void Engine::changeScene(std::string scene_name) {
    auto scene_index = scenes.find(scene_name);
    if (scene_index == scenes.end()) {
        Logger::logError("Scene [" + scene_name + "] not found");
        return;
    }
    active_scene = &scene_index->second;
}
SDL_Window* Engine::getWindow() {
    return window;
}