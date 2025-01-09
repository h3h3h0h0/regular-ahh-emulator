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
    protected: //to make extensions of the instruction set
    map<uint8_t, void(*)(CPU&, uint8_t, uint8_t, uint8_t, uint8_t)> reg_instructions; //the 4th is for the shift value
    map<uint8_t, void(*)(CPU&, uint8_t, uint8_t, int16_t)> imm_instructions;
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
    uint32_t frequency_cap;

    public:
    string lookup(uint32_t inst);
    string lookup_current();

    void run();
    void cap(uint32_t freq); //cap at 0 means no frequency cap
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
    friend void add(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void addu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void sub(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void subu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void mult(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a); //NOTE: RD IS UNUSED IN MULT AND DIV!
    friend void multu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void div(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void divu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void aand(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void nor(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void oor(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void sllv(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void srav(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void srlv(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void xxor(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void sll(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void sra(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
    friend void srl(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);

    //math with immediates (goes in imm_instructions)
    friend void addi(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
    friend void addiu(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
    friend void andi(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
    friend void ori(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
    friend void xori(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);

    //memory load and store operations (goes in imm_instructions)
    friend void lb(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
    friend void lbu(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
    friend void lh(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
    friend void lhu(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
    friend void lw(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
    friend void sb(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
    friend void sh(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
    friend void sw(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);

    //branch instructions (goes in imm_instructions)
    friend void beq(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
    friend void bgtz(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
    friend void blez(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
    friend void bne(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);

    //jump instructions (goes in jmp_instructions)
    friend void jmp(CPU &c, uint8_t rs, int32_t imm);
    friend void jal(CPU &c, uint8_t rs, int32_t imm);
    friend void jalr(CPU &c, uint8_t rs, int32_t imm);
    friend void jr(CPU &c, uint8_t rs, int32_t imm);
    friend void trap(CPU &c, uint8_t rs, int32_t imm);

    //misc instructions (depends)
    friend void lhi(CPU &c, uint8_t rs, uint8_t rt, int16_t imm); //imm_instructions
    friend void llo(CPU &c, uint8_t rs, uint8_t rt, int16_t imm); //imm_instructions
    friend void mfhi(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt); //reg_instructions
    friend void mflo(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt); //reg_instructions
    friend void mthi(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt); //reg_instructions
    friend void mtlo(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt); //reg_instructions
};

//actual function decls
//register-only math (goes in reg_instructions)
void add(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
void addu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
void sub(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
void subu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
void mult(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a); //NOTE: RD IS UNUSED IN MULT AND DIV!
void multu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
void div(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
void divu(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
void aand(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
void nor(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
void oor(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
void sllv(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
void srav(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
void srlv(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
void xxor(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
void sll(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a); //these 3 are the only ones using the a parameter, as that is the shift
void sra(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);
void srl(CPU &c, uint8_t rd, uint8_t rs, uint8_t rt, uint8_t a);

//math with immediates (goes in imm_instructions)
void addi(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
void addiu(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
void andi(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
void ori(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
void xori(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);

//memory load and store operations (goes in imm_instructions)
void lb(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
void lbu(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
void lh(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
void lhu(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
void lw(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
void sb(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
void sh(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);
void sw(CPU &c, uint8_t rs, uint8_t rt, int16_t imm);

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