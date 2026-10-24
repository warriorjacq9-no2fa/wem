#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>

struct timer {
    unsigned int c;
    bool done;
};

void timer_start(struct timer* t, unsigned int ticks);
void timer_cancel(struct timer* t);
bool timer_wait(struct timer* t);

#endif