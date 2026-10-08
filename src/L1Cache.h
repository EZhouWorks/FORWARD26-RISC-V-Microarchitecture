//16kb L1 cache
//256 cache lines
#ifndef RISC_V_CPU_SIMULATOR_L1CACHE_H
#define RISC_V_CPU_SIMULATOR_L1CACHE_H
#include<cstdint>

#include "CPUcore.h"
#include "RAM.h"
#include "L2Cache.h"
constexpr int L1_SIZE = 16*1024;
constexpr int BLOCK_SIZE = 64;
constexpr int CACHE_LINES = L1_SIZE/BLOCK_SIZE;
constexpr int SET_ASSOIATIVE_CACHE_LINES = CACHE_LINES / 2;

enum Memory_op {
    READBYTE,
    READHALF,
    READWORD,
    WRITE,
    NO_MEMORY_OP
};
enum Memory_data_type {
    SIGN,
    UNSIGN
};
enum Store_op {
    STOREBYTE,
    STOREHALF,
    STOREWORD,
    NO_STORE_OP

};

class L1Cache {
public:
    CacheLine cachelines[CACHE_LINES];
    L1Cache(int set_associative_mode) { //initialize
        if (set_associative_mode == 0) { //direct mapping
            for (int i = 0; i < CACHE_LINES; i++) {
                cachelines[i].valid = 0;
                cachelines[i].tag = 0;
                cachelines[i].dirty = 0;
                for (int j = 0; j < BLOCK_SIZE; j++) {
                    cachelines[i].bytes[j] = 0;
                }
            }
        }
        else { //set associative
            for (int i = 0; i<CACHE_LINES; i += 2) {
                cachelines[i].valid = 0;
                cachelines[i+1].valid = 0;
                cachelines[i].tag = 0;
                cachelines[i+1].tag=0;
                cachelines[i].set = 0;
                cachelines[i+1].set = 1;
                cachelines[i].LRU = 0;
                cachelines[i+1].LRU = 0;
                cachelines[i].set_associative_line = i;
                cachelines[i+1].set_associative_line = i;
                cachelines[i].dirty = 0;
                cachelines[i+1].dirty = 0;
                for (int j = 0; j < BLOCK_SIZE; j++) {
                    cachelines[i].bytes[j] = 0;
                    cachelines[i+1].bytes[j] = 0;
                }
            }
        }
    }

    CacheLine LoadCacheBlockFromRAM(uint32_t addr, RAM& ram, int set_associative_mode) {
        if (set_associative_mode == 0) { // direct mapping
            uint32_t block = addr/BLOCK_SIZE;
            uint32_t index = block%CACHE_LINES;
            uint32_t tag = block/CACHE_LINES;

            uint32_t blockStartAddr = block*BLOCK_SIZE;
            uint32_t blockEndAddr = blockStartAddr+BLOCK_SIZE-1;

            for (int i = blockStartAddr; i <= blockEndAddr; i++) { //load RAM block into cache line
                cachelines[index].bytes[i-blockStartAddr] = ram.readCell(i);
            }

            cachelines[index].valid = 1;
            cachelines[index].tag = tag;
            cachelines[index].dirty = 0;

            return cachelines[index];
        }
        else {
            uint32_t block = addr/BLOCK_SIZE;
            uint32_t index = block%SET_ASSOIATIVE_CACHE_LINES;
            uint32_t tag = block/SET_ASSOIATIVE_CACHE_LINES;

            uint32_t blockStartAddr = block*BLOCK_SIZE;
            uint32_t blockEndAddr = blockStartAddr+BLOCK_SIZE-1;

            CacheLine write_ready_set{};
            write_ready_set.valid = 1;
            write_ready_set.tag = tag;
            write_ready_set.dirty = 0;

            for (int i = blockStartAddr; i <= blockEndAddr; i++) { //load RAM block into cache line
                write_ready_set.bytes[i-blockStartAddr] = ram.readCell(i);
            }

            return write_ready_set;
        }

    }

