#include "../include/emulator.h"

#include "../include/log.h"

#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

static inline uint8_t __rsh(uint8_t x, uint8_t n) {
    uint8_t mask = (CHAR_BIT * sizeof(x) - 1);

    n &= mask;

    return x >> n;
}

static inline uint8_t __lsh(uint8_t x, uint8_t n) {
    uint8_t mask = (CHAR_BIT * sizeof(x) - 1);

    n &= mask;

    return x << n;
}

static inline uint8_t __rotl(uint8_t x, uint8_t n) {
    uint8_t mask = (CHAR_BIT * sizeof(x) - 1);

    n &= mask;

    return (x << n) | (x >> ((-n) & mask));
}

static inline int8_t __ars(int8_t x, uint8_t n) {
    uint8_t mask = (CHAR_BIT * sizeof(x) - 1);

    n &= mask;

    return x >> n;
}

static inline uint8_t gen_random_byte(struct pi_emulator_t *emulator) {
    uint8_t x;

    if(read(emulator->urandom_fd, &x, 1) != 1) {
        PLG_FATAL("Failed to read from `/dev/urandom`");
    }

    return x;
}

/* ========================================================================== */
/* ============================ Branch Conditions =========================== */
/* ========================================================================== */

static inline int beq(struct pi_emulator_t *emulator);
static inline int bne(struct pi_emulator_t *emulator);
static inline int pos(struct pi_emulator_t *emulator);
static inline int neg(struct pi_emulator_t *emulator);
static inline int peq(struct pi_emulator_t *emulator);
static inline int neq(struct pi_emulator_t *emulator);
static inline int evn(struct pi_emulator_t *emulator);
static inline int sof(struct pi_emulator_t *emulator);

int (*check[PICND_SIZE])(struct pi_emulator_t *) = {
    beq, bne, pos, neg, peq, neq, evn, sof
};

/* ========================================================================== */
/* ============================== Instructions ============================== */
/* ========================================================================== */

static inline void  nop(struct pi_emulator_t *emulator);
static inline void  hlt(struct pi_emulator_t *emulator);
static inline void  jmp(struct pi_emulator_t *emulator);
static inline void  brh(struct pi_emulator_t *emulator);
static inline void call(struct pi_emulator_t *emulator);
static inline void  ret(struct pi_emulator_t *emulator);
static inline void  ldi(struct pi_emulator_t *emulator);
static inline void  mov(struct pi_emulator_t *emulator);
static inline void  add(struct pi_emulator_t *emulator);
static inline void  sub(struct pi_emulator_t *emulator);
static inline void addi(struct pi_emulator_t *emulator);
static inline void adsi(struct pi_emulator_t *emulator);
static inline void  xor(struct pi_emulator_t *emulator);
static inline void  and(struct pi_emulator_t *emulator);
static inline void   or(struct pi_emulator_t *emulator);
static inline void  cmp(struct pi_emulator_t *emulator);
static inline void xori(struct pi_emulator_t *emulator);
static inline void andi(struct pi_emulator_t *emulator);
static inline void  ori(struct pi_emulator_t *emulator);
static inline void cmpi(struct pi_emulator_t *emulator);
static inline void  rsh(struct pi_emulator_t *emulator);
static inline void  lsh(struct pi_emulator_t *emulator);
static inline void  rtl(struct pi_emulator_t *emulator);
static inline void  ars(struct pi_emulator_t *emulator);
static inline void rshi(struct pi_emulator_t *emulator);
static inline void lshi(struct pi_emulator_t *emulator);
static inline void rtli(struct pi_emulator_t *emulator);
static inline void arsi(struct pi_emulator_t *emulator);
static inline void  mst(struct pi_emulator_t *emulator);
static inline void  mld(struct pi_emulator_t *emulator);
static inline void  pst(struct pi_emulator_t *emulator);
static inline void  pld(struct pi_emulator_t *emulator);

void (*execute[PIOP_SIZE])(struct pi_emulator_t *) = {
     nop,  hlt,  jmp,  brh, call,  ret,  ldi,  mov,
     add,  sub, addi, adsi,  xor,  and,   or,  cmp,
    xori, andi,  ori, cmpi,  rsh,  lsh,  rtl,  ars,
    rshi, lshi, rtli, arsi,  mst,  mld,  pst,  pld,
};

