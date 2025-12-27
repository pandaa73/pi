#ifndef __PANDAA73_PI_EMULATOR_H
#define __PANDAA73_PI_EMULATOR_H

#include <stdint.h>

#define REGISTERS_LEN   8
#define PORTS_LEN       8
#define MEMORY_LEN      256
#define CALLSTACK_LEN   32
#define MAX_PROGRAM_LEN 2048

enum pi_flag_t {
    PIFLG_ZERO = 0x01,
    PIFLG_MSB  = 0x02,
    PIFLG_BIT7 = 0x04,
    PIFLG_LSB  = 0x08,
    PIFLG_HLT  = 0x80,
};

enum pi_branch_condition_t {
    PICND_BEQ  = 0b000,
    PICND_BNE  = 0b001,
    PICND_POS  = 0b010,
    PICND_NEG  = 0b011,
    PICND_PEQ  = 0b100,
    PICND_NEQ  = 0b101,
    PICND_EVN  = 0b110,
    PICND_SOF  = 0b111,
    PICND_SIZE
};

enum pi_opcode_t {
    PIOP_NOP  = 0b00000,
    PIOP_HLT  = 0b00001,
    PIOP_JMP  = 0b00010,
    PIOP_BRH  = 0b00011,
    PIOP_CALL = 0b00100,
    PIOP_RET  = 0b00101,
    PIOP_LDI  = 0b00110,
    PIOP_MOV  = 0b00111,
    PIOP_ADD  = 0b01000,
    PIOP_SUB  = 0b01001,
    PIOP_ADDI = 0b01010,
    PIOP_ADSI = 0b01011,
    PIOP_XOR  = 0b01100,
    PIOP_AND  = 0b01101,
    PIOP_OR   = 0b01110,
    PIOP_CMP  = 0b01111,
    PIOP_XORI = 0b10000,
    PIOP_ANDI = 0b10001,
    PIOP_ORI  = 0b10010,
    PIOP_CMPI = 0b10011,
    PIOP_RSH  = 0b10100,
    PIOP_LSH  = 0b10101,
    PIOP_RTL  = 0b10110,
    PIOP_ARS  = 0b10111,
    PIOP_RSHI = 0b11000,
    PIOP_LSHI = 0b11001,
    PIOP_RTLI = 0b11010,
    PIOP_ARSI = 0b11011,
    PIOP_MST  = 0b11100,
    PIOP_MLD  = 0b11101,
    PIOP_PST  = 0b11110,
    PIOP_PLD  = 0b11111,
    PIOP_SIZE
};

struct pi_emulator_t {
    uint8_t flags;
    uint8_t regs[REGISTERS_LEN];

    uint8_t mem[MEMORY_LEN];
    uint8_t *ports_in[PORTS_LEN];
    uint8_t *ports_out[PORTS_LEN];

    uint16_t callstack_ptr;
    uint16_t callstack[CALLSTACK_LEN];

    uint16_t inst_ptr;
    uint16_t program[MAX_PROGRAM_LEN];

    int urandom_fd;
};

void pi_emulator_init(struct pi_emulator_t *emulator);
void pi_emulator_load_ports(
    struct pi_emulator_t *emulator,
    uint8_t *ports_in[PORTS_LEN],
    uint8_t *ports_out[PORTS_LEN]
);

void pi_emulator_load_program(
    struct pi_emulator_t *emulator,
    const uint16_t program[MAX_PROGRAM_LEN]
);

void pi_emulator_step(struct pi_emulator_t *emulator);
void pi_emulator_execute(struct pi_emulator_t *emulator);

#endif /* __PANDAA73_PI_EMULATOR_H */
