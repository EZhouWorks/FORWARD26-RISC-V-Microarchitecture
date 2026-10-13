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
    int retied_instructions;
    int total_branch_predictions;
    int correct_branch_predictions;
    int IF_active_count;
    int ID_active_count;
    int EX_active_count;
    int MEM_active_count;
    int WB_active_count;
    Probe() {
        stall_count = 0;
        total_cycles = 0;
        total_mem_read = 0;
        data_cache_hit_count = 0;
        fetch_cache_hit_count = 0;
        retied_instructions = 0;
        total_branch_predictions = 0;
        correct_branch_predictions = 0;
        IF_active_count = 0;
        ID_active_count = 0;
        EX_active_count = 0;
        MEM_active_count = 0;
        WB_active_count = 0;
        
    };

    float calculate_BPA(int BPU_enable) {
        if (total_branch_predictions != 0) {
            return correct_branch_predictions / total_branch_predictions;
        }
        else {
            cout<<"NO BRANCH PREDICTIONS DONE"<<endl;
        }
    }


};
#endif //RISCV_CPU_PROBE_H