/* ========================================================================== */
/* =========================== Emulator Functions =========================== */
/* ========================================================================== */

void pi_emulator_init(struct pi_emulator_t *emulator) {
    if(!emulator) { PLG_FATAL("init: emulator is NULL"); }

    memset(emulator, 0x00, sizeof(*emulator));

    emulator->urandom_fd = open("/dev/urandom", O_RDONLY);
    if(emulator->urandom_fd < 0) {
        PLG_FATAL("init: failed to open `/dev/urandom`");
    }
}

void pi_emulator_load_ports(
    struct pi_emulator_t *emulator,
    struct pi_port_t ports[PORTS_LEN]
) {
    if(!emulator) { PLG_FATAL("load_ports: emulator is NULL"); }
    if(!ports)    { PLG_FATAL("load_ports: ports_in is NULL"); }

    for(uint16_t i = 0; i < PORTS_LEN; ++i) {
        emulator->ports[i] = ports[i];
    }
}

void pi_emulator_load_program(
    struct pi_emulator_t *emulator,
    const uint16_t program[MAX_PROGRAM_LEN]
) {
    if(!emulator) { PLG_FATAL("load_program: emulator is NULL"); }
    if(!program)  { PLG_FATAL("load_program: program is NULL"); }

    for(uint16_t i = 0; i < MAX_PROGRAM_LEN; ++i) {
        emulator->program[i] = program[i];
    }
}

static inline void unsafe_pi_emulator_step(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    /*
     * Note that the opcode must be valid (i.e. opcode < PI_SIZE) since it's a
     * 5-bit number
     */
    const enum pi_opcode_t opcode = instruction >> 11;

    execute[opcode](emulator);

    emulator->inst_ptr = (emulator->inst_ptr + 1) % MAX_PROGRAM_LEN;
}

inline void pi_emulator_step(struct pi_emulator_t *emulator) {
    if(!emulator) { PLG_FATAL("step: emulator is NULL"); }

    unsafe_pi_emulator_step(emulator);
}

void pi_emulator_execute(struct pi_emulator_t *emulator) {
    if(!emulator) { PLG_FATAL("execute: emulator is NULL"); }

    while((emulator->flags & PIFLG_HLT) == 0) {
        unsafe_pi_emulator_step(emulator);
    }
}

/* ========================================================================== */
/* ============================ Branch Conditions =========================== */
/* ========================================================================== */

static inline int beq(struct pi_emulator_t *emulator) {
    return (emulator->flags & PIFLG_ZERO) != 0;
}

static inline int bne(struct pi_emulator_t *emulator) {
    return (emulator->flags & PIFLG_ZERO) == 0;
}

static inline int pos(struct pi_emulator_t *emulator) {
    return (emulator->flags & (PIFLG_ZERO | PIFLG_MSB)) == 0;
}

static inline int neg(struct pi_emulator_t *emulator) {
    return (emulator->flags & PIFLG_MSB) != 0;
}

static inline int peq(struct pi_emulator_t *emulator) {
    return (emulator->flags & PIFLG_MSB) == 0;
}

static inline int neq(struct pi_emulator_t *emulator) {
    return (emulator->flags & (PIFLG_ZERO | PIFLG_MSB)) != 0;
}

static inline int evn(struct pi_emulator_t *emulator) {
    return (emulator->flags & PIFLG_LSB) == 0;
}

static inline int sof(struct pi_emulator_t *emulator) {
    return ((emulator->flags & PIFLG_MSB) != 0)
        ^ ((emulator->flags & PIFLG_BIT7));
}

/* ========================================================================== */
/* ============================== Instructions ============================== */
/* ========================================================================== */

static inline void  nop(struct pi_emulator_t *emulator) {
    (void)emulator;
}

static inline void  hlt(struct pi_emulator_t *emulator) {
    emulator->flags |= PIFLG_HLT;
}

static inline void  jmp(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    emulator->inst_ptr = instruction & 0x07FF;
}

