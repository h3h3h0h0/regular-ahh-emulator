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
    vector<pair<memrange, int>> read_mapping; //an index of -1 means access to main memory, any other means access to devices
    vector<pair<memrange, int>> write_mapping; 
    //vector<MMD*> devices;
    public:
    //bool connect_device(MMD *d);
    //bool disconnect_device(MMD *d);
    bool load(int32_t &reg, uint32_t base, int16_t offset, opsize s, bool sign=false);
    bool store(int32_t &reg, uint32_t base, int16_t offset, opsize s);
    void load_to_main(int32_t *d, size_t start, size_t size);
    Memory(memrange mainmem);
    ~Memory();
};