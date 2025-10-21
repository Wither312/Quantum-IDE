#pragma once

#include <vector>
#include <cstddef>

namespace core {

    class MemoryPool {
    public:
        MemoryPool() {}
        explicit MemoryPool(std::size_t blockSize, std::size_t blockCount);
        ~MemoryPool();

        void* allocate();
        void deallocate(void* ptr);

    private:
        std::vector<std::byte> m_memory;
        void* m_freeList = nullptr;
        std::size_t m_blockSize;
        std::size_t m_blockCount;
    };

} // namespace core
