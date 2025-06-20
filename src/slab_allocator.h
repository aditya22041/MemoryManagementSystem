#ifndef SLAB_ALLOCATOR_H
#define SLAB_ALLOCATOR_H

#include <cstddef>
#include <vector>
#include <pthread.h>

class SlabAllocator {
public:
    // A slab is a contiguous block of memory for same-sized objects
    struct Slab {
        void* memory;          // Start of the slab's memory
        std::vector<void*> freeList; // List of free objects in this slab
    };

    SlabAllocator(size_t objSize, size_t slabSize = 65536); // Default 64KB slabs
    ~SlabAllocator();

    void* alloc();
    void  free(void* ptr);

    // For thread-safety within the slab cache
    void lock() { pthread_mutex_lock(&slab_mtx); }
    void unlock() { pthread_mutex_unlock(&slab_mtx); }

private:
    const size_t objectSize;
    const size_t slabMemorySize;
    const size_t objectsPerSlab;
    std::vector<Slab> slabs;
    pthread_mutex_t slab_mtx;

    bool grow(); // Add a new slab
};

#endif // SLAB_ALLOCATOR_H