static inline void  brh(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const enum pi_branch_condition_t cond = (instruction >> 8) & 0x07;
    const uint8_t address = instruction & 0xFF;

    if(!check[cond](emulator)) return;

    emulator->inst_ptr = (emulator->inst_ptr & 0x700) | address;
}

static inline void call(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    emulator->callstack[emulator->callstack_ptr] = emulator->inst_ptr;

    emulator->callstack_ptr = (emulator->callstack_ptr + 1) % CALLSTACK_LEN;

    emulator->inst_ptr = instruction & 0x07FF;
}

static inline void  ret(struct pi_emulator_t *emulator) {
    emulator->inst_ptr = emulator->callstack[emulator->callstack_ptr];

    emulator->callstack_ptr =
        (emulator->callstack_ptr + CALLSTACK_LEN - 1) % CALLSTACK_LEN;
}

static inline void  ldi(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A   = (instruction >> 8) & 0x07;
    const uint8_t imm = (instruction >> 0) & 0xFF;

    if(A != 0) { emulator->regs[A] = imm; }
}

static inline void  mov(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint16_t B = (instruction >> 0) & 0x07;

    if(A != 0) { emulator->regs[A] = emulator->regs[B]; }
}

static inline void  add(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint16_t C = (instruction >> 5) & 0x07;
    const uint16_t B = (instruction >> 0) & 0x07;

    if(C != 0) { emulator->regs[C] = emulator->regs[A] + emulator->regs[B]; }
}

static inline void  sub(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint16_t C = (instruction >> 5) & 0x07;
    const uint16_t B = (instruction >> 0) & 0x07;

    if(C != 0) { emulator->regs[C] = emulator->regs[A] - emulator->regs[B]; }
}

static inline void addi(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A   = (instruction >> 8) & 0x07;
    const uint8_t imm = (instruction >> 0) & 0xFF;

    if(A != 0) { emulator->regs[A] += imm; }
}

static inline void adsi(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint16_t C = (instruction >> 5) & 0x07;
    uint8_t sximm   = (instruction >> 0) & 0x1F;

    sximm |= (sximm & 0x10) << 1;
    sximm |= (sximm & 0x10) << 2;
    sximm |= (sximm & 0x10) << 3;

    if(A != 0) { emulator->regs[C] = emulator->regs[A] + sximm; }
}

static inline void  xor(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint16_t C = (instruction >> 5) & 0x07;
    const uint16_t B = (instruction >> 0) & 0x07;

    if(C != 0) { emulator->regs[C] = emulator->regs[A] ^ emulator->regs[B]; }
}

static inline void  and(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint16_t C = (instruction >> 5) & 0x07;
    const uint16_t B = (instruction >> 0) & 0x07;

    if(C != 0) { emulator->regs[C] = emulator->regs[A] & emulator->regs[B]; }
}

static inline void   or(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint16_t C = (instruction >> 5) & 0x07;
    const uint16_t B = (instruction >> 0) & 0x07;

    if(C != 0) { emulator->regs[C] = emulator->regs[A] | emulator->regs[B]; }
}

static inline void  cmp(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint16_t B = (instruction >> 0) & 0x07;

    uint8_t diff = emulator->regs[A] - emulator->regs[B];

    emulator->flags = 0; /* CPU shouldn't be halted at this stage */
    if(         diff == 0) { emulator->flags |= PIFLG_ZERO; }
    if((diff & 0x80) != 0) { emulator->flags |= PIFLG_MSB; }
    if((diff & 0x01) != 0) { emulator->flags |= PIFLG_LSB; }
    if((diff & 0x40) != 0) { emulator->flags |= PIFLG_BIT7; }
}

static inline void xori(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A   = (instruction >> 8) & 0x07;
    const uint8_t imm = (instruction >> 0) & 0xFF;

    if(A != 0) { emulator->regs[A] ^= imm; }
}

static inline void andi(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A   = (instruction >> 8) & 0x07;
    const uint8_t imm = (instruction >> 0) & 0xFF;

    if(A != 0) { emulator->regs[A] &= imm; }
}