    uint32_t readWord(uint32_t addr,L2Cache& l2cache,RAM& ram, int set_associative_on, int& data_cache_hit_count, int& fetch_cache_hit_count, int fetch_visit) { //loads data from ram if missed, can also be used to access data in L1Cache
        if (set_associative_on == 0) {
            uint32_t block = addr/64;
            uint32_t index = block%CACHE_LINES;
            uint32_t tag = block/CACHE_LINES;
            uint32_t offset = addr%64;

            //check if data addr is in two separate lines
            if (offset>60) {
                throw runtime_error("Access data in sepate cachelines");
            }

            CacheLine& target_cacheLine = cachelines[index];
            if (target_cacheLine.valid == 1 and target_cacheLine.tag == tag) { //hit
                if (fetch_visit != 1) {
                    data_cache_hit_count += 1;
                }
                else {
                    fetch_cache_hit_count += 1;
                }
                return
                      (uint32_t)target_cacheLine.bytes[offset]
                    | ((uint32_t)target_cacheLine.bytes[offset+1] << 8)
                    | ((uint32_t)target_cacheLine.bytes[offset+2] << 16)
                    | ((uint32_t)target_cacheLine.bytes[offset+3] << 24);
            }
            else { //miss, read and load from ram
                if (target_cacheLine.dirty == 1 and target_cacheLine.valid == 1) {
                    //save previous dirty data to ram
                    uint32_t blockStartAddr = (target_cacheLine.tag * CACHE_LINES + index) * BLOCK_SIZE;
                    for (int i=0;i<64;i++) {
                        ram.memory[blockStartAddr+i] = target_cacheLine.bytes[i];
                    }
                }
                target_cacheLine = LoadCacheBlockFromRAM(addr, ram, set_associative_on);
                return
                      (uint32_t)target_cacheLine.bytes[offset]
                    | ((uint32_t)target_cacheLine.bytes[offset+1] << 8)
                    | ((uint32_t)target_cacheLine.bytes[offset+2] << 16)
                    | ((uint32_t)target_cacheLine.bytes[offset+3] << 24);
            }
        }
        else {   //set associative
            uint32_t block = addr/64;
            uint32_t index = block%SET_ASSOIATIVE_CACHE_LINES;
            uint32_t tag = block/SET_ASSOIATIVE_CACHE_LINES;
            uint32_t offset = addr%64;

            //check if data addr is in two separate lines
            if (offset>60) {
                throw runtime_error("Access data in separate cachelines");
            }

            CacheLine& set1 = cachelines[index*2];
            CacheLine& set2 = cachelines[index*2+1];
            if (set1.valid == 1 and set1.tag == tag) {
                //hit set 1
                set1.LRU = 0;
                set2.LRU = 1;
                return
                      (uint32_t)set1.bytes[offset]
                    | ((uint32_t)set1.bytes[offset+1] << 8)
                    | ((uint32_t)set1.bytes[offset+2] << 16)
                    | ((uint32_t)set1.bytes[offset+3] << 24);
            }
            else if (set2.valid == 1 and set2.tag == tag) {
                //hit set 2
                set1.LRU = 1;
                set2.LRU = 0;
                return
                      (uint32_t)set2.bytes[offset]
                    | ((uint32_t)set2.bytes[offset+1] << 8)
                    | ((uint32_t)set2.bytes[offset+2] << 16)
                    | ((uint32_t)set2.bytes[offset+3] << 24);
            }
            else {
                //miss
                if (set1.valid == 0) {
                    set1 = LoadCacheBlockFromRAM(addr, ram, set_associative_on);
                    set1.LRU = 0;
                    set2.LRU = 1;
                    return
                      (uint32_t)set1.bytes[offset]
                    | ((uint32_t)set1.bytes[offset+1] << 8)
                    | ((uint32_t)set1.bytes[offset+2] << 16)
                    | ((uint32_t)set1.bytes[offset+3] << 24);
                }
                else if (set2.valid == 0) {
                    set2 = LoadCacheBlockFromRAM(addr, ram, set_associative_on);
                    set1.LRU = 1;
                    set2.LRU = 0;
                    return
                      (uint32_t)set2.bytes[offset]
                    | ((uint32_t)set2.bytes[offset+1] << 8)
                    | ((uint32_t)set2.bytes[offset+2] << 16)
                    | ((uint32_t)set2.bytes[offset+3] << 24);
                }
                else if (set1.LRU == 1) {
                    //save dirty data to RAM
                    if (set1.valid == 1 and set1.dirty == 1) {
                        uint32_t blockStartAddr = (set1.tag * SET_ASSOIATIVE_CACHE_LINES + index) * BLOCK_SIZE;
                        for (int i=0;i<64;i++) {
                            ram.memory[blockStartAddr+i] = set1.bytes[i];
                        }
                    }
                    //grab data from RAM
                    set1 = LoadCacheBlockFromRAM(addr, ram, set_associative_on);
                    set1.LRU = 0;
                    set2.LRU = 1;
                    return
                      (uint32_t)set1.bytes[offset]
                    | ((uint32_t)set1.bytes[offset+1] << 8)
                    | ((uint32_t)set1.bytes[offset+2] << 16)
                    | ((uint32_t)set1.bytes[offset+3] << 24);

                }
                else if (set2.LRU == 1) {
                    //save dirty data to RAM
                    if (set2.valid == 1 and set2.dirty == 1) {
                        uint32_t blockStartAddr = (set2.tag * SET_ASSOIATIVE_CACHE_LINES + index) * BLOCK_SIZE;
                        for (int i=0;i<64;i++) {
                            ram.memory[blockStartAddr+i] = set2.bytes[i];
                        }
                    }
                    //grab data from RAM
                    set2 = LoadCacheBlockFromRAM(addr, ram, set_associative_on);
                    set2.LRU = 0;
                    set1.LRU = 1;
                    return
                      (uint32_t)set2.bytes[offset]
                    | ((uint32_t)set2.bytes[offset+1] << 8)
                    | ((uint32_t)set2.bytes[offset+2] << 16)
                    | ((uint32_t)set2.bytes[offset+3] << 24);
                }
            }

        }
    }

