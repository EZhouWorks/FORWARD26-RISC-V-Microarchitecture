//L2 Cache is currently not implemented as a private cache! Missing L1 will not result in visiting L2
//L2 Cache does not have dirty data
//256kB L2 Cache
#ifndef RISC_V_CPU_SIMULATOR_L2CACHE_H
#define RISC_V_CPU_SIMULATOR_L2CACHE_H
#include <cstdint>
#include "RAM.h"
constexpr int BLOCK_SIZE_L2 = 64;
constexpr int CACHE_LINES_L2 = 4096;

struct CacheLine {
    int dirty = 0; //indicates if this line is the same as corresponding data block in RAM
    int valid = 0;
    int tag = 0;
    int set = 0;
    int LRU = 0; // 0: recently used;  1: swap next time
    int set_associative_line = 0;
    uint8_t bytes[64]; //each cacheline stores 16 commands， 64byte storage,
};

class L2Cache {
public:
    CacheLine cachelines[CACHE_LINES_L2];
    L2Cache(RAM &ram) { //initialize
        for (int i = 0; i < CACHE_LINES_L2; i++) {
            cachelines[i].valid = 0;
            cachelines[i].tag = 0;
            cachelines[i].dirty = 0;
            for (int j = 0; j < BLOCK_SIZE_L2; j++) {
                cachelines[i].bytes[j] = 0;
            }
        }
    }

    CacheLine loadCacheBlockFromRAM(uint32_t addr,RAM& ram) {
        uint32_t block = addr/BLOCK_SIZE_L2;
        uint32_t index = block%CACHE_LINES_L2;
        uint32_t tag = block/CACHE_LINES_L2;
        uint32_t offset = addr%64;

        uint32_t blockStartAddr = block*BLOCK_SIZE_L2;
        uint32_t blockEndAddr = blockStartAddr+BLOCK_SIZE_L2-1;

        for (int i = blockStartAddr; i <= blockEndAddr; i++) { //load RAM block into cache line
            cachelines[index].bytes[i-blockStartAddr] = ram.readCell(i);
        }

        cachelines[index].valid = 1;
        cachelines[index].tag = tag;
        cachelines[index].dirty = 0;

        return cachelines[index];

    }
    CacheLine passL2DataToL1(CacheLine cacheLine) {
        return cacheLine;
    }

    CacheLine readFullData(uint32_t addr,RAM& ram) { //this returns the whole cacheline
        uint32_t block = addr/BLOCK_SIZE_L2;
        uint32_t index = block%CACHE_LINES_L2;
        uint32_t tag = block/CACHE_LINES_L2;
        uint32_t offset = addr%BLOCK_SIZE_L2;

        CacheLine& target_cacheLine = cachelines[index];
        if (target_cacheLine.valid == 1 and target_cacheLine.tag == tag) { //hit
            //cout<<"hit L2"<<endl;

            return target_cacheLine;
        }
        else { //miss, manually read from RAM
            //cout<<"miss L2"<<endl;
            return loadCacheBlockFromRAM(addr,ram);
        }
    }

    void replaceCacheLine(CacheLine cacheLine, uint32_t addr) {
        uint32_t block = addr/BLOCK_SIZE_L2;
        uint32_t index = block%CACHE_LINES_L2;
        uint32_t tag = block/CACHE_LINES_L2;
        cachelines[index] = cacheLine;
        cachelines[index].tag = tag;
        cachelines[index].valid = 1;
    }

};
#endif //RISC_V_CPU_SIMULATOR_L2CACHE_H
