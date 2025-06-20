#include "memory_manager.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <algorithm>

// Optional logging
static std::ofstream logFile("memlog.txt");
static int eventCounter = 0;

MemoryManager::MemoryManager(size_t totalBytes, Strategy strat)
  : strategy(strat)
{
    pageSize   = sysconf(_SC_PAGESIZE);
    totalPages = (totalBytes + pageSize - 1) / pageSize;
    size_t mapSize = totalPages * pageSize;

    memBase = mmap(nullptr, mapSize,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS,
                   -1, 0);
    if (memBase == MAP_FAILED)
        throw std::runtime_error("mmap failed");

    pageBitmap.assign(totalPages, true);

    pthread_mutex_init(&mtx,    nullptr);
    pthread_mutex_init(&logMtx, nullptr);

    // init log (optional)
    logFile << "ID EVENT DETAILS\n";
}

MemoryManager::~MemoryManager() {
    munmap(memBase, totalPages * pageSize);
    pthread_mutex_destroy(&mtx);
    pthread_mutex_destroy(&logMtx);
    logFile.close();
}

void MemoryManager::logEvent(const std::string &msg) {
    pthread_mutex_lock(&logMtx);
    logFile << ++eventCounter << " " << msg << "\n";
    pthread_mutex_unlock(&logMtx);
}

size_t MemoryManager::findBlock(size_t numPages) {
    size_t bestIdx  = size_t(-1);
    size_t bestSize = (strategy == Strategy::BEST_FIT ? SIZE_MAX : 0);

    for (size_t i = 0; i + numPages <= totalPages; ++i) {
        if (!pageBitmap[i]) { i += 0; continue; }
        size_t cnt = 0, j = i;
        while (j < totalPages && pageBitmap[j] && cnt < numPages) {
            ++j; ++cnt;
        }
        if (cnt < numPages) { i = j; continue; }
        if (strategy == Strategy::FIRST_FIT)
            return i;
        if (strategy == Strategy::BEST_FIT && cnt < bestSize) {
            bestSize = cnt; bestIdx = i;
        }
    }
    return bestIdx;
}

void* MemoryManager::alloc(size_t size) {
    if (size == 0) return nullptr;

    size_t numPages = (size + pageSize - 1) / pageSize;
    pthread_mutex_lock(&mtx);

    size_t idx = findBlock(numPages);
    if (idx == size_t(-1)) {
        pthread_mutex_unlock(&mtx);
        return nullptr;
    }

    // mark pages
    for (size_t p = idx; p < idx + numPages; ++p)
        pageBitmap[p] = false;

    void* addr = (char*)memBase + idx * pageSize;
    allocMap[addr] = { idx, numPages };

    // optional logging
    {
        std::ostringstream oss;
        oss << "ALLOC ADDR=" << addr
            << " SIZE=" << size
            << " PAGES=" << numPages;
        logEvent(oss.str());
    }

    pthread_mutex_unlock(&mtx);
    return addr;
}

void MemoryManager::free(void* ptr) {
    if (!ptr) return;

    pthread_mutex_lock(&mtx);
    auto it = allocMap.find(ptr);
    if (it == allocMap.end()) {
        pthread_mutex_unlock(&mtx);
        return;
    }
    // capture details
    size_t start = it->second.startPage;
    size_t cnt   = it->second.pageCount;
    allocMap.erase(it);

    // clear bitmap
    for (size_t p = start; p < start + cnt; ++p)
        pageBitmap[p] = true;

    // optional logging
    {
        std::ostringstream oss;
        oss << "FREE  ADDR=" << ptr
            << " PAGES=" << cnt;
        logEvent(oss.str());
    }

    pthread_mutex_unlock(&mtx);
}

double MemoryManager::getFragmentation() {
    pthread_mutex_lock(&mtx);
    size_t totalFree  = 0, largestFree = 0, run = 0;

    for (bool freePage : pageBitmap) {
        if (freePage) {
            ++totalFree;
            ++run;
        } else {
            largestFree = std::max(largestFree, run);
            run = 0;
        }
    }
    largestFree = std::max(largestFree, run);
    pthread_mutex_unlock(&mtx);

    if (totalFree == 0) return 0.0;
    return 1.0 - static_cast<double>(largestFree) / static_cast<double>(totalFree);
}
