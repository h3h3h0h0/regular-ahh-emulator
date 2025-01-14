#include "memory.h"

//bool Memory::connect_device(MMD *d) {
//    return false; //implement later
//}
//bool Memory::disconnect_device(MMD *d) {
//    return false;
//}
bool Memory::load(int32_t &reg, uint32_t base, int16_t offset, opsize s, bool sign) {
    size_t combined = (size_t)base+(size_t)offset;
    //alignment and range checks
    if(combined < 0 || combined > (int64_t)1<<32) return false;
    if(s == HALF && combined%2 != 0) return false;
    else if(s == WORD && combined%4 != 0) return false;

    size_t idx = combined>>2; //which word are we getting
    size_t shift = (3-(combined%4))*8; //for positions 0, 1, 2 in byte addressing, we need to shift the word over
    int64_t temp = ((int64_t)data[idx])>>shift; //do the shift
    if(s == BYTE) {
        int8_t tempbyte = temp;
        reg = (int32_t)(sign? (int32_t)tempbyte : (uint32_t)tempbyte);
    } else if(s == HALF) {
        int16_t temphalf = temp;
        reg = (int32_t)(sign? (int32_t)temphalf : (uint32_t)temphalf);
    } else {
        reg = (int32_t)temp;
    }
    return true;
}
bool Memory::store(int32_t &reg, uint32_t base, int16_t offset, opsize s) {
    size_t combined = (size_t)base+(size_t)offset;
    //alignment and range checks
    if(combined < 0 || combined > (int64_t)1<<32) return false;
    if(s == HALF && combined%2 != 0) return false;
    else if(s == WORD && combined%4 != 0) return false;

    size_t idx = combined>>2; //which word are we setting
    size_t shift = (3-(combined%4))*8; //for positions 0, 1, 2 in byte addressing, we need to shift the word over
    if(s == BYTE) {
        int32_t mask = 0xFF<<shift;
        data[idx] = (data[idx]&(~mask))+(reg<<shift);
    } else if(s == HALF) {
        int32_t mask = 0xFFFF<<shift;
        data[idx] = (data[idx]&(~mask))+(reg<<shift);
    } else {
        data[idx] = reg;
    }
    return true;
}
Memory::Memory(memrange mainmem) {
    data = new int32_t[1<<30];
    read_mapping.emplace_back(mainmem, -1);
    write_mapping.emplace_back(mainmem, -1);
}
Memory::~Memory() {
    delete [] data;
}