    uint16_t readHalfWord(uint32_t addr,L2Cache& l2cache,RAM& ram, int set_associative_on, int& data_cache_hit_count, int& fetch_cache_hit_count, int fetch_visit) { //loads data from ram if missed, can also be used to access data in L1Cache
        if (set_associative_on == 0) { //direct mapping
            uint32_t block = addr/64;
            uint32_t index = block%CACHE_LINES;
            uint32_t tag = block/CACHE_LINES;
            uint32_t offset = addr%64;

            if (offset > 62) {
                throw runtime_error("Access data in separate cachelines");
            }

            CacheLine& target_cacheLine = cachelines[index];
            if (target_cacheLine.valid == 1 and target_cacheLine.tag == tag) { //hit
                if (fetch_visit != 1) {
                    data_cache_hit_count += 1;
                }
                else {
                    fetch_cache_hit_count += 1;
                }
                return
                      (uint32_t)target_cacheLine.bytes[offset]
                    | ((uint32_t)target_cacheLine.bytes[offset+1] << 8);
            }
            else { //miss, read and load from RAM
                cout<<"miss L1"<<endl;
                if (target_cacheLine.dirty == 1 and target_cacheLine.valid == 1) {
                    //save previous dirty data to ram
                    uint32_t blockStartAddr = (target_cacheLine.tag * CACHE_LINES + index) * BLOCK_SIZE;
                    for (int i=0;i<64;i++) {
                        ram.memory[blockStartAddr+i] = target_cacheLine.bytes[i];
                    }
                }
                target_cacheLine = LoadCacheBlockFromRAM(addr,ram,set_associative_on);
                return
                      (uint32_t)target_cacheLine.bytes[offset]
                    | ((uint32_t)target_cacheLine.bytes[offset+1] << 8);
            }
        }
        else { //set associative

            uint32_t block = addr/64;
            uint32_t index = block%SET_ASSOIATIVE_CACHE_LINES;
            uint32_t tag = block/SET_ASSOIATIVE_CACHE_LINES;
            uint32_t offset = addr%64;

            //check if data addr is in two separate lines
            if (offset>62) {
                throw runtime_error("Access data in separate cachelines");
            }

            CacheLine& set1 = cachelines[index*2];
            CacheLine& set2 = cachelines[index*2+1];
            if (set1.valid == 1 and set1.tag == tag) {
                //hit set 1
                set1.LRU = 0;
                set2.LRU = 1;
                return
                      (uint32_t)set1.bytes[offset]
                    | ((uint32_t)set1.bytes[offset+1] << 8);
            }
            else if (set2.valid == 1 and set2.tag == tag) {
                //hit set 2
                set1.LRU = 1;
                set2.LRU = 0;
                return
                      (uint32_t)set2.bytes[offset]
                    | ((uint32_t)set2.bytes[offset+1] << 8);
            }
            else {
                //miss
                if (set1.valid == 0) {
                    set1 = LoadCacheBlockFromRAM(addr, ram, set_associative_on);
                    set1.LRU = 0;
                    set2.LRU = 1;
                    return
                      (uint32_t)set1.bytes[offset]
                    | ((uint32_t)set1.bytes[offset+1] << 8);
                }
                else if (set2.valid == 0) {
                    set2 = LoadCacheBlockFromRAM(addr, ram, set_associative_on);
                    set1.LRU = 1;
                    set2.LRU = 0;
                    return
                      (uint32_t)set2.bytes[offset]
                    | ((uint32_t)set2.bytes[offset+1] << 8);
                }
                else if (set1.LRU == 1) {
                    //save dirty data to RAM
                    if (set1.valid == 1 and set1.dirty == 1) {
                        uint32_t blockStartAddr = (set1.tag * SET_ASSOIATIVE_CACHE_LINES + index) * BLOCK_SIZE;
                        for (int i=0;i<64;i++) {
                            ram.memory[blockStartAddr+i] = set1.bytes[i];
                        }
                    }
                    //grab data from RAM
                    set1 = LoadCacheBlockFromRAM(addr, ram, set_associative_on);
                    set1.LRU = 0;
                    set2.LRU = 1;
                    return
                      (uint32_t)set1.bytes[offset]
                    | ((uint32_t)set1.bytes[offset+1] << 8);

                }
                else if (set2.LRU == 1) {
                    //save dirty data to RAM
                    if (set2.valid == 1 and set2.dirty == 1) {
                        uint32_t blockStartAddr = (set2.tag * SET_ASSOIATIVE_CACHE_LINES + index) * BLOCK_SIZE;
                        for (int i=0;i<64;i++) {
                            ram.memory[blockStartAddr+i] = set2.bytes[i];
                        }
                    }
                    //grab data from RAM
                    set2 = LoadCacheBlockFromRAM(addr, ram, set_associative_on);
                    set2.LRU = 0;
                    set1.LRU = 1;
                    return
                      (uint32_t)set2.bytes[offset]
                    | ((uint32_t)set2.bytes[offset+1] << 8);
                }
            }
        }
    }

