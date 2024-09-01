#include "engine.hpp"
#include "scene.hpp"
#include "systems.hpp"
#include "resources.hpp"
#include "vulkan/device.hpp"
#include "vulkan/swap_chain.hpp"
#include "vulkan/pipeline.hpp"

using namespace engine;

std::unordered_map<std::string, Scene> Engine::scenes = std::unordered_map<std::string, Scene>();
Scene* Engine::active_scene = nullptr;
SDL_Window* Engine::window = nullptr;
IVector2 Engine::window_size = IVector2(0, 0);
Device* Engine::device = nullptr;
SwapChain* Engine::swap_chain = nullptr;
std::vector<VkCommandBuffer> Engine::commandsBuffers = std::vector<VkCommandBuffer>();
unsigned Engine::currentImageIndex = 0;
bool Engine::framebufferResized = false;

void Engine::init(const char* title, IVector2 window_size) {
    Engine::window_size = window_size;
	  // initialize and configure
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        Logger::logError("SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
        exit(1);
    }
    // window creation
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_size.x, window_size.y, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    if (window == nullptr)
    {
        Logger::logError("SDL window could not initialize! SDL_Error: " + std::string(SDL_GetError()));
        exit(1);
    }

    Engine::device = new Device(window);
    Engine::swap_chain = new SwapChain{getDevice(), {(unsigned)window_size.x, (unsigned)window_size.y}};
    createCommandBuffers();
    
}

bool quit_engine = false;

void Engine::run() {
    SDL_Event event;
    while (!quit_engine) {
        newFrame();
        if (active_scene == nullptr) {
            Logger::logError("No active scene!");
            throw;
        }
        Engine::active_scene->executeStages();
        drawFrame();
    }
    vkDeviceWaitIdle(device->device());
    SDL_Quit();
}

void Engine::quit() {
    quit_engine = true;
}

void Engine::createCommandBuffers() {
    commandsBuffers.resize(swap_chain->imageCount());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = device->getCommandPool();
    allocInfo.commandBufferCount = static_cast<unsigned>(commandsBuffers.size());

    if (vkAllocateCommandBuffers(device->device(), &allocInfo, commandsBuffers.data()) != VK_SUCCESS) {
        Logger::logError("Failed to allocate command buffers");
        throw;
    }
}

void Engine::recreateSwapChain() {
    vkDeviceWaitIdle(device->device());

    delete Engine::swap_chain;
    Engine::swap_chain = new SwapChain{getDevice(), {(unsigned)window_size.x, (unsigned)window_size.y}};
}

void Engine::newFrame() {
    VkResult result = swap_chain->acquireNextImage(&currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        Logger::logError("Failed to acquire next swap chain image");
        throw;
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandsBuffers[currentImageIndex], &beginInfo) != VK_SUCCESS) {
        Logger::logError("Failed to begin recording command buffer!");
        throw;
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = swap_chain->getRenderPass();
    renderPassInfo.framebuffer = swap_chain->getFrameBuffer(currentImageIndex);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swap_chain->getSwapChainExtent();

    VkClearValue clearValues[2];
    clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandsBuffers[currentImageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Engine::drawFrame() {
    vkCmdEndRenderPass(commandsBuffers[currentImageIndex]);
    if (vkEndCommandBuffer(commandsBuffers[currentImageIndex]) != VK_SUCCESS) {
        Logger::logError("Failed to record command buffer");
        throw;
    }

    resetWindowResized();
    VkResult result = swap_chain->submitCommandBuffers(&commandsBuffers[currentImageIndex], &currentImageIndex);
    if (result == VK_SUBOPTIMAL_KHR) {
        SDL_GetWindowSize(window, &window_size.x, &window_size.y);
        framebufferResized = true;
        recreateSwapChain();
    } else if (result != VK_SUCCESS)
    {
        Logger::logWarning("Failed to present swap chain image");
    }
}

Scene& Engine::addScene(std::string name, bool with_defaults) {
    scenes[name] = Scene(name);
    Scene& scene = scenes[name];
    if (active_scene == nullptr) {
        active_scene = &scene;
    }
    if (with_defaults) {
        scene.addStageAt("init_frame", 0);
        scene.addStageAfter("update", "init_frame");
        scene.addStageAfter("render", "update");

        scene.getStage("init_frame")->addSystem(timeSystem);
        scene.getStage("init_frame")->addSystem(pollSDLEvents);

        scene.getStage("update")->addSystem(updateUITransforms);

        scene.getStage("render")->addSystem(engine::renderColorMeshes);
        scene.getStage("render")->addSystem(engine::renderUVMeshes);
        scene.getStage("render")->addSystem(engine::renderTileMaps);
        scene.getStage("render")->addSystem(engine::renderColorUI);

        scene.addResource(Time{});
        scene.addResource(Input{});
    }
    return scene;
}
void Engine::changeScene(std::string scene_name) {
    auto scene_index = scenes.find(scene_name);
    if (scene_index == scenes.end()) {
        Logger::logError("Scene [" + scene_name + "] not found");
        return;
    }
    active_scene = &scene_index->second;
}
Device& Engine::getDevice() {
    return *device;
}