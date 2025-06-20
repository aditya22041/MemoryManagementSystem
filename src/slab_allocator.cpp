#include "slab_allocator.h"
#include <sys/mman.h>
#include <stdexcept>
#include <cassert>

SlabAllocator::SlabAllocator(size_t objSize, size_t slabSize)
    : objectSize(objSize),
      slabMemorySize(slabSize),
      objectsPerSlab(slabSize / objSize) {
    pthread_mutex_init(&slab_mtx, nullptr);
}

SlabAllocator::~SlabAllocator() {
    for (auto& slab : slabs) {
        munmap(slab.memory, slabMemorySize);
    }
    pthread_mutex_destroy(&slab_mtx);
}

bool SlabAllocator::grow() {
    void* mem = mmap(nullptr, slabMemorySize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        return false; // Could not allocate more memory
    }

    Slab newSlab;
    newSlab.memory = mem;
    newSlab.freeList.reserve(objectsPerSlab);
    for (size_t i = 0; i < objectsPerSlab; ++i) {
        newSlab.freeList.push_back(static_cast<char*>(mem) + i * objectSize);
    }
    slabs.push_back(newSlab);
    return true;
}

void* SlabAllocator::alloc() {
    // Find a slab with free objects
    for (auto& slab : slabs) {
        if (!slab.freeList.empty()) {
            void* ptr = slab.freeList.back();
            slab.freeList.pop_back();
            return ptr;
        }
    }

    // No free objects in any existing slab, try to grow
    if (grow()) {
        Slab& newSlab = slabs.back();
        void* ptr = newSlab.freeList.back();
        newSlab.freeList.pop_back();
        return ptr;
    }

    return nullptr; // Growth failed, out of memory
}

void SlabAllocator::free(void* ptr) {
    // O(1) free: calculate which slab the pointer belongs to
    for (auto& slab : slabs) {
        // Check if the pointer is within the memory range of this slab
        if (ptr >= slab.memory && ptr < static_cast<char*>(slab.memory) + slabMemorySize) {
            // Check for alignment to prevent corruption
            size_t offset = static_cast<char*>(ptr) - static_cast<char*>(slab.memory);
            if (offset % objectSize == 0) {
                 slab.freeList.push_back(ptr);
            } else {
                // Pointer is in this slab's range but not aligned. This is an error.
                assert(false && "Attempted to free an invalid (unaligned) pointer to a slab.");
            }
            return;
        }
    }
    // If the loop finishes, the pointer does not belong to any of our slabs.
    assert(false && "Attempted to free a pointer that does not belong to this slab allocator.");
}
