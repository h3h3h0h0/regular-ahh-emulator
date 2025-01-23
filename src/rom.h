#pragma once
#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include <cstdint>
#include <vector>
#include <fstream>
#include "utils.h"
#include "mmdevice.h"

using namespace std;

class ROM: public MMD { //this simulates how a ROM operates in a computer with a read-only file, writes throw an error
    memrange mr;
    int32_t *data;
    public:
    bool load_file(string fn); //if the file isn't long enough, pads with zeroes
    bool read(int32_t &reg, uint32_t add, opsize s, bool sign=true);
    bool write(int32_t &reg, uint32_t add, opsize s) {return false;}
    vector<memrange> get_reads();
    vector<memrange> get_writes() {return vector<memrange>();}
    ROM(int64_t uid, uint32_t addr, uint32_t sz); //starting MMIO address will be forced to word alignment (last 2 bits cleared), size of ROM in WORDS
    //it will cap the size if the ROM exceeds memory addressable ranges
    ~ROM();
};