//
// Created by zhoue on 2026/7/17.
//

#ifndef RISC_V_CPU_SIMULATOR_CONTROLLER_H
#define RISC_V_CPU_SIMULATOR_CONTROLLER_H
#include <cstdint>
#include "ALU.h"
#include "BranchUnit.h"
#include "JumpUnit.h"
#include "L1Cache.h"

class Controller {
public:
    ALU_op ALU_operation;
    ALU_source ALU_source1;
    ALU_source ALU_source2;
    Memory_op Memory_op;
    Memory_data_type Memory_data_type;
    Store_op Store_op;
    uint32_t Register_operation = 0;
    uint32_t Memory_operation = 0;
    RegFile_op RegFile_op;
    Branch_op Branch_op;
    Jump_op Jump_op;
    
    Controller(){};
    void SetControlSignal(uint32_t decoder_code) {
        //initialization
        ALU_operation = NO_ALU_OP;
        ALU_source1 = rs1;
        ALU_source2 = rs2;
        Memory_op = NO_MEMORY_OP;
        Memory_data_type = UNSIGN;
        Store_op = NO_STORE_OP;
        RegFile_op = NO_RegFile_OP;
        Branch_op = NO_BRANCH_OP;
        Jump_op = NO_JUMP_OP;
        switch (decoder_code) { //remember to clean the code
            //R type
            case(0): this->ALU_operation = NO_ALU_OP; ALU_source1 = rs1; ALU_source2 = rs2; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(1): this->ALU_operation = ADD_op; ALU_source1 = rs1; ALU_source2 = rs2; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(2): this->ALU_operation = SUB_op; ALU_source1 = rs1; ALU_source2 = rs2; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(3): this->ALU_operation = SLL_op; ALU_source1 = rs1; ALU_source2 = rs2; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(4): this->ALU_operation = SLT_op; ALU_source1 = rs1; ALU_source2 = rs2; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(5): this->ALU_operation = SLTU_op; ALU_source1 = rs1; ALU_source2 = rs2; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(6): this->ALU_operation = XOR_op; ALU_source1 = rs1; ALU_source2 = rs2; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(7): this->ALU_operation = SRL_op; ALU_source1 = rs1; ALU_source2 = rs2; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(8): this->ALU_operation = SRA_op; ALU_source1 = rs1; ALU_source2 = rs2; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(9): this->ALU_operation = OR_op; ALU_source1 = rs1; ALU_source2 = rs2; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(10): this->ALU_operation = AND_op; ALU_source1 = rs1; ALU_source2 = rs2; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            //I type ALU
            case(11): this->ALU_operation = ADD_op; ALU_source1 = rs1; ALU_source2 = I_12bit_imm; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(12): this->ALU_operation = SLT_op; ALU_source1 = rs1; ALU_source2 = I_12bit_imm; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(13): this->ALU_operation = SLTU_op; ALU_source1 = rs1; ALU_source2 = I_12bit_imm; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(14): this->ALU_operation = XOR_op; ALU_source1 = rs1; ALU_source2 = I_12bit_imm; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(15): this->ALU_operation = SLL_op; ALU_source1 = rs1; ALU_source2 = I_shamt_imm; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(16): this->ALU_operation = SRL_op; ALU_source1 = rs1; ALU_source2 = I_shamt_imm; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(17): this->ALU_operation = SRA_op; ALU_source1 = rs1; ALU_source2 = I_shamt_imm; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(18): this->ALU_operation = OR_op; ALU_source1 = rs1; ALU_source2 = I_12bit_imm; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            case(19): this->ALU_operation = AND_op; ALU_source1 = rs1; ALU_source2 = I_12bit_imm; Memory_op = NO_MEMORY_OP; Store_op = NO_STORE_OP; RegFile_op = SAVE_ALU_RESULT; break;
            //Load
            case(20): this->ALU_operation = ADD_op; Memory_op = READBYTE; ALU_source1 = rs1; ALU_source2 = I_12bit_imm; Memory_data_type = SIGN; Store_op = NO_STORE_OP; RegFile_op = SAVE_DATA; break;
            case(21): this->ALU_operation = ADD_op; Memory_op = READHALF; ALU_source1 = rs1; ALU_source2 = I_12bit_imm; Memory_data_type = SIGN; Store_op = NO_STORE_OP; RegFile_op = SAVE_DATA; break;
            case(22): this->ALU_operation = ADD_op; Memory_op = READWORD; ALU_source1 = rs1; ALU_source2 = I_12bit_imm; Memory_data_type = UNSIGN; Store_op = NO_STORE_OP; RegFile_op = SAVE_DATA; break;
            case(23): this->ALU_operation = ADD_op; Memory_op = READBYTE; ALU_source1 = rs1; ALU_source2 = I_12bit_imm; Memory_data_type = UNSIGN; Store_op = NO_STORE_OP; RegFile_op = SAVE_DATA; break;
            case(24): this->ALU_operation = ADD_op; Memory_op = READHALF; ALU_source1 = rs1; ALU_source2 = I_12bit_imm; Memory_data_type = UNSIGN; Store_op = NO_STORE_OP; RegFile_op = SAVE_DATA; break;
            //Store
            case(25): this->ALU_operation = ADD_op; Store_op = STOREBYTE; ALU_source1 = rs1; ALU_source2 = store_imm; Memory_op = NO_MEMORY_OP; RegFile_op = NO_RegFile_OP; break;
            case(26): this->ALU_operation = ADD_op; Store_op = STOREHALF; ALU_source1 = rs1; ALU_source2 = store_imm; Memory_op = NO_MEMORY_OP; RegFile_op = NO_RegFile_OP; break;
            case(27): this->ALU_operation = ADD_op; Store_op = STOREWORD; ALU_source1 = rs1; ALU_source2 = store_imm; Memory_op = NO_MEMORY_OP; RegFile_op = NO_RegFile_OP; break;
            //B type
            case(28):this->Branch_op = BEQ; ALU_source1 = rs1; ALU_source2 = rs2; break;
            case(29):this->Branch_op = BNE; ALU_source1 = rs1; ALU_source2 = rs2; break;
            case(30):this->Branch_op = BLT; ALU_source1 = rs1; ALU_source2 = rs2; break;
            case(31):this->Branch_op = BGE; ALU_source1 = rs1; ALU_source2 = rs2; break;
            case(32):this->Branch_op = BLTU; ALU_source1 = rs1; ALU_source2 = rs2; break;
            case(33):this->Branch_op = BGEU; ALU_source1 = rs1; ALU_source2 = rs2; break;

            //J type
            case(36):this->Jump_op = JAL; ALU_operation = ADD_op; ALU_source1 = J_imm; ALU_source2 = command_PC_value; RegFile_op = SAVE_PC_VAL;break;
            case(37):this->Jump_op = JALR; ALU_operation = ADD_op; ALU_source1 = rs1; ALU_source2 = I_12bit_imm; RegFile_op = SAVE_PC_VAL;break;
        }
    }
};
#endif //RISC_V_CPU_SIMULATOR_CONTROLLER_H
