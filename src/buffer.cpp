#include "buffer.hpp"
#include "vulkan/device.hpp"
#include "engine.hpp"

namespace engine {
    Buffer::Buffer(size_t item_size, int usage_flag, bool staged) : item_size(item_size), usage_flag(usage_flag), staged(staged) {}


    Buffer::Buffer(Buffer&& other) noexcept {
        buffer = other.buffer;
        memory = other.memory;

        other.buffer = VK_NULL_HANDLE;
        other.memory = VK_NULL_HANDLE;
    }

    Buffer& Buffer::operator=(Buffer&& other) noexcept {
        if (this != &other) {
            // Free existing resources
            vkDestroyBuffer(Engine::getDevice().device(), buffer, nullptr);
            vkFreeMemory(Engine::getDevice().device(), memory, nullptr);

            // Move resources from the other object
            buffer = other.buffer;
            memory = other.memory;

            // Invalidate the moved-from object
            other.buffer = VK_NULL_HANDLE;
            other.memory = VK_NULL_HANDLE;
        }
        return *this;
    }

    Buffer::~Buffer() {
        VkDevice device = Engine::getDevice().device();
        if (buffer == nullptr) {
            Logger::logWarning("destroying null buffer");
        }
        vkDestroyBuffer(device, buffer, nullptr);
        vkFreeMemory(device, memory, nullptr);
    }

    void Buffer::setVector(const void *data, size_t size) {
        Device& device = Engine::getDevice();
        VkDeviceSize bufferSize = size * item_size;

        if (buffer == nullptr) {
            device.createBuffer(bufferSize, usage_flag | staged*VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            staged ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            buffer, memory);
        }

        if (staged) {
            StagingBuffer stagingBuffer(item_size);
            stagingBuffer.setVector(data, size);
            Engine::getDevice().copyBuffer(stagingBuffer.buffer, buffer, bufferSize);
            return;
        }

        void* data_loc;
        vkMapMemory(device.device(), memory, 0, bufferSize, 0, &data_loc);
        memcpy(data_loc, data, bufferSize);
        vkUnmapMemory(device.device(), memory);
    }

    void Buffer::bind() {
        Logger::logWarning("bind() does nothing for this buffer!");
        return;
    }

    void VertexBuffer::bind() {
        VkDeviceSize offset[] = {0};
        vkCmdBindVertexBuffers(Engine::getCurrentCommandBuffer(), 0, 1, &buffer, offset);
    }
    void IndexBuffer::bind() {
        vkCmdBindIndexBuffer(Engine::getCurrentCommandBuffer(), buffer, 0, VK_INDEX_TYPE_UINT32);
    }
}