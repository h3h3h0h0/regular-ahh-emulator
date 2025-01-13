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

//LHI and LLO instruction masks
#define HIMASK 0b11111111111111110000000000000000
#define LOMASK 0b00000000000000001111111111111111

#define HSHIFT 16

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
        oss<<hex<<uppercase<<"INVALID BYTEREAD PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm;
        c.errors.push_back(oss.str());
    }
}
void lbu(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    bool result = c.mem->load(c.regs[rt], c.regs[rs], imm, BYTE, true);
    if(!result) {
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID UBYTEREAD PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm;
        c.errors.push_back(oss.str());
    }
}
void lh(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    bool result = c.mem->load(c.regs[rt], c.regs[rs], imm, HALF);
    if(!result) {
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID HALFREAD PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm;
        c.errors.push_back(oss.str());
    }
}
void lhu(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    bool result = c.mem->load(c.regs[rt], c.regs[rs], imm, HALF, true);
    if(!result) {
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID UHALFREAD PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm;
        c.errors.push_back(oss.str());
    }
}
void lw(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    bool result = c.mem->load(c.regs[rt], c.regs[rs], imm, WORD, true);
    if(!result) {
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID WORDREAD PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm;
        c.errors.push_back(oss.str());
    }
}
void sb(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    bool result = c.mem->store(c.regs[rt], c.regs[rs], imm, BYTE);
    if(!result) {
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID BYTESTORE PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm;
        c.errors.push_back(oss.str());
    }
}
void sh(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    bool result = c.mem->store(c.regs[rt], c.regs[rs], imm, HALF);
    if(!result) {
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID BYTESTORE PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm;
        c.errors.push_back(oss.str());
    }
}
void sw(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    bool result = c.mem->store(c.regs[rt], c.regs[rs], imm, WORD);
    if(!result) {
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID BYTESTORE PC: "<<c.pc<<" BASE: "<<c.regs[rs]<<" OFFSET: "<<imm;
        c.errors.push_back(oss.str());
    }
}

//branch instructions (goes in imm_instructions)
void beq(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    if(c.regs[rs] == c.regs[rt]) {
        c.pc += (imm<<2);
    }
}
void bgtz(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    if(c.regs[rs] > 0) {
        c.pc += (imm<<2);
    }
}
void blez(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    if(c.regs[rs] <= 0) {
        c.pc += (imm<<2);
    }
}
void bne(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    if(c.regs[rs] != c.regs[rt]) {
        c.pc += (imm<<2);
    }
}

//jump instructions (goes in jmp_instructions)
void jmp(CPU &c, int32_t imm) {
    c.pc += (imm<<2);
}
void jal(CPU &c, int32_t imm) {
    c.regs[31] = c.pc;
    c.pc += (imm<<2);
}
void trap(CPU &c, int32_t imm) {
    if(!c.syscalls.count(imm)) { //do we have an actual syscall registered?
        c.state = ERROR;
        ostringstream oss;
        oss<<hex<<uppercase<<"INVALID SYSCALL PC: "<<c.pc<<" NUMBER: "<<imm;
        c.errors.push_back(oss.str());
    }
    c.syscalls[imm](c);
}

//jump-register instructions (goes in reg_instructions)
void jalr(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.regs[31] = c.pc;
    c.pc = c.regs[rs];
}
void jr(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a) {
    c.pc = c.regs[rs];
}

//misc instructions (depends)
void lhi(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    c.regs[rt] = (c.regs[rt]&LOMASK)+(((int32_t)imm)<<HSHIFT);
}
void llo(CPU &c, uint8_t rs, uint8_t rt, int16_t imm) {
    c.regs[rt] = (c.regs[rt]&HIMASK)+imm;
}
void mfhi(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt) {
    c.regs[rt] = c.hi;
}
void mflo(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt) {
    c.regs[rt] = c.lo;
}
void mthi(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt) {
    c.hi = c.regs[rt];
}
void mtlo(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt) {
    c.lo = c.regs[rt];
}

