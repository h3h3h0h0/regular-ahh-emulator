#include "memory.h"

#define KUSEG 0x00000000
#define KSEG0 0x80000000
#define KSEG1 0xA0000000
#define KSEG2 0xC0000000

using namespace std;

uint32_t Memory::get_physical(uint32_t virt) { //we will emulate a non-MMU version of the R3000 in terms of memory
    if(virt < KSEG0) {
        //KUSEG will be "moved" to the top 2GB
        return virt+KSEG0;
    } else if(virt < KSEG1) {
        return virt-KSEG0; //maps to first 512MB
    } else if(virt < KSEG2) {
        return virt-KSEG1;
    } else {
        //KSEG2 will take up the top of memory
        return virt;
    }
}

int get_device(vector<pair<memrange, int64_t>> &mp, uint32_t addr, opsize s) { //memory ranges MUST be sorted and non-overlapping
    int start = 0;
    int end = mp.size()-1;
    int cur = (start+end)/2;
    weak_ordering res = mp[cur].first.validate(addr, s);
    while(start < end) {
        res = mp[cur].first.validate(addr, s);
        if(res == weak_ordering::equivalent) break;
        else if(res == weak_ordering::greater) {
            start = cur;
            cur = (start+end)/2;
        } else {
            end = cur;
            cur = (start+end)/2;
        }
    }
    if(res == weak_ordering::equivalent) return mp[cur].second;
    return -2; //since -1 is taken by main memory
}
int get_idx(vector<pair<memrange, int64_t>> &mp, memrange mr) { //get the insertion index of a memory range, or -1 if it cannot be inserted without overlap
    if(mp.size() == 0) return 0;
    int start = 0;
    int end = mp.size()-1;
    int cur = (start+end)/2;
    while(start < end) {
        if(mp[cur].first > mr) {
            if(cur == 0) return 0;
            else {
                if(mp[cur-1].first < mr) return cur;
                else {
                    end = cur;
                    cur = (start+end)/2;
                }
            }
        } else if(mp[cur].first == mr) {
            return -1; //equivalence means overlap
        } else {
            if(cur == mp.size()-1) return mp.size();
            else {
                start = cur;
                cur = (start+end)/2;
            }
        }
    }
    return -1;
}

bool Memory::connect_device(MMD *d) {
    int ndev_id = d->get_uid();
    if(devices.count(ndev_id)) return false; //device with this UID already exists!
    vector<pair<memrange, int64_t>> rcopy = read_mapping; //we need to easily roll back the changes we try out
    vector<pair<memrange, int64_t>> wcopy = write_mapping;
    vector<memrange> newreads = d->get_reads();
    vector<memrange> newwrites = d->get_writes();
    for(int i = 0; i < newreads.size(); i++) {
        int idx = get_idx(rcopy, newreads[i]);
        if(idx == -1) return false;
        pair<memrange, int64_t> to_insert;
        to_insert.first = newreads[i];
        to_insert.second = ndev_id;
        rcopy.insert(rcopy.begin()+idx, to_insert);
    }
    for(int i = 0; i < newwrites.size(); i++) {
        int idx = get_idx(wcopy, newwrites[i]);
        if(idx == -1) return false;
        pair<memrange, int> to_insert;
        to_insert.first = newwrites[i];
        to_insert.second = ndev_id;
        wcopy.insert(wcopy.begin()+idx, to_insert);
    }
    devices[ndev_id] = d;
    read_mapping = rcopy;
    write_mapping = wcopy;
    return true;
}
bool Memory::disconnect_device(MMD *d) {
    int devid = d->get_uid();
    if(!devices.count(devid)) return false; //device does not exist
    devices.erase(devid);
    int idx = 0;
    while(idx < read_mapping.size()) {
        if(read_mapping[idx].second == devid) read_mapping.erase(read_mapping.begin()+idx);
        else idx++;
    }
    idx = 0;
    while(idx < write_mapping.size()) {
        if(write_mapping[idx].second == devid) write_mapping.erase(write_mapping.begin()+idx);
        else idx++;
    }
    return true;
}
void Memory::load_to_main(int32_t *d, size_t start, size_t size) {
    //do error checking later
    copy(d, d+size, data+start);
}
bool Memory::load(int32_t &reg, uint32_t base, int16_t offset, opsize s, bool sign) {
    size_t combined = (size_t)base+(size_t)offset;
    //alignment and range checks
    if(combined < 0 || combined > (int64_t)1<<32) return false;
    if(s == HALF && combined%2 != 0) return false;
    else if(s == WORD && combined%4 != 0) return false;

    //address translation
    combined = get_physical(combined);

    //check for device loads
    int64_t devid = get_device(read_mapping, (uint32_t)combined, s);
    if(devid >= 0) return devices[devid]->read(reg, (uint32_t)combined, s);
    else if(devid == -2) return false; //we couldn't find any mapping
    //when mapping is -1, means main memory

    size_t idx = combined>>2; //which word are we getting
    int64_t temp = (int64_t)data[idx]; //do the shift
    if(s == BYTE) {       
        size_t shift = (3-(combined%4))*8; //for positions 0, 1, 2 in byte addressing, we need to shift the word over
        int8_t tempbyte = temp>>shift;
        reg = (int32_t)(sign? (int32_t)tempbyte : (uint32_t)tempbyte);
    } else if(s == HALF) {
        size_t shift = (2-(combined%4))*8; //for positions 0, 2 in byte addressing, we need to shift the word over
        int16_t temphalf = temp>>shift;
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

    //address translation
    combined = get_physical(combined);

    //check for device stores
    int64_t devid = get_device(write_mapping, (uint32_t)combined, s);
    if(devid >= 0) return devices[devid]->write(reg, (uint32_t)combined, s);
    else if(devid == -2) return false; //we couldn't find any mapping
    //when mapping is -1, means main memory

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
Memory::Memory(vector<memrange> mainmem_read, vector<memrange> mainmem_write): data(new int32_t[1<<30]) { //sometimes there are sections of main memory that are read only (such as parts which are mapped to ROM for startup code)
    //instead of returning error, we will simply not add overlapping main memory segments
    //it's bad practice though to pass those in
    for(int i = 0; i < mainmem_read.size(); i++) {
        int idx = get_idx(read_mapping, mainmem_read[i]);
        if(idx == -1) continue;
        pair<memrange, int> to_insert;
        to_insert.first = mainmem_read[i];
        to_insert.second = -1;
        read_mapping.insert(read_mapping.begin()+idx, to_insert);
    }
    for(int i = 0; i < mainmem_write.size(); i++) {
        int idx = get_idx(write_mapping, mainmem_write[i]);
        if(idx == -1) continue;
        pair<memrange, int> to_insert;
        to_insert.first = mainmem_write[i];
        to_insert.second = -1;
        write_mapping.insert(write_mapping.begin()+idx, to_insert);
    }
}
Memory::~Memory() {
    delete [] data;
    //devices are not the memory controller's responsibility
}