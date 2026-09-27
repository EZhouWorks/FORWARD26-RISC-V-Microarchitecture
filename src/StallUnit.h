//
// Created by zhoue on 2026/7/24.
//

#ifndef RISCV_CPU_STALLUNIT_H
#define RISCV_CPU_STALLUNIT_H
#include "ProgramCounter.h"
#include "PipelineRegisters.h"
struct EnableSignals {
    int FetchEnable;
    int DecodeEnable;
    int ExecuteEnable;
    int MemoryEnable;
    int WriteBackEnable;
};

class StallUnit {
public:
    StallUnit() {}
    int checkStall(ID_EX_data& EX_command, uint32_t current_command_src_addr){
        if (EX_command.Memory_op != NO_MEMORY_OP and EX_command.rd_addr == current_command_src_addr){
            cout<<"STALL DETECTED"<<endl;
            return 1;
        }
        if (EX_command.Store_op != NO_STORE_OP and EX_command.rd_addr == current_command_src_addr) {
            cout<<"STALL DETECTED"<<endl;
            return 1;
        }
        else {
            return 0;
        }
    }

    int CheckLoadStoreStall(uint32_t addr1, uint32_t addr2) { //used for both LS and SL stall
        if (addr1 == addr2) {
            cout<<"LS STALL TRIGGERED"<<endl;
            return 1;
        }
        else {
            return 0;
        }
    }

    void SetStall(ProgramCounter& program_counter, Decoder& decoder, IF_ID_data& IF_ID_Register, int& stall_count) {
        if (decoder.insert_bubble == 0) {
            program_counter.enable = 0;
            decoder.insert_bubble = 1;
            IF_ID_Register.enable = 0;
            cout<<"DECODER INSRT BUBBLE "<<decoder.insert_bubble<<endl;
            stall_count += 1;
        }
    }

    void ExitStall(ProgramCounter& program_counter,Decoder& decoder, IF_ID_data& IF_ID_Register) {
        if (decoder.insert_bubble == 1) {
            program_counter.enable = 1;
            decoder.insert_bubble = 0;
            IF_ID_Register.enable = 1;
            cout<<"Stall END"<<endl;
            cout<<"!!!"<<endl;
        }
    }

    void SetStoreLoadStall(ProgramCounter& program_counter, Decoder& decoder, IF_ID_data& IF_ID_Register, ID_EX_data& ID_EX_Register, int& stall_count) {
        if (decoder.insert_bubble == 0) {
            program_counter.enable = 0;
            decoder.insert_bubble = 1;
            decoder.StoreLoad_bubble = 1;
            IF_ID_Register.enable = 0;
            ID_EX_Register.enable = 0;
            cout<<"S-L Stall begin "<<decoder.insert_bubble<<endl;
            stall_count += 1;
        }
    }

    void ExitStoreLoadStall(ProgramCounter& program_counter,Decoder& decoder, IF_ID_data& IF_ID_Register, ID_EX_data& ID_EX_Register) {
        if (decoder.insert_bubble == 1) {
            program_counter.enable = 1;
            decoder.insert_bubble = 0;
            decoder.StoreLoad_bubble = 0;
            IF_ID_Register.enable = 1;
            ID_EX_Register.enable = 1;
            cout<<"S-L Stall END"<<endl;
        }
    }

};
#endif //RISCV_CPU_STALLUNIT_H
