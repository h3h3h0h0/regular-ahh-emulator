#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include <cstdint>

using namespace std;

class Memory {
    int32_t *data;
    uint32_t size; //size of ram
    public:
    bool load(int32_t &reg, uint32_t base, uint32_t offset);
    bool store(int32_t &reg, uint32_t base, uint32_t offset);
};