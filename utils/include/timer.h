#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>

struct timer {
    unsigned int c;
};

void timer_start(struct timer* t, unsigned int ticks);
bool timer_wait(struct timer* t);

#endif