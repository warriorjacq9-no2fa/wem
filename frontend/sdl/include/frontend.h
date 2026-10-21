#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

int frontend_run(int width, int height, uint32_t* pixels);
void do_sample(int8_t sample);

#endif