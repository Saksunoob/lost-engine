#include <vulkan/vulkan_core.h>

namespace engine {
    class Buffer {
        size_t item_size;
        int usage_flag;
        bool staged;
        
        VkDeviceMemory memory = nullptr;
        unsigned bindingSize = 0;

        public:
            VkBuffer buffer = nullptr;

            Buffer(size_t item_size, int usage_flag, bool staged);

            Buffer(const Buffer&) = delete;
            Buffer& operator=(const Buffer&) = delete;


            Buffer(Buffer&& other) noexcept;
            Buffer& operator=(Buffer&& other) noexcept;

            ~Buffer();

            virtual void bind();

            inline void set(const void *data) {
                setVector(data, 1);
            }

            void setVector(const void *data, size_t size);
    };
    class VertexBuffer : public Buffer {
        public:
        VertexBuffer(unsigned item_size, bool staged) : Buffer(item_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, staged) {};

        void bind() override;
    };
    class IndexBuffer : public Buffer {
        public:
        IndexBuffer(unsigned item_size, bool staged) : Buffer(item_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, staged) {};

        void bind() override;
    };
    class UniformBuffer : public Buffer {
        public:
        UniformBuffer(unsigned item_size, bool staged) : Buffer(item_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, staged) {};
    };
    class StagingBuffer : public Buffer {
        public:
        StagingBuffer(unsigned item_size) : Buffer(item_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false) {};
    };
}