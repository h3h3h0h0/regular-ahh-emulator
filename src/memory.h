#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include <cstdint>
#include <vector>
#include "mmdevice.h"
#include "misc.h"

using namespace std;

class Memory {
    int32_t *data;
    vector<pair<memrange, int>> mapping; //an index of -1 means access to main memory, any other means access to devices
    vector<MMD*> devices;
    public:
    bool connect_device(MMD *d);
    bool load(int32_t &reg, uint32_t base, uint32_t offset);
    bool store(int32_t &reg, uint32_t base, uint32_t offset);
};