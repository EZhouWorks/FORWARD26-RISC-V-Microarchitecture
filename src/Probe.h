//
// Created by zhoue on 2026/9/16.
//

//THIS PROBE IS NOT A COMPONENT FOR THE FORWARD RISC-V MICROARCH

#ifndef RISCV_CPU_PROBE_H
#define RISCV_CPU_PROBE_H
class Probe {
public:
    int stall_count;
    int total_cycles;
    int total_mem_read;
    int data_cache_hit_count;
    int is_fetch_mem_visit;
    int fetch_cache_hit_count;
    Probe() {
        stall_count = 0;
        total_cycles = 0;
        total_mem_read = 0;
        data_cache_hit_count = 0;
        fetch_cache_hit_count = 0;
    };

};
#endif //RISCV_CPU_PROBE_H
