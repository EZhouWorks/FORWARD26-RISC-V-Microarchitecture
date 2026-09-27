//64 mB RAM
//Each block = 64Byte
//mapping notation: cacheline = Block Number mod 256
#ifndef RISC_V_CPU_SIMULATOR_RAM_H
#define RISC_V_CPU_SIMULATOR_RAM_H
#include <cstdint>
#include<vector>
using namespace std;

class RAM {
public:
    vector<uint8_t> memory;

    RAM() { //initialize
        memory.resize(67108864);
        for (int i=0; i<memory.size(); i++) {
            memory[i] = 0;
        }
    }

    uint8_t readCell(uint32_t address) {
        return memory[address];
    }

    void writeCell(uint32_t address, uint8_t data) {
        memory[address] = data;
    }


    //functions below are fow debug only
    void loadCommands(const uint32_t commands[], uint32_t count, uint32_t startAddr = 0){
        uint32_t addr = startAddr;
        for(uint32_t i = 0; i < count; i++)
        {
            uint32_t cmd = commands[i];
            memory[addr++] = cmd & 0xff;
            memory[addr++] = (cmd >> 8) & 0xff;
            memory[addr++] = (cmd >> 16) & 0xff;
            memory[addr++] = (cmd >> 24) & 0xff;
        }
    }

};
#endif //RISC_V_CPU_SIMULATOR_RAM_H
