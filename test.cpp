//this is a test environment for each cpu core
#include "src/RegisterFile.h"
#include<iostream>
#include<string>
#include<bitset>
#include<array>
#include "src/CPUcore.h"
#include <vector>
#include"src/Probe.h"

using namespace std;
void dumpSelectedReg(CPUcore core, int reg1_index, int reg2_index, int reg3_index) {
    cout<<"Reg "<<reg1_index<<" :"<<core.registerFile.read(reg1_index)<<endl;
    cout<<"Reg "<<reg2_index<<" :"<<core.registerFile.read(reg2_index)<<endl;
    cout<<"Reg "<<reg3_index<<" :"<<core.registerFile.read(reg3_index)<<endl;
}

int RunCommands(CPUcore &core, L2Cache& l2cache, RAM &ram, int num_commands, int& total_cycles) {
    uint32_t commands[] = {
        // addi x1, x0, 1
        0b00000000000100000000000010010011,
        // slli x1, x1, 12 -> A 地址 = 4096
        0b00000000110000001001000010010011,

        // addi x2, x0, 5
        0b00000000010100000000000100010011,
        // slli x2, x2, 12 -> B 地址 = 20480
        0b00000000110000010001000100010011,

        // addi x9, x0, 9
        0b00000000100100000000010010010011,
        // slli x9, x9, 12 -> C 地址 = 36864
        0b00000000110001001001010010010011,

        // addi x3, x0, 42
        0b00000010101000000000000110010011,
        // addi x4, x0, 99
        0b00000110001100000000001000010011,
        // addi x10, x0, 123
        0b00000111101100000000010100010011,

        // sw x3, 0(x1) -> 写 A = 42
        0b00000000001100001010000000100011,

        // sw x4, 0(x2) -> 写 B = 99
        0b00000000010000010010000000100011,

        // lw x11, 0(x1) -> 读 A，使 B 成为 LRU
        0b00000000000000001010010110000011,

        // sw x10, 0(x9) -> 写 C，淘汰并写回 B
        0b00000000101001001010000000100011,

        // lw x12, 0(x1) -> A 应命中
        0b00000000000000001010011000000011,

        // lw x13, 0(x9) -> C 应命中
        0b00000000000001001010011010000011,
    };

    ram.loadCommands(commands, num_commands);

    for (int i=0;i<200;i++) {
        cout<<endl;
        cout<<"================================"<<endl;
        cout<<"->Cycle "<<i<<endl;
        core.Step(l2cache,ram);

        if (i != 0 and core.pipeline_registers_read.IF_ID_register.valid == 0
            and core.pipeline_registers_read.ID_EX_register.valid == 0
            and core.pipeline_registers_read.EX_MEM_register.valid == 0
            and core.pipeline_registers_read.MEM_WB_register.valid == 0) {
            total_cycles = i+1;
            break;
            }
    }
    core.registerFile.dumpRawValue();
    return total_cycles;
}

int main() {
    //component initialization
    Probe probes = Probe();
    RAM ram = RAM();
    L2Cache l2cache = L2Cache(ram);
    CPUcore core0 = CPUcore(0,59,probes);


    //Execution
    RunCommands(core0,l2cache,ram,15,probes.total_cycles);

    //Show statistics
    cout<<endl;
    cout<<"Statistics: "<<endl;
    cout<<"Total Cycles: "<<probes.total_cycles<<endl;
    if (core0.branch_predictor.enable == 1) {
        cout<<"Branch Predictor Status: On"<<endl;
    }
    else {
        cout<<"Branch Predictor Status: Off"<<endl;
    }
    if (core0.set_associative_on == 1) {
        cout<<"Cache Mode: Set associative"<<endl;
    }
    else {cout<<"Cache Mode: Direct mapping"<<endl;}
    cout<<"Stall Count: "<<probes.stall_count<<endl;

    //code below are for debug purposes only
    cout << unsigned(ram.readCell(4096))  << endl; // 0
    cout << unsigned(ram.readCell(20480)) << endl; // 99
    cout << unsigned(ram.readCell(36864)) << endl; // 0
}