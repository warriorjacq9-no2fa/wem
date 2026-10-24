#include <timer.h>

void timer_start(struct timer* t, unsigned int ticks) {
    t->done = false;
    t->c = ticks;
}

void timer_cancel(struct timer* t) {
    t->c = 0;
    t->done = true;
}

bool timer_wait(struct timer* t) {
    if(t->done) return false;
    if(t->c == 0) {
        t->done = true;
        return true;
    }
    t->c--;
    return false;
}