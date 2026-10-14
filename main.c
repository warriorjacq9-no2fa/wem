#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <core.h>
#include <graphics.h>

uint32_t pixels[160*144];

void* emu_thread(void* arg) {
    uint8_t *data = malloc(ROM_SZ);
    if(data == NULL) {
        perror("Failed to allocate ROM buffer");
        return NULL;
    }
    size_t res = fread(data, 1, ROM_SZ, (FILE*)arg);
    if(res < ROM_SZ) {
        fprintf(stderr, "ROM must be at least %d bytes\n", ROM_SZ);
        return NULL;
    }
    load_rom(data);
    struct registers regs;
    poweron(&regs);
    run(&regs);
    return NULL;
}

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

    for(int y = 0; y < 144; y++) {
        for(int x = 0; x < 160; x++) {
            uint8_t r = x;
            uint8_t g = y;
            uint8_t b = 128;

            pixels[y * 160 + x] = r | (g << 8) | (b << 16) | 0xFF000000;
        }
    }

    pthread_t thread;

    printf("Creating emulation thread...\n");
    if(pthread_create(&thread, NULL, emu_thread, fp) != 0) {
        perror("Failed to create emulation thread");
        return 1;
    }

    printf("Initializing graphics...\n");
    graphics_run(160, 144, pixels);
    pthread_kill(thread, SIGKILL);
    pthread_join(thread, NULL);
    return 0;
}