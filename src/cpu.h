#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include <cstdint>
#include <set>
#include "memory.h"

using namespace std;

enum RunState {
    READY,
    RUNNING,
    ERROR
};

class CPU {
    map<uint8_t, void(*)(CPU&, uint8_t, uint8_t, uint8_t)> reg_instructions;
    map<uint8_t, void(*)(CPU&, uint8_t, uint8_t, int32_t)> imm_instructions;
    map<uint8_t, void(*)(CPU&, uint8_t, int32_t)> jmp_instructions;

    map<uint8_t, string> reg_disassemblies; //instruction names (for debug)
    map<uint8_t, string> root_disassemblies;
    set<uint32_t> breakpoints; //run() will stop at these

    map<int32_t, void(*)(CPU&)> syscalls; //system calls that happen when you call the trap instruction

    Memory *mem;

    int32_t regs[32];
    int32_t hi, lo;

    uint32_t pc;
    RunState state;

    public:
    string lookup(uint32_t inst);
    string lookup_current();

    void run();
    void run_until_opcode(uint32_t oc);
    void reset();

    void execute(); //execute next instruction
    bool load(); //load instruction @ PC
    bool load_at(uint32_t inst); //load from a spot in memory (bool to track if we actually loaded a valid address)
    uint32_t get_pc(); //what is the current program counter?
    void set_register(uint8_t reg, int32_t val); //can be used for debug, also used by instructions to set registers
    void set_breakpoint(uint32_t location);
    void remove_breakpoint(uint32_t location);

    CPU(Memory *m);
    ~CPU();

    //declare all instruction functions as friend
    //register-only math (goes in reg_instructions)
    friend void add(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt);
    friend void addu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt);
    friend void sub(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt);
    friend void subu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt);
    friend void mult(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt); //NOTE: RD IS UNUSED IN MULT AND DIV!
    friend void multu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt);
    friend void div(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt);
    friend void divu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt);
    friend void aand(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt);
    friend void nor(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt);
    friend void oor(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt);
    friend void sllv(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt);
    friend void srav(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt);
    friend void srlv(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt);
    friend void xxor(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt);

    //math with immediates (goes in imm_instructions)
    friend void addi(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void addiu(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void andi(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void ori(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void xori(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void sll(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void sra(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void srl(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);

    //memory load and store operations (goes in imm_instructions)
    friend void lb(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void lbu(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void lh(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void lhu(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void lw(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void sb(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void sh(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void sw(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);

    //branch instructions (goes in imm_instructions)
    friend void beq(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void bgtz(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void blez(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);
    friend void bne(CPU &c, uint8_t rs, uint8_t rt, int32_t imm);

    //jump instructions (goes in jmp_instructions)
    friend void jmp(CPU &c, uint8_t rs, int32_t imm);
    friend void jal(CPU &c, uint8_t rs, int32_t imm);
    friend void jalr(CPU &c, uint8_t rs, int32_t imm);
    friend void jr(CPU &c, uint8_t rs, int32_t imm);
    friend void trap(CPU &c, uint8_t rs, int32_t imm);

    //misc instructions (depends)
    friend void lhi(CPU &c, uint8_t rs, uint8_t rt, int32_t imm); //imm_instructions
    friend void llo(CPU &c, uint8_t rs, uint8_t rt, int32_t imm); //imm_instructions
    friend void mfhi(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt); //reg_instructions
    friend void mflo(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt); //reg_instructions
    friend void mthi(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt); //reg_instructions
    friend void mtlo(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt); //reg_instructions
};