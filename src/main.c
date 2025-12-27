#include "../include/emulator.h"

#include <stdio.h>

uint8_t reader(void) {
    printf("PORT: Please input a number: ");

    int x; scanf("%d", &x);

    return (uint8_t)x;
}

void writer(uint8_t value) {
    printf("PORT: %d\n", (int)value);
}

int main(void) {
    struct pi_port_t ports[PORTS_LEN];
    for(size_t i = 0; i < PORTS_LEN; ++i) {
        ports[i].reader = reader;
        ports[i].writer = writer;
    }

    uint16_t program[MAX_PROGRAM_LEN] = {
        0b1111100100000000, // PLD r1, p0
        0b1111101000000001, // PLD r2, p1
        0b0100000100100010, // ADD r1, r2, r1
        0b1111000100000000, // PST r1, p0
        0b0000100000000000, // HLT
    };

    struct pi_emulator_t emulator;
    pi_emulator_init(&emulator);
    pi_emulator_load_ports(&emulator, ports);
    pi_emulator_load_program(&emulator, program);

    pi_emulator_execute(&emulator);

    return 0;
}
