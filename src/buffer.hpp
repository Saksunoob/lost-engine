#include <vulkan/vulkan.hpp>
#include "vulkan/device.hpp"
#include "engine.hpp"

namespace engine {
    class Buffer {
        size_t item_size;
        int usage_flag;
        
        VkDeviceMemory memory = nullptr;
        unsigned bindingSize = 0;

        public:
            VkBuffer buffer = nullptr;

            Buffer(size_t item_size, int usage_flag) : item_size(item_size), usage_flag(usage_flag) {}

            Buffer operator=(Buffer&) = delete;

            ~Buffer() {
                VkDevice device = Engine::getDevice().device();
                if (buffer == nullptr) {
                    Logger::logWarning("destroying null buffer");
                }
                vkDestroyBuffer(device, buffer, nullptr);
                vkFreeMemory(device, memory, nullptr);
            }

            virtual void bind() {
                Logger::logWarning("bind() does nothing for this buffer!");
                return;
            }

            inline void set(const void *data) {
                setVector(data, 1);
            }

            void setVector(const void *data, size_t size) {
                Device& device = Engine::getDevice();
                VkDeviceSize bufferSize = size * item_size;

                if (buffer == nullptr) {
                    device.createBuffer(bufferSize, usage_flag,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    buffer, memory);
                }

                void* data_loc;
                vkMapMemory(device.device(), memory, 0, bufferSize, 0, &data_loc);
                memcpy(data_loc, data, bufferSize);
                vkUnmapMemory(device.device(), memory);
            }
    };
    class VertexBuffer : public Buffer {
        public:
        VertexBuffer(unsigned item_size) : Buffer(item_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {};

        void bind() override {
            VkDeviceSize offset[] = {0};
            vkCmdBindVertexBuffers(Engine::getCurrentCommandBuffer(), 0, 1, &buffer, offset);
        }
    };
    class IndexBuffer : public Buffer {
        public:
        IndexBuffer(unsigned item_size) : Buffer(item_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {};

        void bind() override {
            vkCmdBindIndexBuffer(Engine::getCurrentCommandBuffer(), buffer, 0, VK_INDEX_TYPE_UINT32);
        }
    };
    class UniformBuffer : public Buffer {
        public:
        UniformBuffer(unsigned item_size) : Buffer(item_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {};
    };
}