    uint8_t readByte(uint32_t addr,L2Cache& l2cache,RAM& ram, int set_associative_on, int& data_cache_hit_count, int& fetch_cache_hit_count, int fetch_visit) {
        //loads data from ram if missed, can also be used to access data in L1Cache
        if (set_associative_on == 0) {
            uint32_t block = addr/64;
            uint32_t index = block%CACHE_LINES;
            uint32_t tag = block/CACHE_LINES;
            uint32_t offset = addr%64;

            CacheLine& target_cacheLine = cachelines[index];
            if (target_cacheLine.valid == 1 and target_cacheLine.tag == tag) { //hit
                if (fetch_visit != 1) {
                    data_cache_hit_count += 1;
                }
                else{
                    fetch_cache_hit_count += 1;
                }
                //cout<<"hit L1"<<endl;
                return
                      (uint32_t)target_cacheLine.bytes[offset];
            }
            else { //miss, read and load from L2 Cache
                //cout<<"miss L1"<<endl;
                if (target_cacheLine.dirty == 1 and target_cacheLine.valid == 1) {
                    //save previous dirty data to ram
                    uint32_t blockStartAddr = (target_cacheLine.tag * CACHE_LINES + index) * BLOCK_SIZE;
                    for (int i=0;i<64;i++) {
                        ram.memory[blockStartAddr+i] = target_cacheLine.bytes[i];
                    }
                }
                target_cacheLine = LoadCacheBlockFromRAM(addr,ram,set_associative_on);
                return
                      (uint32_t)target_cacheLine.bytes[offset];
            }
        }
        else { //set associative
            uint32_t block = addr/64;
            uint32_t index = block%SET_ASSOIATIVE_CACHE_LINES;
            uint32_t tag = block/SET_ASSOIATIVE_CACHE_LINES;
            uint32_t offset = addr%64;

            CacheLine& set1 = cachelines[index*2];
            CacheLine& set2 = cachelines[index*2+1];
            if (set1.valid == 1 and set1.tag == tag) {
                //hit set 1
                set1.LRU = 0;
                set2.LRU = 1;
                return
                      (uint32_t)set1.bytes[offset];
            }
            else if (set2.valid == 1 and set2.tag == tag) {
                //hit set 2
                set1.LRU = 1;
                set2.LRU = 0;
                return
                      (uint32_t)set2.bytes[offset];
            }
            else {
                //miss
                if (set1.valid == 0) {
                    set1 = LoadCacheBlockFromRAM(addr, ram, set_associative_on);
                    set1.LRU = 0;
                    set2.LRU = 1;
                    return
                      (uint32_t)set1.bytes[offset];
                }
                else if (set2.valid == 0) {
                    set2 = LoadCacheBlockFromRAM(addr, ram, set_associative_on);
                    set1.LRU = 1;
                    set2.LRU = 0;
                    return
                      (uint32_t)set2.bytes[offset];
                }
                else if (set1.LRU == 1) {
                    //save dirty data to RAM
                    if (set1.valid == 1 and set1.dirty == 1) {
                        uint32_t blockStartAddr = (set1.tag * SET_ASSOIATIVE_CACHE_LINES + index) * BLOCK_SIZE;
                        for (int i=0;i<64;i++) {
                            ram.memory[blockStartAddr+i] = set1.bytes[i];
                        }
                    }
                    //grab data from RAM
                    set1 = LoadCacheBlockFromRAM(addr, ram, set_associative_on);
                    set1.LRU = 0;
                    set2.LRU = 1;
                    return
                      (uint32_t)set1.bytes[offset];

                }
                else if (set2.LRU == 1) {
                    //save dirty data to RAM
                    if (set2.valid == 1 and set2.dirty == 1) {
                        uint32_t blockStartAddr = (set2.tag * SET_ASSOIATIVE_CACHE_LINES + index) * BLOCK_SIZE;
                        for (int i=0;i<64;i++) {
                            ram.memory[blockStartAddr+i] = set2.bytes[i];
                        }
                    }
                    //grab data from RAM
                    set2 = LoadCacheBlockFromRAM(addr, ram, set_associative_on);
                    set2.LRU = 0;
                    set1.LRU = 1;
                    return
                      (uint32_t)set2.bytes[offset];
                }

            }
        }
    }

