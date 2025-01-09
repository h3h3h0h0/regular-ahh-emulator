#include <cstdint>
#include <compare>

using namespace std;

struct memrange {
    uint32_t start; //first address that is valid in this range
    uint32_t end; //last address that is valid in this range    
    auto operator<=>(const memrange other) { //compare for overlap between two memranges
        if(end < other.start) return weak_ordering::less;
        else if(other.end < start) return weak_ordering::greater;
        else return weak_ordering::equivalent;
    }
    auto operator<=>(const uint32_t &addr) { //compares for if address in range (both orders)
        if(start > addr) return weak_ordering::greater;
        else if(addr <= end) return weak_ordering::equivalent;
        else return weak_ordering::less;
    }
    friend auto operator<=>(const uint32_t &addr, const memrange &mr) noexcept {
        if(mr.start > addr) return weak_ordering::less;
        else if(addr <= mr.end) return weak_ordering::equivalent;
        else return weak_ordering::greater;
    }
};
