//
// Created by zhoue on 2026/7/17.
//
#ifndef RISC_V_CPU_SIMULATOR_BRANCHPREDICTOR_H
#define RISC_V_CPU_SIMULATOR_BRANCHPREDICTOR_H
#include <cstdint>
#include <array>
#include <stdexcept>
#include <bitset>
using namespace std;
class BranchPredictor{
public:
    uint8_t GlobalHistoryRegister = 0b00000000;
    array<uint8_t, 256> PatternHistoryTable;
    int enable;
    int total_branch_predictions;

    BranchPredictor(int enable, int& total_branch_predictions) {
        GlobalHistoryRegister = 0b00000000;
        for (int i = 0; i < 256; i++) {
            PatternHistoryTable[i] = 0b00000010; //initializes all default to strongly taken
            this->total_branch_predictions = total_branch_predictions;
        }
        this->enable = enable;
    }

    uint8_t PreTypeDecode(uint32_t opcode) {
        return (opcode & 0x7F) == 0b1100011;
    }

    uint32_t PreCommandAddrDecode(uint32_t opcode) {
        uint32_t raw_imm =
            (((opcode >> 31) & 0x1) << 12) |  // imm[12]
            (((opcode >> 7)  & 0x1) << 11) |  // imm[11]
            (((opcode >> 25) & 0x3F) << 5) |  // imm[10:5]
            (((opcode >> 8)  & 0x0F) << 1);   // imm[4:1]

        return (raw_imm & 0x1000)
            ? (raw_imm | 0xFFFFE000u)
            : raw_imm;
    }

    int predict(uint32_t command_addr) {
        total_branch_predictions += 1;
        int PHT_entry = static_cast<uint8_t>((command_addr >> 2) & 0xFF) ^ GlobalHistoryRegister;
        uint8_t PHT_value = PatternHistoryTable[PHT_entry];
        switch (PHT_value) {
            case(0b00000000): return 0; //strongly not taken
            case(0b00000001): return 0; //weakly not taken
            case(0b00000010): return 1; //weakly taken
            case(0b00000011): return 1; // strongly taken
            default: throw runtime_error("PHT has an unexpected BTB value");
        }
    }

    void updatePHT(uint32_t command_addr, int actual_taken) {
        int PHT_entry = static_cast<uint8_t>((command_addr >> 2) & 0xFF) ^ GlobalHistoryRegister;
        if (actual_taken == 1) {
            if (PatternHistoryTable[PHT_entry] == 0b00000011) {
                PatternHistoryTable[PHT_entry] = 0b00000011;
            }
            else {
                PatternHistoryTable[PHT_entry] += 0b00000001;
            }
        }
        else {
            if (PatternHistoryTable[PHT_entry] == 0b00000000) {
                PatternHistoryTable[PHT_entry] = 0b00000000;
            }
            else {
                PatternHistoryTable[PHT_entry] -= 0b00000001;
            }
        }
    }

    //functions below are for debug purposes only
    void DumpPHT(int PHT_entry=256) {
        if (PHT_entry == 256) {
            for (int i = 0; i < 256; i++) {
                cout<<"Entry: "<<i<<" ["<<bitset<8>(PatternHistoryTable[i])<<"]"<<endl;
            }
        }
        else {
            cout<<"Entry: "<<PHT_entry<<" ["<<bitset<8>(PatternHistoryTable[PHT_entry])<<"]"<<endl;
        }
    }

    void DumpGHR() {
        cout<<bitset<8>(GlobalHistoryRegister)<<endl;
    }
};




#endif //RISC_V_CPU_SIMULATOR_BRANCHPREDICTOR_H
