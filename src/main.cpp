#include "cpu.h"
#include "memory.h"
#include <iostream>
#include <cstdint>

using namespace std;

int main() {
    Memory *m = new Memory({0x0, 0xFFFFFFFF});
    CPU *c = new CPU(m);
    int32_t program = 0b01101000000000000000000000001010;
    cout<<c->lookup(program)<<endl;
    m->load_to_main(&program, 0, 1);
    m->load(program, 0, 0, WORD);
    cout<<"at 0: "<<program<<endl;
    c->load_at(0);
    c->run();
    cout<<"RUN FINISHED!"<<endl;
    delete c;
    delete m;
    return 0;
}