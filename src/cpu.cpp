#include "cpu.h"

//register encoding: ooooooss sssttttt dddddaaa aaffffff
#define OMASK 0b11111100000000000000000000000000
#define FMASK 0b00000000000000000000000000111111
#define SMASK 0b00000011111000000000000000000000
#define TMASK 0b00000000000111110000000000000000
#define DMASK 0b00000000000000001111100000000000
#define AMASK 0b00000000000000000000011111000000
//how much do you need to right-shift to get the actual value?
#define OSHIFT 26
#define FSHIFT 0
#define SSHIFT 21
#define TSHIFT 16
#define DSHIFT 11
#define ASHIFT 6

//immediate encoding: ooooooss sssttttt iiiiiiii iiiiiiii
#define IMASK 0b00000000000000001111111111111111

#define ISHIFT 0

//jump encoding: ooooooii iiiiiiii iiiiiiii iiiiiiii
#define JMASK 0b00000011111111111111111111111111

#define JSHIFT 0

void add(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.regs[rd] = c.regs[rs]+c.regs[rt];
}
void addu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.regs[rd] = (int32_t)(((uint32_t)c.regs[rs])+((uint32_t)c.regs[rt]));
}
void sub(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.regs[rd] = c.regs[rs]-c.regs[rt];
}
void subu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.regs[rd] = (int32_t)(((uint32_t)c.regs[rs])-((uint32_t)c.regs[rt]));
}
void mult(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    int64_t full = ((int64_t)c.regs[rs])*((int64_t)c.regs[rt]);
    c.hi = (int32_t)(full>>32);
    c.lo = (int32_t)full;
}
void multu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    int64_t full = ((uint64_t)c.regs[rs])*((uint64_t)c.regs[rt]);
    c.hi = (int32_t)(full>>32);
    c.lo = (int32_t)full;
}
void div(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.hi = c.regs[rs]%c.regs[rt];
    c.lo = c.regs[rs]/c.regs[rt];
}
void divu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.hi = (int32_t)(((uint32_t)c.regs[rs])%((uint32_t)c.regs[rt]));
    c.lo = (int32_t)(((uint32_t)c.regs[rs])/((uint32_t)c.regs[rt]));
}
void aand(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.regs[rd] = c.regs[rs]&c.regs[rt];
}
void nor(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.regs[rd] = ~(c.regs[rs]|c.regs[rt]);
}
void oor(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.regs[rd] = c.regs[rs]|c.regs[rt];
}
void sllv(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.regs[rd] = c.regs[rt]<<(((uint8_t)c.regs[rs])&0x1F);
}
void srav(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.regs[rd] = c.regs[rt]>>(((uint8_t)c.regs[rs])&0x1F);
}
void srlv(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.regs[rd] = ((uint32_t)c.regs[rt])>>(((uint8_t)c.regs[rs])&0x1F);
}
void xxor(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.regs[rd] = c.regs[rs]^c.regs[rt];
}
void sll(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.regs[rd] = c.regs[rt]<<(a&0x1F);
}
void sra(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.regs[rd] = c.regs[rt]>>(a&0x1F);
}
void srl(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.regs[rd] = ((uint32_t)c.regs[rt])>>(a&0x1F);
}

//math with immediates (goes in imm_instructions)
void addi(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    c.regs[rt] = c.regs[rs]+imm;
}
void addiu(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    c.regs[rt] = ((uint32_t)c.regs[rs])+((uint16_t)imm);
}
void andi(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    c.regs[rt] = c.regs[rs]&((uint32_t)imm);
}
void ori(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    c.regs[rt] = c.regs[rs]|((uint32_t)imm);
}
void xori(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    c.regs[rt] = c.regs[rs]^((uint32_t)imm);
}

//memory load and store operations (goes in imm_instructions)
void lb(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    bool result = c.mem->load(c.regs[rt], c.regs[rs], imm, BYTE);
    if(!result) {
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID BYTEREAD PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm<<endl;
        c.errors.push_back(oss.str());
    }
}
void lbu(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    bool result = c.mem->load(c.regs[rt], c.regs[rs], imm, BYTE, true);
    if(!result) {
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID UBYTEREAD PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm<<endl;
        c.errors.push_back(oss.str());
    }
}
void lh(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    bool result = c.mem->load(c.regs[rt], c.regs[rs], imm, HALF);
    if(!result) {
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID HALFREAD PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm<<endl;
        c.errors.push_back(oss.str());
    }
}
void lhu(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    bool result = c.mem->load(c.regs[rt], c.regs[rs], imm, HALF, true);
    if(!result) {
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID UHALFREAD PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm<<endl;
        c.errors.push_back(oss.str());
    }
}
void lw(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    bool result = c.mem->load(c.regs[rt], c.regs[rs], imm, WORD, true);
    if(!result) {
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID WORDREAD PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm<<endl;
        c.errors.push_back(oss.str());
    }
}
void sb(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    bool result = c.mem->store(c.regs[rt], c.regs[rs], imm, BYTE);
    if(!result) {
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID BYTESTORE PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm<<endl;
        c.errors.push_back(oss.str());
    }
}
void sh(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    bool result = c.mem->store(c.regs[rt], c.regs[rs], imm, HALF);
    if(!result) {
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID BYTESTORE PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm<<endl;
        c.errors.push_back(oss.str());
    }
}
void sw(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    bool result = c.mem->store(c.regs[rt], c.regs[rs], imm, WORD);
    if(!result) {
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID BYTESTORE PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm<<endl;
        c.errors.push_back(oss.str());
    }
}

//branch instructions (goes in imm_instructions)
void beq(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
void bgtz(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
void blez(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
void bne(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);

//jump instructions (goes in jmp_instructions)
void jmp(CPU &c, uint8_t rs, int32_t imm);
void jal(CPU &c, uint8_t rs, int32_t imm);
void jalr(CPU &c, uint8_t rs, int32_t imm);
void jr(CPU &c, uint8_t rs, int32_t imm);
void trap(CPU &c, uint8_t rs, int32_t imm);

//misc instructions (depends)
void lhi(CPU &c, uint8_t rs, uint8_t rt, int16_t imm); //imm_instructions
void llo(CPU &c, uint8_t rs, uint8_t rt, int16_t imm); //imm_instructions
void mfhi(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt); //reg_instructions
void mflo(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt); //reg_instructions
void mthi(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt); //reg_instructions
void mtlo(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt); //reg_instructions

string CPU::lookup(uint32_t inst) {}
string CPU::lookup_current() {}

void CPU::run() {}
void CPU::cap(uint32_t freq) {}
void CPU::set_stop_on_error(bool yn) {}
void CPU::reset() {}
vector<string> CPU::get_errors() {}
void CPU::clear_error() {}

void CPU::execute() {}
bool CPU::load() {}
bool CPU::load_at(uint32_t inst) {}
uint32_t CPU::get_pc() {}
void CPU::set_register(uint8_t reg, int32_t val) {}
void CPU::set_breakpoint(uint32_t location) {}
void CPU::remove_breakpoint(uint32_t location) {}

CPU::CPU(Memory *m) {}
CPU::~CPU() {}