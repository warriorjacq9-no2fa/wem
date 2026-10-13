#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "core/dmg/cpu.h"

int main(int argc, char** argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }
    FILE *fp = fopen(argv[1], "rb");
    if(fp == NULL) {
        char msg[64];
        snprintf(msg, 64, "failed to open %s", argv[1]);
        perror(msg);
        return 1;
    }

    uint8_t *data = malloc(MEM_SZ);
    size_t res = fread(data, 1, ROM_SZ, fp);
    if(res < ROM_SZ) {
        fprintf(stderr, "ROM must be at least %d bytes\n", ROM_SZ);
        return 1;
    }
    struct registers regs;
    run(data, &regs);
    return 0;
}