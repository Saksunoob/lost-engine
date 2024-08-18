#pragma once

#include <unordered_map>
#include <string>
#include <iostream>
#include <thread>

#include <SDL2/SDL.h>
#include <vulkan/vulkan.hpp>

#include "scene.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include "vulkan/device.hpp"
#include "vulkan/swap_chain.hpp"
#include "vulkan/pipeline.hpp"

namespace engine {
    class Engine {
        static std::unordered_map<std::string, Scene> scenes;
        static Scene* active_scene;
        static Device* device;
        static SwapChain* swap_chain;
        static std::vector<VkCommandBuffer> commandsBuffers;
        static unsigned currentImageIndex;

        static SDL_Window* window;
        static IVector2 window_size;

        static bool framebufferResized;
        
        static void createCommandBuffers();
        static void recreateSwapChain();
        static void newFrame();
        static void drawFrame();
        static void resetWindowResized() {framebufferResized = false;};

        public:
            static void init(const char* title, IVector2 window_size);
            static void run();
            static Scene& addScene(std::string name);
            static void changeScene(std::string scene_name);
            static Device& getDevice();
            static IVector2 getWindowSize() {return window_size;};
            static VkCommandBuffer& getCurrentCommandBuffer() {return commandsBuffers[currentImageIndex];};
            static unsigned getCurrentSwapChainImage() {return currentImageIndex;};
            static SwapChain* getSwapChain() {return swap_chain;};
            static bool wasWindowResized() {return framebufferResized;};
    };
}