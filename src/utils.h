#pragma once
#include <compare>

enum opsize { //how many times do we have to divide the word?
    WORD = 0,
    HALF = 1,
    BYTE = 2
};

struct memrange {
    uint32_t start; //first address that is valid in this range
    uint32_t end; //last address that is valid in this range  
    auto operator<=>(const memrange &second) const { //compare for overlap between two memranges
        if(end < second.start) return std::weak_ordering::less;
        else if(second.end < start) return std::weak_ordering::greater;
        else return std::weak_ordering::equivalent;
    }  
    auto operator==(const memrange &second) const {
        return (*this <=> second) == std::weak_ordering::equivalent;
    }
    
bool validate(uint32_t &addr, opsize s) {
    if(s == WORD) {
        if(addr > 4294967292) return false; //the other end goes past the addressing space!
        return (addr >= start && addr+3 <= end);
    } else if(s == HALF) {
        if(addr > 4294967294) return false; //the other end goes past the addressing space!
        return (addr >= start && addr+1 <= end);
    } else {
        //for a byte, any address will not cause overflow
        return (addr >= start && addr <= end);
    }
}
};

