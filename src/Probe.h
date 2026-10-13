//
// Created by zhoue on 2026/9/16.
//

//THIS PROBE IS NOT A COMPONENT FOR THE FORWARD RISC-V MICROARCH
#include <iostream>
#ifndef RISCV_CPU_PROBE_H
#define RISCV_CPU_PROBE_H
class Probe {
public:
    int stall_count;
    int total_cycles;
    int total_mem_visit; //This excludes instruction fetch
    int total_read_visit;
    int total_store_visit;
    int data_cache_hit_count; //This accounts for load hit
    int data_store_hit_count; //This accounts for store hit
    int is_fetch_mem_visit;
    int fetch_cache_hit_count;
    int retired_instructions;
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
        total_mem_visit = 0;
        data_cache_hit_count = 0;
        fetch_cache_hit_count = 0;
        total_read_visit = 0;
        total_store_visit = 0;
        data_store_hit_count = 0;
        retired_instructions = 0;
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
    void calculate_IPC() {
        float ipc = (float(retired_instructions) / total_cycles);
        cout<<"IPC: "<<ipc<<endl;
    }
    void calculate_CPI() {
        cout<<"CPI: "<<(float(total_cycles) / retired_instructions)<<endl;
    }
    void calculate_load_hit_count() {
        if (total_read_visit != 0) {
            cout<<"Cache Hit Count (Read): "<<100*float(data_cache_hit_count)/total_read_visit<<"%"<<endl;
        }
        else {
            cout<<"NO READ OPERATIONS"<<endl;
        }
    }
    void calculate_store_hit_count() {
        if (total_store_visit != 0) {
            cout<<"Cache Hit Count (Store): "<<100*float(data_store_hit_count)/total_store_visit<<"%"<<endl;
        }
        else {
            cout<<"NO STORE OPERATIONS"<<endl;
        }
    }
    void calculate_overall_hit() {
        if (total_mem_visit != 0) {
            cout<<"Cache Hit Count (Overall): "<<100*float(data_store_hit_count + data_cache_hit_count)/total_mem_visit<<"%"<<endl;
        }
        else {
            cout<<"NO MEM OPERATIONS"<<endl;
        }
    }


};
#endif //RISCV_CPU_PROBE_H
