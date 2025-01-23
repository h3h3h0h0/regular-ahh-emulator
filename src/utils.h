#pragma once
#include <compare>

enum opsize { //how many times do we have to divide the word?
    WORD = 0,
    HALF = 1,
    BYTE = 2
};
enum mem_status { //not used currently
    SUCCESS = 0,
    PAGEFAULT = 1,
    INVACCESS = 2 //improper alignment or out of bounds
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
    
std::weak_ordering validate(uint32_t &addr, opsize s) {
    if(s == WORD) {
        if(addr > 4294967292) return std::weak_ordering::greater; //the other end overflows
        else if(addr+3 > end) return std::weak_ordering::greater; 
        else if(addr < start) return std::weak_ordering::less;
        else return std::weak_ordering::equivalent;
    } else if(s == HALF) {
        if(addr > 4294967294) return std::weak_ordering::greater; //the other end overflows
        else if(addr+1 > end) return std::weak_ordering::greater; 
        else if(addr < start) return std::weak_ordering::less;
        else return std::weak_ordering::equivalent;
    } else {
        //for a byte, any address will not cause overflow
        if(addr > end) return std::weak_ordering::greater;
        else if(addr < start) return std::weak_ordering::less;
        else return std::weak_ordering::equivalent;
    }
}
};