static inline void  ori(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A   = (instruction >> 8) & 0x07;
    const uint8_t imm = (instruction >> 0) & 0xFF;

    if(A != 0) { emulator->regs[A] |= imm; }
}

static inline void cmpi(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A   = (instruction >> 8) & 0x07;
    const uint8_t imm = (instruction >> 0) & 0xFF;

    uint8_t diff = emulator->regs[A] - imm;

    emulator->flags = 0; /* CPU shouldn't be halted at this stage */
    if(         diff == 0) { emulator->flags |= PIFLG_ZERO; }
    if((diff & 0x80) != 0) { emulator->flags |= PIFLG_MSB; }
    if((diff & 0x40) != 0) { emulator->flags |= PIFLG_BIT7; }
    if((diff & 0x01) != 0) { emulator->flags |= PIFLG_LSB; }
}

static inline void  rsh(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint16_t C = (instruction >> 5) & 0x07;
    const uint16_t B = (instruction >> 0) & 0x07;

    if(C != 0) {
        emulator->regs[C] = __rsh(
            emulator->regs[A],
            -emulator->regs[B]
        );
    }
}

static inline void  lsh(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint16_t C = (instruction >> 5) & 0x07;
    const uint16_t B = (instruction >> 0) & 0x07;

    if(C != 0) {
        emulator->regs[C] = __lsh(
            emulator->regs[A],
            emulator->regs[B]
        );
    }
}

static inline void  rtl(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint16_t C = (instruction >> 5) & 0x07;
    const uint16_t B = (instruction >> 0) & 0x07;

    if(C != 0) {
        emulator->regs[C] = __rotl(
            emulator->regs[A],
            emulator->regs[B]
        );
    }
}

static inline void  ars(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint16_t C = (instruction >> 5) & 0x07;
    const uint16_t B = (instruction >> 0) & 0x07;

    /*
     * Immediate might have to be negative although it is not marked as such
     * in the ISA. I am not sure.
     */

    if(C != 0) {
        emulator->regs[C] = __ars(
            emulator->regs[A],
            emulator->regs[B]
        );
    }
}

static inline void rshi(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint8_t imm = (instruction >> 0) & 0x07;

    if(A != 0) {
        emulator->regs[A] = __rsh(
            emulator->regs[A],
            -imm
        );
    }
}

static inline void lshi(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint8_t imm = (instruction >> 0) & 0x07;

    if(A != 0) {
        emulator->regs[A] = __lsh(
            emulator->regs[A],
            imm
        );
    }
}

static inline void rtli(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint8_t imm = (instruction >> 0) & 0x07;

    if(A != 0) {
        emulator->regs[A] = __rotl(
            emulator->regs[A],
            imm
        );
    }
}

static inline void arsi(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint8_t imm = (instruction >> 0) & 0x07;

    /*
     * Immediate might have to be negative although it is not marked as such
     * in the ISA. I am not sure.
     */

    if(A != 0) {
        emulator->regs[A] = __ars(
            emulator->regs[A],
            imm
        );
    }
}

static inline void  mst(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint16_t B = (instruction >> 0) & 0x07;

    emulator->mem[emulator->regs[B]] = emulator->regs[A];
}

static inline void  mld(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A = (instruction >> 8) & 0x07;
    const uint16_t B = (instruction >> 0) & 0x07;

    emulator->regs[A] = emulator->mem[emulator->regs[B]];
}

static inline void  pst(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A    = (instruction >> 8) & 0x07;
    const uint16_t port = (instruction >> 0) & 0x07;

    if(emulator->ports[port].writer != NULL) {
        emulator->ports[port].writer(emulator->regs[A]);
    }
}

static inline void  pld(struct pi_emulator_t *emulator) {
    const uint16_t instruction = emulator->program[emulator->inst_ptr];

    const uint16_t A    = (instruction >> 8) & 0x07;
    const uint16_t port = (instruction >> 0) & 0x07;

    if(((instruction >> 3) & 0x01) != 0) {
        emulator->regs[A] = gen_random_byte(emulator);
    } else if(emulator->ports[port].reader != NULL) {
        emulator->regs[A] = emulator->ports[port].reader();
    }
}
