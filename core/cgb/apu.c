#include <stdint.h>
#include <timer.h>

uint8_t
    reg_nr50, reg_nr51, reg_nr52,
    reg_nr10, reg_nr11, reg_nr12, reg_nr13, reg_nr14,
    reg_nr21, reg_nr22, reg_nr23, reg_nr24,
    reg_nr30, reg_nr31, reg_nr32, reg_nr33, reg_nr34,
    reg_nr41, reg_nr42, reg_nr43, reg_nr44;
uint8_t waveram[16];

struct timer ch1_freq_timer, ch1_sw_timer, ch1_vol_timer, ch1_len_timer;
uint8_t ch1_step, ch1_vol, ch1_shadow, ch1_len;
bool ch1_sw_en;

void trigger_ch1() {
    reg_nr52 |= 0x01;
    if(timer_wait(&ch1_len_timer)) timer_start(&ch1_len_timer, reg_nr11 & 0x3F);

    uint16_t freq = reg_nr13 | (reg_nr14 & 0x07) << 8;
    timer_start(&ch1_freq_timer, (2048 - freq) * 4);

    uint8_t vol_pace = reg_nr12 & 0x07;
    timer_start(&ch1_vol_timer, 65536 * (vol_pace ? vol_pace : 8));

    ch1_vol = (reg_nr12 >> 4) & 0x0F;
    ch1_step = 0;

    ch1_shadow = freq;
    ch1_sw_en = reg_nr10 & 0x77;
    if(ch1_sw_en) {
        uint8_t step = reg_nr10 & 0x07;
        uint16_t freq_next = ch1_shadow + ((reg_nr10 & 0x08) ? -1 : 1) * (ch1_shadow / (2 * step));
        if(freq_next > 0x07FF) reg_nr52 &= ~0x01;
    }

    ch1_len = reg_nr11 & 0x3F;
}

uint8_t run_ch1() {
    if(!(reg_nr52 & 0x01)) return 0;

    // Volume sweep
    uint8_t vol_pace = reg_nr12 & 0x07;
    if(timer_wait(&ch1_vol_timer)) {
        timer_start(&ch1_vol_timer, 65536 * (vol_pace ? vol_pace : 8));
        uint8_t v = (reg_nr12 & 0x08) ? 1 : -1;
        if(vol_pace && ch1_vol + v >= 0 && ch1_vol + v < 16)
            ch1_vol += v;
    }

    uint16_t freq = reg_nr13 | (reg_nr14 & 0x07) << 8;

    // Freq sweep
    uint8_t sw_pace = (reg_nr10 >> 4) & 0x07;
    if(ch1_sw_en && sw_pace != 0 && timer_wait(&ch1_sw_timer)) {
        timer_start(&ch1_sw_timer, 32768 * sw_pace);
        uint8_t step = reg_nr10 & 0x07;
        uint16_t freq_next = ch1_shadow + ((reg_nr10 & 0x08) ? -1 : 1) * (ch1_shadow / (2 * step));
        if(freq_next > 0x07FF) reg_nr52 &= ~0x01;
        else {
            ch1_shadow = freq_next;
            freq = freq_next;
            reg_nr13 = freq_next & 0xFF;
            reg_nr14 &= ~0x07;
            reg_nr14 |= (freq_next >> 8) & 0x07;

            uint16_t freq_next = ch1_shadow + ((reg_nr10 & 0x08) ? -1 : 1) * (ch1_shadow / (2 * step));
            if(freq_next > 0x07FF) reg_nr52 &= ~0x01;
        }
    }

    // Length timer
    if(timer_wait(&ch1_len_timer)) {
        timer_start(&ch1_len_timer, 16384);
        if(--ch1_len == 0) reg_nr52 &= ~0x01;
    }

    // Wave gen
    if(timer_wait(&ch1_freq_timer)) {
        timer_start(&ch1_freq_timer, (2048 - freq) * 4);

        uint8_t waveform;
        switch(reg_nr11 & 0xC0) {
            case 0x00: waveform = 0b00000001; break;
            case 0x40: waveform = 0b10000001; break;
            case 0x80: waveform = 0b00111100; break;
            case 0xC0: waveform = 0b01111110; break;
        }
        if(++ch1_step >= 8) ch1_step = 0;
        return ((waveform & (1 << ch1_step)) >> ch1_step) ? ch1_vol : 0;
    }
    return 0;
}

int8_t run_ch1_dac(uint8_t ch1_res) {
    if(reg_nr12 & 0xF8) {
        return 127 - (254 * ch1_res) / 15;
    }
    return 0;
}

void init_apu() {

}

void run_apu(void (*do_sample)(int8_t)) {
    if(reg_nr14 & 0x80) {
        reg_nr14 &= ~0x80;
        trigger_ch1();
    }
    int8_t ch1_sample = run_ch1_dac(run_ch1());

    do_sample(ch1_sample);
}