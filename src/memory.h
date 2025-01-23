#pragma once
#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include <cstdint>
#include <vector>
#include <cstdint>
#include "utils.h"
#include "mmdevice.h"

using namespace std;

class Memory { //for now, we will just have the maximum amount of memory without any reserved ranges for IO
    protected:
    int32_t *data;
    vector<pair<memrange, int64_t>> read_mapping; //an index of -1 means access to main memory, any other means access to devices
    vector<pair<memrange, int64_t>> write_mapping; 
    map<int, MMD*> devices;
    uint32_t get_physical(uint32_t virt); //performs the (limited) address translation available in the MIPS 1 architecture, override if needed
    public:
    bool connect_device(MMD *d);
    bool disconnect_device(MMD *d);
    bool load(int32_t &reg, uint32_t base, int16_t offset, opsize s, bool sign=true);
    bool store(int32_t &reg, uint32_t base, int16_t offset, opsize s);
    void load_to_main(int32_t *d, size_t start, size_t size);
    Memory(vector<memrange> mainmem_read, vector<memrange> mainmem_write);
    ~Memory();
};