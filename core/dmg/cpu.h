#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

enum cpu_state {
    S_RUN,
    S_WAIT,
    S_STOP,
    S_HALT,
};

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
struct registers {
    union {
        struct { uint8_t c, b; };
        uint16_t bc;
    };
    union {
        struct { uint8_t e, d; };
        uint16_t de;
    };
    union {
        struct { uint8_t l, h; };
        uint16_t hl;
    };
    union {
        struct { uint8_t f, a; };
        uint16_t af;
    };
    uint16_t sp, pc;
    bool ime;
    enum cpu_state state;
};
#else
struct registers {
    union {
        struct { uint8_t b, c; };
        uint16_t bc;
    };
    union {
        struct { uint8_t d, e; };
        uint16_t de;
    };
    union {
        struct { uint8_t h, l; };
        uint16_t hl;
    };
    union {
        struct { uint8_t a, f; };
        uint16_t af;
    };
    uint16_t sp, pc;
    bool ime;
    enum cpu_state state;
};
#endif

#define FLAG_Z 0b10000000
#define FLAG_N 0b01000000
#define FLAG_H 0b00100000
#define FLAG_C 0b00010000

void run_cpu(struct registers* regs);

#endif