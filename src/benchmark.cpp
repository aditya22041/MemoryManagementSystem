#include "memory_manager.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <numeric>
#include <string>

using high_res_clock = std::chrono::high_resolution_clock;

struct BenchmarkResult {
    std::string workload_name;
    Strategy strategy;
    int op_count;
    double total_time_ms;
    double avg_alloc_time_ns;
    double final_fragmentation;
};

void run_workload(const std::string& name, Strategy strat, int ops, std::ofstream& out) {
    std::cout << "Running workload: " << name << " with strategy: "
              << (strat == Strategy::BEST_FIT ? "BEST_FIT" : "FIRST_FIT") << std::endl;
              
    MemoryManager mm(16 * 1024 * 1024, strat); // 16MB manager
    std::vector<void*> allocations;
    allocations.reserve(ops);
    std::vector<long long> alloc_times;
    alloc_times.reserve(ops);

    auto start_time = high_res_clock::now();

    if (name == "Variable Size Stress") {
        for (int i = 0; i < ops; ++i) {
            size_t size = (rand() % 8192) + 1; // 1B to 8KB
            auto t1 = high_res_clock::now();
            void* p = mm.alloc(size);
            auto t2 = high_res_clock::now();
            if (p) {
                alloc_times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
                allocations.push_back(p);
            }
            if (i % 10 == 0 && !allocations.empty()) {
                size_t idx = rand() % allocations.size();
                mm.free(allocations[idx]);
                allocations.erase(allocations.begin() + idx);
            }
        }
    } else if (name == "Small Object Stress") {
        for (int i = 0; i < ops; ++i) {
            size_t size = (rand() % 500) + 16; // 16B to 512B (slab territory)
            auto t1 = high_res_clock::now();
            void* p = mm.alloc(size);
            auto t2 = high_res_clock::now();
            if (p) {
                alloc_times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
                allocations.push_back(p);
            }
            if (i > ops / 2) {
                 size_t idx = rand() % allocations.size();
                 mm.free(allocations[idx]);
                 allocations.erase(allocations.begin() + idx);
            }
        }
    }

    for (void* p : allocations) {
        mm.free(p);
    }
    auto end_time = high_res_clock::now();

    double total_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    double avg_ns = std::accumulate(alloc_times.begin(), alloc_times.end(), 0LL) / (double)alloc_times.size();
    
    BenchmarkResult result = {
        name, strat, ops, total_ms, avg_ns, mm.getFragmentation()
    };
    std::cout << "Recording: " << result.workload_name << " avg_ns=" << result.avg_alloc_time_ns << "\n";

    out << result.workload_name << ","
        << (result.strategy == Strategy::BEST_FIT ? "BEST_FIT" : "FIRST_FIT") << ","
        << result.op_count << ","
        << result.total_time_ms << ","
        << result.avg_alloc_time_ns << ","
        << result.final_fragmentation << "\n";
}

int main() {
    srand(time(0));
    std::ofstream out("benchmark_results.csv");
    out << "workload,strategy,op_count,total_time_ms,avg_alloc_time_ns,final_fragmentation\n";

    int ops = 50000;
    run_workload("Variable Size Stress", Strategy::BEST_FIT, ops, out);
    run_workload("Variable Size Stress", Strategy::FIRST_FIT, ops, out);
    run_workload("Small Object Stress", Strategy::BEST_FIT, ops, out); // Best_fit uses slabs here
    run_workload("Small Object Stress", Strategy::FIRST_FIT, ops, out); // First_fit uses slabs here

    std::cout << "\n Benchmark complete. Results saved to benchmark_results.csv" << std::endl;
    return 0;
}