    //fucntions below all relate to STORE command
    // int checkL2Hit(uint32_t addr,L2Cache& l2cache,RAM& ram) { //check if store command hit L2 cache
    //     uint32_t block = addr/BLOCK_SIZE_L2;
    //     uint32_t index = block%CACHE_LINES_L2;
    //     uint32_t tag = block/CACHE_LINES_L2;
    //     CacheLine& target_cacheLine = l2cache.cachelines[index];
    //     if (target_cacheLine.valid == 1 and target_cacheLine.tag == tag) {
    //         return 1;
    //     }
    //     else {
    //         return 0;
    //     }
    // }

    void Store(Store_op Store_op, uint32_t addr, uint32_t data,L2Cache& l2cache,RAM& ram, int set_associative_on) {
        if (set_associative_on == 0) {
            //direct mapping
            uint32_t block = addr/64;
            uint32_t index = block%CACHE_LINES;
            uint32_t tag = block/CACHE_LINES;
            uint32_t offset = addr%64;
            uint32_t L2index = block%4096;
            CacheLine& target_cacheLine = cachelines[index];
            switch (Store_op) {
                case(Store_op::STOREBYTE):{
                    if (target_cacheLine.valid == 1 and target_cacheLine.tag == tag) { //check if L1 hit
                        target_cacheLine.bytes[offset] = data; //update new data
                    }
                    else { //miss, save dirty cacheline, load new cacheline
                        if (target_cacheLine.dirty == 1 and target_cacheLine.valid == 1) { //save previous dirty data to ram
                            uint32_t blockStartAddr = (target_cacheLine.tag * CACHE_LINES + index) * BLOCK_SIZE;
                            for (int i=0;i<64;i++) {
                                ram.memory[blockStartAddr+i] = target_cacheLine.bytes[i];
                            }
                        }
                        cachelines[index] = LoadCacheBlockFromRAM(addr,ram, set_associative_on);
                        cachelines[index].bytes[offset] = uint8_t(data);
                        cachelines[index].valid = 1;
                        cachelines[index].tag = tag;
                    }
                    cachelines[index].dirty = 1;
                    break;
                }
                case(Store_op::STOREHALF): {
                    if (offset > 62) {
                        throw runtime_error("Store block in separate cachelines");
                    }
                    uint8_t lowByte  = uint16_t(data) & 0xFF;
                    uint8_t highByte = (uint16_t(data) >> 8) & 0xFF;
                    if (target_cacheLine.valid == 1 and target_cacheLine.tag == tag) { //check if L1 hit
                        target_cacheLine.bytes[offset] = lowByte;
                        target_cacheLine.bytes[offset+1] = highByte;
                    }
                    else { //miss, save dirty cacheline, load new cacheline
                        if (target_cacheLine.dirty == 1 and target_cacheLine.valid == 1) { //save previous dirty data to ram
                            uint32_t blockStartAddr = (target_cacheLine.tag * CACHE_LINES + index) * BLOCK_SIZE;
                            for (int i=0;i<64;i++) {
                                ram.memory[blockStartAddr+i] = target_cacheLine.bytes[i];
                            }
                        }
                        cachelines[index] = LoadCacheBlockFromRAM(addr,ram,set_associative_on);
                        cachelines[index].bytes[offset] = lowByte;
                        cachelines[index].bytes[offset+1] = highByte;
                        cachelines[index].valid = 1;
                        cachelines[index].tag = tag;
                    }
                    cachelines[index].dirty = 1;
                    break;
                }
                case(Store_op::STOREWORD): {
                    if (offset > 60) {
                        throw runtime_error("Store block in separate cachelines");
                    }
                    uint8_t byte0 =  data & 0xFF;
                    uint8_t byte1 = (data >> 8)  & 0xFF;
                    uint8_t byte2 = (data >> 16) & 0xFF;
                    uint8_t byte3 = (data >> 24) & 0xFF;
                    if (target_cacheLine.valid == 1 and target_cacheLine.tag == tag) { //check if L1 hit
                        target_cacheLine.bytes[offset] = byte0;
                        target_cacheLine.bytes[offset+1] = byte1;
                        target_cacheLine.bytes[offset+2] = byte2;
                        target_cacheLine.bytes[offset+3] = byte3;
                    }
                    else { //miss, save dirty cacheline, load new cacheline
                        if (target_cacheLine.dirty == 1 and target_cacheLine.valid == 1) { //save previous dirty data to ram
                            uint32_t blockStartAddr = (target_cacheLine.tag * CACHE_LINES + index) * BLOCK_SIZE;
                            for (int i=0;i<64;i++) {
                                ram.memory[blockStartAddr+i] = target_cacheLine.bytes[i];
                            }
                        }
                        cachelines[index] = LoadCacheBlockFromRAM(addr,ram, set_associative_on);
                        target_cacheLine.bytes[offset] = byte0;
                        target_cacheLine.bytes[offset+1] = byte1;
                        target_cacheLine.bytes[offset+2] = byte2;
                        target_cacheLine.bytes[offset+3] = byte3;
                        cachelines[index].valid = 1;
                        cachelines[index].tag = tag;
                    }
                    cachelines[index].dirty = 1;
                    break;
                }
                case(Store_op::NO_STORE_OP):break;
                default: throw runtime_error("Unknown Memory Data Operation");
            }
        }
        else { //set associative
            uint32_t block = addr/64;
            uint32_t index = block%SET_ASSOIATIVE_CACHE_LINES;
            uint32_t tag = block/SET_ASSOIATIVE_CACHE_LINES;
            uint32_t offset = addr%64;
            
            CacheLine& set1 = cachelines[index*2];
            CacheLine& set2 = cachelines[index*2+1];
            
            switch (Store_op) {
                case(Store_op::STOREBYTE): {
                    //check set1 hit
                    if (set1.valid == 1 and set1.tag == tag) {
                        //hit set 1
                        set1.bytes[offset] = data;
                        set1.dirty = 1;
                        set1.LRU = 0;
                        set2.LRU = 1;
                        break;
                    }
                    //check set2 hit
                    else if (set2.valid == 1 and set2.tag == tag) {
                        //hit set 2
                        set2.bytes[offset] = data;
                        set2.dirty = 1;
                        set2.LRU = 0;
                        set1.LRU = 1;
                        break;
                    }
                    //set1 empty
                    else if (set1.valid == 0) {
                        set1 = LoadCacheBlockFromRAM(addr,ram,set_associative_on);
                        set1.bytes[offset] = data;
                        set1.dirty = 1;
                        set1.valid = 1;
                        set1.tag = tag;
                        set1.LRU = 0;
                        set2.LRU = 1;
                        break;
                    }
                    //set2 empty
                    else if (set2.valid == 0) {
                        set2 = LoadCacheBlockFromRAM(addr,ram, set_associative_on);
                        set2.bytes[offset] = data;
                        set2.dirty = 1;
                        set2.valid = 1;
                        set2.tag = tag;
                        set2.LRU = 0;
                        set1.LRU = 1;
                        break;
                    }
                    //Swap set1
                    //store dirty data
                    else if (set1.valid == 1 and set1.dirty == 1 and set1.LRU == 1) {
                        uint32_t blockStartAddr = (set1.tag * SET_ASSOIATIVE_CACHE_LINES + index) * BLOCK_SIZE;
                        for (int i=0;i<64;i++) {
                            ram.memory[blockStartAddr+i] = set1.bytes[i];
                        }
                        //Load corresponding cacheline from RAM and edit value
                        set1 = LoadCacheBlockFromRAM(addr,ram, set_associative_on);
                        set1.bytes[offset] = uint8_t(data);
                        set1.valid = 1;
                        set1.tag = tag;
                        set1.dirty = 1;
                        set1.LRU = 0;
                        set2.LRU = 1;
                        break;
                    }
                    //swap set 2
                    else if (set2.valid == 1 and set2.dirty == 1 and set2.LRU == 1) {
                        //store dirty data
                        uint32_t blockStartAddr = (set2.tag * SET_ASSOIATIVE_CACHE_LINES + index) * BLOCK_SIZE;
                        for (int i=0;i<64;i++) {
                            ram.memory[blockStartAddr+i] = set2.bytes[i];
                        }
                    }
                    //Load corresponding cacheline from RAM and edit value
                    set2 = LoadCacheBlockFromRAM(addr,ram, set_associative_on);
                    set2.bytes[offset] = uint8_t(data);
                    set2.valid = 1;
                    set2.tag = tag;
                    set2.dirty = 1;
                    set2.LRU = 0;
                    set1.LRU = 1;
                    break;
                    
                }
                case (Store_op::STOREHALF): {
                    if (offset > 62) {
                        throw runtime_error("Store block in separate cachelines");
                    }
                    uint8_t lowByte  = uint16_t(data) & 0xFF;
                    uint8_t highByte = (uint16_t(data) >> 8) & 0xFF;
                    //check set 1 hit
                    if (set1.valid == 1 and set1.tag == tag) {
                        set1.bytes[offset] = lowByte;
                        set1.bytes[offset+1] = highByte;
                        set1.dirty = 1;
                        set1.LRU = 0;
                        break;
                    }
                    //check set 2 hit
                    else if (set2.valid == 1 and set2.tag == tag) {
                        set2.bytes[offset] = lowByte;
                        set2.bytes[offset+1] = highByte;
                        set2.dirty = 1;
                        set2.LRU = 0;
                        break;
                    }
                    //check set 1 empty
                    else if (set1.valid == 0) {
                        set1 = LoadCacheBlockFromRAM(addr,ram,set_associative_on);
                        set1.bytes[offset] = lowByte;
                        set1.bytes[offset+1] = highByte;
                        set1.valid = 1;
                        set1.dirty = 1;
                        set1.tag = tag;
                        set1.LRU = 0;
                        set2.LRU = 1;
                        break;
                    }
                    //check set2 empty
                    else if (set2.valid == 0) {
                        set2 = LoadCacheBlockFromRAM(addr,ram,set_associative_on);
                        set2.bytes[offset] = lowByte;
                        set2.bytes[offset+1] = highByte;
                        set2.valid = 1;
                        set2.dirty = 1;
                        set2.tag = tag;
                        set2.LRU = 0;
                        set1.LRU = 1;
                        break;
                    }
                    //swap set1
                    else if (set1.LRU == 1) {
                        if (set1.dirty == 1) {
                            uint32_t blockStartAddr = (set1.tag * SET_ASSOIATIVE_CACHE_LINES + index) * BLOCK_SIZE;
                            for (int i=0;i<64;i++) {
                                ram.memory[blockStartAddr+i] = set1.bytes[i];
                            }
                        }
                        set1 = LoadCacheBlockFromRAM(addr,ram,set_associative_on);
                        set1.bytes[offset] = lowByte;
                        set1.bytes[offset+1] = highByte;
                        set1.valid = 1;
                        set1.tag = tag;
                        set1.dirty = 1;
                        set1.LRU = 0;
                        set2.LRU = 1;
                        break;
                    }
                    else if (set2.LRU == 1) {
                        if (set2.dirty == 1) {
                            uint32_t blockStartAddr = (set2.tag * SET_ASSOIATIVE_CACHE_LINES + index) * BLOCK_SIZE;
                            for (int i=0;i<64;i++) {
                                ram.memory[blockStartAddr+i] = set2.bytes[i];
                            }
                        }
                        set2 = LoadCacheBlockFromRAM(addr,ram,set_associative_on);
                        set2.bytes[offset] = lowByte;
                        set2.bytes[offset+1] = highByte;
                        set2.valid = 1;
                        set2.tag = tag;
                        set2.dirty = 1;
                        set2.LRU = 0;
                        set1.LRU = 1;
                        break;
                    }
                }
                case(Store_op::STOREWORD): {
                    if (offset > 60) {
                        throw runtime_error("Store block in separate cachelines");
                    }
                    uint8_t byte0 =  data & 0xFF;
                    uint8_t byte1 = (data >> 8)  & 0xFF;
                    uint8_t byte2 = (data >> 16) & 0xFF;
                    uint8_t byte3 = (data >> 24) & 0xFF;

                    //check set 1 hit
                    if (set1.valid == 1 and set1.tag == tag) {
                        set1.bytes[offset] = byte0;
                        set1.bytes[offset+1] = byte1;
                        set1.bytes[offset+2] = byte2;
                        set1.bytes[offset+3] = byte3;
                        set1.dirty = 1;
                        set1.LRU = 0;
                        set2.LRU = 1;
                        break;
                    }
                    //check set 2 hit
                    else if (set2.valid == 1 and set2.tag == tag) {
                        set2.bytes[offset] = byte0;
                        set2.bytes[offset+1] = byte1;
                        set2.bytes[offset+2] = byte2;
                        set2.bytes[offset+3] = byte3;
                        set2.dirty = 1;
                        set2.LRU = 0;
                        set1.LRU = 1;
                        break;
                    }
                    //check set1 empty
                    else if (set1.valid == 0) {
                        set1 = LoadCacheBlockFromRAM(addr,ram,set_associative_on);
                        set1.bytes[offset] = byte0;
                        set1.bytes[offset+1] = byte1;
                        set1.bytes[offset+2] = byte2;
                        set1.bytes[offset+3] = byte3;
                        set1.dirty = 1;
                        set1.valid = 1;
                        set1.tag = tag;
                        set1.LRU = 0;
                        set2.LRU = 1;
                        break;
                    }
                    else if (set2.valid == 0) {
                        set2 = LoadCacheBlockFromRAM(addr,ram, set_associative_on);
                        set2.bytes[offset] = byte0;
                        set2.bytes[offset+1] = byte1;
                        set2.bytes[offset+2] = byte2;
                        set2.bytes[offset+3] = byte3;
                        set2.dirty = 1;
                        set2.valid = 1;
                        set2.tag = tag;
                        set2.LRU = 0;
                        set1.LRU = 1;
                        break;
                    }
                    else if (set1.LRU == 1){
                        if (set1.valid == 1 and set1.dirty == 1) {
                            uint32_t blockStartAddr = (set1.tag * SET_ASSOIATIVE_CACHE_LINES + index) * BLOCK_SIZE;
                            for (int i=0;i<64;i++) {
                                ram.memory[blockStartAddr+i] = set1.bytes[i];
                            }
                        }
                        //Load corresponding cacheline from RAM and edit value
                        set1 = LoadCacheBlockFromRAM(addr,ram, set_associative_on);
                        set1.bytes[offset] = byte0;
                        set1.bytes[offset+1] = byte1;
                        set1.bytes[offset+2] = byte2;
                        set1.bytes[offset+3] = byte3;
                        set1.valid = 1;
                        set1.tag = tag;
                        set1.dirty = 1;
                        set1.LRU = 0;
                        set2.LRU = 1;
                        break;
                    }
                    else if (set2.LRU == 1) {
                        if (set2.valid == 1 and set2.dirty == 1 and set2.LRU == 1) {
                            //store dirty data
                            uint32_t blockStartAddr = (set2.tag * SET_ASSOIATIVE_CACHE_LINES + index) * BLOCK_SIZE;
                            for (int i=0;i<64;i++) {
                                ram.memory[blockStartAddr+i] = set2.bytes[i];
                            }
                        }
                        //Load corresponding cacheline from RAM and edit value
                        set2 = LoadCacheBlockFromRAM(addr,ram, set_associative_on);
                        set2.bytes[offset] = byte0;
                        set2.bytes[offset+1] = byte1;
                        set2.bytes[offset+2] = byte2;
                        set2.bytes[offset+3] = byte3;
                        set2.valid = 1;
                        set2.tag = tag;
                        set2.dirty = 1;
                        set2.LRU = 0;
                        set1.LRU = 1;
                        break;
                    }
                }
            }
        }
    }

