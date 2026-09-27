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

        // addi x2, x0, 100
        0b00000110010000000000000100010011,

        // lw x3, 0(x2)
        0b00000000000000010010000110000011,

        // sw x3, 4(x2)
        0b00000000001100010010001000100011,
    };

    ram.loadCommands(commands, num_commands);

    for (int i=0;i<20;i++) {
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
    CPUcore core0 = CPUcore(0,11,probes);


    //Execution
    RunCommands(core0,l2cache,ram,3,probes.total_cycles);

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
    cout<<"Stall Count: "<<probes.stall_count<<endl;

}