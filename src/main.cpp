
#include "memory_manager.h"
#include <pthread.h>
#include <vector>
#include <cstdlib>
#include <unistd.h>

static const int THREADS = 4, OPS = 100;

struct Arg { MemoryManager* mm; };

void* worker(void* v) {
    Arg* a = (Arg*)v;
    std::vector<void*> vptr;
    vptr.reserve(OPS/2);
    for(int i=0;i<OPS;i++){
        if(vptr.empty() || rand()%2==0) {
            void* p = a->mm->alloc((rand()%2048)+1);
            if(p) vptr.push_back(p);
        } else {
            int idx = rand()%vptr.size();
            a->mm->free(vptr[idx]);
            vptr.erase(vptr.begin()+idx);
        }
        usleep(1000);
    }
    for(auto p: vptr) a->mm->free(p);
    return nullptr;
}

int main(){
    MemoryManager mm(4LL*1024*1024, Strategy::BEST_FIT);
    pthread_t threads[THREADS];
    Arg arg{&mm};
    for(int i=0;i<THREADS;i++)
        pthread_create(&threads[i], nullptr, worker, &arg);
    for(int i=0;i<THREADS;i++)
        pthread_join(threads[i], nullptr);
    return 0;
}