    uint32_t Load(Memory_op Mem_op,Memory_data_type Memory_data_type, uint32_t addr,L2Cache& l2cache,RAM& ram, int set_associative_on, int& total_mem_read, int& data_cache_hit_count, int& fetch_cache_hit_count) {
        switch (Mem_op) {
            case(Memory_op::READBYTE):
                total_mem_read += 1;
                switch (Memory_data_type) {
                    case(Memory_data_type::UNSIGN):return readByte(addr,l2cache,ram,set_associative_on, data_cache_hit_count,fetch_cache_hit_count,0);
                    case(Memory_data_type::SIGN):return static_cast<int32_t>(static_cast<int8_t>(readByte(addr,l2cache,ram,set_associative_on, data_cache_hit_count,fetch_cache_hit_count,0)));
                default: throw runtime_error("Unknown Memory Data Type");
                }
            case(Memory_op::READHALF):
                total_mem_read += 1;
                switch (Memory_data_type) {
                    case(Memory_data_type::UNSIGN):return readHalfWord(addr,l2cache,ram,set_associative_on, data_cache_hit_count,fetch_cache_hit_count,0);break;
                    case(Memory_data_type::SIGN):return static_cast<int32_t>(static_cast<int16_t>(readHalfWord(addr,l2cache,ram,set_associative_on, data_cache_hit_count,fetch_cache_hit_count,0)));break;
                default: throw runtime_error("Unknown Memory Data Type");
                }
            case(Memory_op::READWORD):
                total_mem_read += 1;
                return readWord(addr,l2cache,ram,set_associative_on, data_cache_hit_count,fetch_cache_hit_count,0);
            case(Memory_op::NO_MEMORY_OP):return 0;
            default: throw runtime_error("Unknown Memory Operation");
        }
    }

};
#endif //RISC_V_CPU_SIMULATOR_L1CACHE_H
