#include "rom.h"

using namespace std;

bool ROM::load_file(string fn) {
    ifstream ifs(fn);
    if(!ifs) return false;
    bool has = true;
    for(uint32_t i = mr.start; i <= mr.end; i+=4) {
        int32_t temp = 0;
        char t = 0;
        for(int j = 0; j < 4; j++) {
            temp = temp<<8;
            if(has) {
                if(ifs.get(t)) {
                    temp += t;
                } else {
                    has = false;
                }
            }
        }
    }
    ifs.close();
}
bool ROM::read(int32_t &reg, uint32_t add, opsize s, bool sign) {
    //range check
    if(mr.validate(add, s) != weak_ordering::equivalent) return false;
    //alignment check
    if(s == HALF && add%2 != 0) return false;
    else if(s == WORD && add%4 != 0) return false;
    //do the read
    size_t idx = add>>2; //which word are we getting
    int64_t temp = (int64_t)data[idx]; //do the shift
    if(s == BYTE) {       
        size_t shift = (3-(add%4))*8; //for positions 0, 1, 2 in byte addressing, we need to shift the word over
        int8_t tempbyte = temp>>shift;
        reg = (int32_t)(sign? (int32_t)tempbyte : (uint32_t)tempbyte);
    } else if(s == HALF) {
        size_t shift = (2-(add%4))*8; //for positions 0, 2 in byte addressing, we need to shift the word over
        int16_t temphalf = temp>>shift;
        reg = (int32_t)(sign? (int32_t)temphalf : (uint32_t)temphalf);
    } else {
        reg = (int32_t)temp;
    }
    return true;
}
vector<memrange> ROM::get_reads() {
    vector<memrange> v;
    v.push_back(mr);
    return v;
}
ROM::ROM(int64_t uid, uint32_t addr, uint32_t sz): MMD(uid), data(new int32_t[sz]) {
    int64_t temp = addr+((int64_t)sz)*4-1;
    temp = max(temp, (int64_t)0xFFFFFFFF);
    mr.start = (addr>>2)<<2;
    mr.end = (uint32_t)temp;
}
ROM::~ROM() {
    delete [] data;
}