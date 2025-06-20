#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <cstddef>
#include <map>
#include <vector>
#include <pthread.h>
#include <string>

// Simple page‑based memory manager with FIRST/FIT & BEST/FIT strategies
enum class Strategy { FIRST_FIT, BEST_FIT };

struct AllocInfo {
    size_t startPage;
    size_t pageCount;
};

class MemoryManager {
public:
    // totalBytes: size of the pool; strat: allocation strategy
    MemoryManager(size_t totalBytes, Strategy strat = Strategy::FIRST_FIT);
    ~MemoryManager();

    // allocate 'size' bytes, returns nullptr on failure
    void* alloc(size_t size);
    // free a previously allocated pointer (nullptr is safe)
    void  free(void* ptr);

    // for benchmarking and tests
    double getFragmentation();

private:
    size_t pageSize;
    size_t totalPages;
    void*   memBase;
    Strategy strategy;

    // bitmap: true=free, false=used
    std::vector<bool> pageBitmap;
    // track each allocation: pointer -> (startPage, pageCount)
    std::map<void*, AllocInfo> allocMap;

    pthread_mutex_t mtx;        // protects bitmap & allocMap
    pthread_mutex_t logMtx;     // protects log file

    // find a block of numPages according to strategy
    size_t findBlock(size_t numPages);

    // log events (optional; can be disabled if not needed)
    void logEvent(const std::string &msg);
};
#endif // MEMORY_MANAGER_H
