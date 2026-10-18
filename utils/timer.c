#include <timer.h>

void timer_start(struct timer* t, unsigned int ticks) {
    t->c = ticks;
}

bool timer_wait(struct timer* t) {
    if(t->c == 0) return true;
    t->c--;
    return false;
}