split_instruction make_split(uint32_t instr) {
    split_instruction si;
    si.o = ((uint32_t)(instr&OMASK))>>OSHIFT;
    si.s = ((uint32_t)(instr&SMASK))>>SSHIFT;
    si.t = ((uint32_t)(instr&TMASK))>>TSHIFT;
    si.d = ((uint32_t)(instr&DMASK))>>DSHIFT;
    si.a = ((uint32_t)(instr&AMASK))>>ASHIFT;
    si.f = ((uint32_t)(instr&FMASK))>>FSHIFT;
    si.i_imm = ((uint32_t)(instr&IMASK))>>ISHIFT;
    si.j_imm = ((uint32_t)(instr&JMASK))>>JSHIFT;
    return si;
}

string CPU::lookup(uint32_t inst) {
    if(inst == nop_code) return "nop"; //this is technically sll $0, $0, 0 but we make a special exception because NOP instructions are important
    split_instruction si = make_split(inst);
    ostringstream oss;
    if(si.o == 0) { //a REG instruction
        if(!reg_instructions.count(si.f)) return "INVALID";
        oss<<reg_instructions[si.f].second<<dec<<" s="<<si.s<<" t="<<si.t<<" d="<<si.d<<" a="<<si.a;
        return oss.str();
    } else { //a ROOT instruction
        if(jmp_instructions.count(si.o)) {
            oss<<jmp_instructions[si.o].second<<dec<<" imm="<<si.j_imm;
            return oss.str();
        } else if(imm_instructions.count(si.o)) {
            oss<<imm_instructions[si.f].second<<dec<<" s="<<si.s<<" t="<<si.t<<" imm="<<si.i_imm;
            return oss.str();
        } else return "INVALID";
    }
}

string CPU::lookup_current() {
    lookup(current_instruction);
}

void CPU::run() {}
void CPU::cap(uint32_t freq) {
    frequency_cap = freq;
}
void CPU::set_stop_on_error(bool yn) {}
void CPU::reset() {}
vector<string> CPU::get_errors() {
    return errors;
}
void CPU::clear_logs() {
    errors.clear();
}
void CPU::clear_state() {
    state = RUN; //lets you load the next instruction
}

void CPU::execute() {
    if(state != READY) return;
    state = RUN;
    uint32_t old_pc = pc;
    pc += 4;
    regs[0] = 0; //special property of register 0
    split_instruction si = make_split(current_instruction);
    bool valid = true;
    if(si.o == 0) { //a REG instruction
        if(!reg_instructions.count(si.f)) valid = false;
        reg_instructions[si.f].first(*this, si.d, si.s, si.t, si.a);
    } else { //a ROOT instruction
        if(jmp_instructions.count(si.o)) {
            jmp_instructions[si.o].first(*this, si.j_imm);
        } else if(imm_instructions.count(si.o)) {
            imm_instructions[si.f].first(*this, si.s, si.t, si.i_imm);
        } else valid = false;
    }
    regs[0] = 0; //special property of register 0
    if(!valid) {
        state = ERROR;
        ostringstream oss;
        oss<<"INVALID INSTRUCTION PC: "<<hex<<old_pc<<" INSTRUCTION WORD: "<<bitset<32>(current_instruction);
        errors.push_back(oss.str());
    }
}
bool CPU::load() {
    return load_at(pc);
}
bool CPU::load_at(uint32_t loc) {
    if(state != RUN) return false;
    int32_t temp = 0;
    if(!mem->load(temp, loc, 0, WORD)) {
        state = ERROR;
        return false;
    }
    //only set PC and current instruction if our memory load was successful
    pc = loc;
    current_instruction = (uint32_t)temp;
    state = READY;
    return true;
}
uint32_t CPU::get_pc() {
    return pc;
}
void CPU::set_register(uint8_t reg, int32_t val) {
    if(reg == 0) val = 0;
    if(reg >= 32) return;
    regs[reg] = val;
}
void CPU::set_breakpoint(uint32_t location) {
    breakpoints.insert(location);
}
void CPU::remove_breakpoint(uint32_t location) {
    if(breakpoints.count(location)) breakpoints.erase(location);
}

CPU::CPU(Memory *m) {}
CPU::~CPU() {}