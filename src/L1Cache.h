//16kb L1 cache
//256 cache lines
#ifndef RISC_V_CPU_SIMULATOR_L1CACHE_H
#define RISC_V_CPU_SIMULATOR_L1CACHE_H
#include<cstdint>

#include "CPUcore.h"
#include "RAM.h"
#include "L2Cache.h"

constexpr int BLOCK_SIZE = 64;
constexpr int CACHE_LINES = 256;

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
    L1Cache() { //initialize
        for (int i = 0; i < CACHE_LINES; i++) {
            cachelines[i].valid = 0;
            cachelines[i].tag = 0;
            for (int j = 0; j < BLOCK_SIZE; j++) {
                cachelines[i].bytes[j] = 0;
            }
        }
    }

    CacheLine LoadCacheBlockFromRAM(uint32_t addr, RAM& ram) {
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

    uint32_t readWord(uint32_t addr,L2Cache& l2cache,RAM& ram, int& cache_hit_count, int is_command_visit) { //loads data from ram if missed, can also be used to access data in L1Cache
        uint32_t block = addr/64;
        uint32_t index = block%256;
        uint32_t tag = block/256;
        uint32_t offset = addr%64;

        CacheLine& target_cacheLine = cachelines[index];
        if (target_cacheLine.valid == 1 and target_cacheLine.tag == tag) { //hit
            if (is_command_visit != 1) {
                cache_hit_count += 1;
            }
            //cout<<"hit L1"<<endl;
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
            target_cacheLine = LoadCacheBlockFromRAM(addr, ram);
            return
                  (uint32_t)target_cacheLine.bytes[offset]
                | ((uint32_t)target_cacheLine.bytes[offset+1] << 8)
                | ((uint32_t)target_cacheLine.bytes[offset+2] << 16)
                | ((uint32_t)target_cacheLine.bytes[offset+3] << 24);
        }
    }

    uint16_t readHalfWord(uint32_t addr,L2Cache& l2cache,RAM& ram, int& cache_hit_count, int is_command_visit) { //loads data from ram if missed, can also be used to access data in L1Cache
        uint32_t block = addr/64;
        uint32_t index = block%256;
        uint32_t tag = block/256;
        uint32_t offset = addr%64;

        CacheLine& target_cacheLine = cachelines[index];
        if (target_cacheLine.valid == 1 and target_cacheLine.tag == tag) { //hit
            if (is_command_visit != 1) {
                cache_hit_count += 1;
            }
            //cout<<"hit L1"<<endl;
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
            target_cacheLine = LoadCacheBlockFromRAM(addr,ram);
            return
                  (uint32_t)target_cacheLine.bytes[offset]
                | ((uint32_t)target_cacheLine.bytes[offset+1] << 8);
        }
    }

    uint8_t readByte(uint32_t addr,L2Cache& l2cache,RAM& ram, int& cache_hit_count, int is_command_visit) { //loads data from ram if missed, can also be used to access data in L1Cache
        uint32_t block = addr/64;
        uint32_t index = block%256;
        uint32_t tag = block/256;
        uint32_t offset = addr%64;

        CacheLine& target_cacheLine = cachelines[index];
        if (target_cacheLine.valid == 1 and target_cacheLine.tag == tag) { //hit
            if (is_command_visit != 1) {
                cache_hit_count += 1;
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
            target_cacheLine = LoadCacheBlockFromRAM(addr,ram);
            return
                  (uint32_t)target_cacheLine.bytes[offset];
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

    void Store(Store_op Store_op, uint32_t addr, uint32_t data,L2Cache& l2cache,RAM& ram) {
        uint32_t block = addr/64;
        uint32_t index = block%256;
        uint32_t tag = block/256;
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
                    cachelines[index] = LoadCacheBlockFromRAM(addr,ram);
                    cachelines[index].bytes[offset] = uint8_t(data);
                    cachelines[index].valid = 1;
                    cachelines[index].tag = tag;
                }
                cachelines[index].dirty = 1;
                break;
            }
            case(Store_op::STOREHALF): {
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
                    cachelines[index] = LoadCacheBlockFromRAM(addr,ram);
                    cachelines[index].bytes[offset] = lowByte;
                    cachelines[index].bytes[offset+1] = highByte;
                    cachelines[index].valid = 1;
                    cachelines[index].tag = tag;
                }
                cachelines[index].dirty = 1;
                break;
            }
            case(Store_op::STOREWORD): {
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
                    cachelines[index] = LoadCacheBlockFromRAM(addr,ram);
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

    uint32_t Load(Memory_op Mem_op,Memory_data_type Memory_data_type, uint32_t addr,L2Cache& l2cache,RAM& ram,int& total_mem_read, int& cache_hit_count) {
        switch (Mem_op) {
            case(Memory_op::READBYTE):
                total_mem_read += 1;
                switch (Memory_data_type) {
                    case(Memory_data_type::UNSIGN):return readByte(addr,l2cache,ram,cache_hit_count,0);
                    case(Memory_data_type::SIGN):return static_cast<int32_t>(static_cast<int8_t>(readByte(addr,l2cache,ram,cache_hit_count,0)));
                default: throw runtime_error("Unknown Memory Data Type");
                }
            case(Memory_op::READHALF):
                total_mem_read += 1;
                switch (Memory_data_type) {
                    case(Memory_data_type::UNSIGN):return readHalfWord(addr,l2cache,ram,cache_hit_count,0);break;
                    case(Memory_data_type::SIGN):return (int32_t)readHalfWord(addr,l2cache,ram,cache_hit_count,0);break;
                default: throw runtime_error("Unknown Memory Data Type");
                }
            case(Memory_op::READWORD):
                total_mem_read += 1;
                return readWord(addr,l2cache,ram,cache_hit_count,0);
            case(Memory_op::NO_MEMORY_OP):return 0;
            default: throw runtime_error("Unknown Memory Operation");
        }
    }

};
#endif //RISC_V_CPU_SIMULATOR_L1CACHE_H
