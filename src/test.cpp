#include "cpu.h"
#include "memory.h"
#include <iostream>
#include <cstdint>

using namespace std;

int main() {
    Memory *m = new Memory({0x0, 0xFFFFFFFF});
    CPU *c = new CPU(m);
    while(1) {
        uint32_t instr;
        cin>>hex>>instr;
        cout<<c->lookup(instr)<<endl;
    }
    delete c;
    delete m;
    return 0;
}