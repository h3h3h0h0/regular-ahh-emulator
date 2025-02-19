#pragma once
#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include <cstdint>
#include <vector>
#include "utils.h"

using namespace std;

class MMD {
    protected:
    int64_t uid;
    MMD(uint64_t id) {uid = id;}
    public:
    int64_t get_uid() {return uid;}
    virtual bool read(int32_t &reg, uint32_t add, opsize s, bool sign=true) = 0;
    virtual bool write(int32_t &reg, uint32_t add, opsize s) = 0;
    virtual vector<memrange> get_reads();
    virtual vector<memrange> get_writes();
    virtual ~MMD() {}
};