#include "../include/emulator.h"

#include <stdio.h>

int main(void) {
    uint8_t ports[2 * PORTS_LEN];
    uint8_t *ports_in[PORTS_LEN], *ports_out[PORTS_LEN];
    for(size_t i = 0; i < PORTS_LEN; ++i) {
        ports_in[i] = &ports[i];
        ports_out[i] = &ports[i + PORTS_LEN];
    }

    printf("Enter two numbers to sum: ");
    int a, b; scanf("%d %d", &a, &b);
    *ports_in[0] = (uint8_t)a;
    *ports_in[1] = (uint8_t)b;

    uint16_t program[MAX_PROGRAM_LEN] = {
        0b1111100100000000, // PLD r1, p0
        0b1111101000000001, // PLD r2, p1
        0b0100000100100010, // ADD r1, r2, r1
        0b1111000100000000, // PST r1, p0
        0b0000100000000000, // HLT
    };

    struct pi_emulator_t emulator;
    pi_emulator_init(&emulator);
    pi_emulator_load_ports(&emulator, ports_in, ports_out);
    pi_emulator_load_program(&emulator, program);

    pi_emulator_execute(&emulator);

    printf("%d + %d = %d\n", a, b, (int)*ports_out[0]);

    return 0;
}
