// TODO: clean up code, timer phase alignment

#include <stdint.h>
#include <timer.h>
#include <stdio.h>

uint8_t
    reg_nr50, reg_nr51, reg_nr52,
    reg_nr10, reg_nr11, reg_nr12, reg_nr13, reg_nr14,
    reg_nr21, reg_nr22, reg_nr23, reg_nr24,
    reg_nr30, reg_nr31, reg_nr32, reg_nr33, reg_nr34,
    reg_nr41, reg_nr42, reg_nr43, reg_nr44;
uint8_t waveram[16];

struct timer ch1_freq_timer, ch1_sw_timer, ch1_vol_timer, ch1_len_timer;
uint8_t ch1_step, ch1_vol, ch1_len, ch1_out;
uint16_t ch1_shadow;
bool ch1_sw_en;

struct timer ch2_freq_timer, ch2_vol_timer, ch2_len_timer;
uint8_t ch2_step, ch2_vol, ch2_len, ch2_out;

struct timer ch3_freq_timer, ch3_len_timer;
uint8_t ch3_step, ch3_wave, ch3_len, ch3_out;

void trigger_ch1() {
    if(!(reg_nr12 & 0xF8)) return;
    reg_nr52 |= 0x01;
    timer_start(&ch1_len_timer, 16384);

    uint16_t freq = reg_nr13 | (reg_nr14 & 0x07) << 8;
    timer_start(&ch1_freq_timer, (2048 - freq) * 4);

    uint8_t vol_pace = reg_nr12 & 0x07;
    timer_start(&ch1_vol_timer, 65536 * (vol_pace ? vol_pace : 8));

    uint8_t sw_pace = (reg_nr10 >> 4) & 0x07;
    timer_start(&ch1_sw_timer, 32768 * (sw_pace ? sw_pace : 8));

    ch1_vol = (reg_nr12 >> 4) & 0x0F;
    ch1_step = 0;

    ch1_shadow = freq;
    uint8_t shift = reg_nr10 & 0x07;
    ch1_sw_en = shift != 0 || sw_pace != 0;
    if(shift != 0) {
        uint16_t freq_next = (reg_nr10 & 0x08) ?
            ch1_shadow - (ch1_shadow >> shift) :
            ch1_shadow + (ch1_shadow >> shift);
        if(freq_next > 0x07FF) reg_nr52 &= ~0x01;
    }

    if(ch1_len == 0) ch1_len = 64; // TODO: check for write
#if DEBUG & 0x04
    fprintf(stderr, "CH1 triggered\n");
#endif
}

void trigger_ch2() {
    if(!(reg_nr22 & 0xF8)) return;
    reg_nr52 |= 0x02;
    timer_start(&ch2_len_timer, 16384);

    uint16_t freq = reg_nr23 | (reg_nr24 & 0x07) << 8;
    timer_start(&ch2_freq_timer, (2048 - freq) * 4);

    uint8_t vol_pace = reg_nr22 & 0x07;
    timer_start(&ch2_vol_timer, 65536 * (vol_pace ? vol_pace : 8));

    ch2_vol = (reg_nr22 >> 4) & 0x0F;
    ch2_step = 0;

    if(ch2_len == 0) ch2_len = 64; // TODO: check for write
#if DEBUG & 0x04
    fprintf(stderr, "CH2 triggered\n");
#endif
}

void trigger_ch3() {
    if(!(reg_nr30 & 0x80)) return;
    reg_nr52 |= 0x04;
    timer_start(&ch3_len_timer, 16384);

    uint16_t freq = reg_nr33 | (reg_nr34 & 0x07) << 8;
    timer_start(&ch3_freq_timer, (2048 - freq) * 4);

    ch3_step = 1;

    if(ch3_len == 0) ch3_len = 64; // TODO: check for write
#if DEBUG & 0x04
    fprintf(stderr, "CH3 triggered\n");
#endif
}

uint8_t run_ch1() {
    if(!(reg_nr52 & 0x01)) return 0;

    // Volume sweep
    uint8_t vol_pace = reg_nr12 & 0x07;
    if(timer_wait(&ch1_vol_timer)) {
        timer_start(&ch1_vol_timer, 65536 * (vol_pace ? vol_pace : 8));
        int v = (reg_nr12 & 0x08) ? ch1_vol + 1 : ch1_vol - 1;
        if(vol_pace && v >= 0 && v < 16)
            ch1_vol = (uint8_t)v;
#if DEBUG & 0x04
        fprintf(stderr, "CH1 envelope pace: %u\n", vol_pace);
#endif
    }

    uint16_t freq = reg_nr13 | (reg_nr14 & 0x07) << 8;

    // Freq sweep
    uint8_t sw_pace = (reg_nr10 >> 4) & 0x07;
    if(timer_wait(&ch1_sw_timer)) {
        timer_start(&ch1_sw_timer, 32768 * (sw_pace ? sw_pace : 8));
        if(ch1_sw_en && sw_pace != 0) {
            uint8_t shift = reg_nr10 & 0x07;
            uint16_t freq_next = (reg_nr10 & 0x08) ?
                ch1_shadow - (ch1_shadow >> shift) :
                ch1_shadow + (ch1_shadow >> shift);

            if(freq_next > 0x07FF) reg_nr52 &= ~0x01;
            else if(shift != 0) {
                ch1_shadow = freq_next;
                freq = freq_next;
                reg_nr13 = freq_next & 0xFF;
                reg_nr14 &= ~0x07;
                reg_nr14 |= (freq_next >> 8) & 0x07;

                freq_next = (reg_nr10 & 0x08) ?
                    ch1_shadow - (ch1_shadow >> shift) :
                    ch1_shadow + (ch1_shadow >> shift);
                if(freq_next > 0x07FF) reg_nr52 &= ~0x01;
            }
#if DEBUG & 0x04
            fprintf(stderr, "CH1 sweep pace: %u shift: %u freq_next: %u shadow: %u\n", sw_pace, shift, freq_next, ch1_shadow);
#endif
        }
    }

    // Length timer
    if(timer_wait(&ch1_len_timer)) {
        timer_start(&ch1_len_timer, 16384);
        if((reg_nr14 & 0x40) && ch1_len > 0) {
            if(--ch1_len == 0) reg_nr52 &= ~0x01;
#if DEBUG & 0x04
            fprintf(stderr, "CH1 len %u\n", ch1_len);
#endif
        }
    }

    // Wave gen
    if(timer_wait(&ch1_freq_timer)) {
        timer_start(&ch1_freq_timer, (2048 - freq) * 4);

        uint8_t waveform;
        switch(reg_nr11 & 0xC0) {
            case 0x00: waveform = 0b00000001; break;
            case 0x40: waveform = 0b00000011; break;
            case 0x80: waveform = 0b11110000; break;
            case 0xC0: waveform = 0b00111111; break;
        }
        if(++ch1_step >= 8) ch1_step = 0;
        if(((waveform & (1 << ch1_step)) >> ch1_step)) ch1_out = ch1_vol;
        else ch1_out = 0;
#if DEBUG & 0x04
        fprintf(stderr, "CH1 wavegen step: %u vol: %u out: %u freq: %u wave: %02x\n\n", ch1_step, ch1_vol, ch1_out, freq, reg_nr11 & 0xC0);
#endif
    }

    return ch1_out;
}

uint8_t run_ch2() {
    if(!(reg_nr52 & 0x02)) return 0;

    // Volume sweep
    uint8_t vol_pace = reg_nr22 & 0x07;
    if(timer_wait(&ch2_vol_timer)) {
        timer_start(&ch2_vol_timer, 65536 * (vol_pace ? vol_pace : 8));
        int v = (reg_nr22 & 0x08) ? ch2_vol + 1 : ch2_vol - 1;
        if(vol_pace && v >= 0 && v < 16)
            ch2_vol = (uint8_t)v;
#if DEBUG & 0x04
        fprintf(stderr, "CH2 envelope pace: %u\n", vol_pace);
#endif
    }

    // Length timer
    if(timer_wait(&ch2_len_timer)) {
        timer_start(&ch2_len_timer, 16384);
        if((reg_nr24 & 0x40) && ch2_len > 0) {
            if(--ch2_len == 0) reg_nr52 &= ~0x02;
#if DEBUG & 0x04
            fprintf(stderr, "CH2 len %u\n", ch2_len);
#endif
        }
    }

    // Wave gen
    uint16_t freq = reg_nr23 | (reg_nr24 & 0x07) << 8;
    if(timer_wait(&ch2_freq_timer)) {
        timer_start(&ch2_freq_timer, (2048 - freq) * 4);

        uint8_t waveform;
        switch(reg_nr21 & 0xC0) {
            case 0x00: waveform = 0b00000001; break;
            case 0x40: waveform = 0b00000011; break;
            case 0x80: waveform = 0b11110000; break;
            case 0xC0: waveform = 0b00111111; break;
        }
        if(++ch2_step >= 8) ch2_step = 0;
        if(((waveform & (1 << ch2_step)) >> ch2_step)) ch2_out = ch2_vol;
        else ch2_out = 0;
#if DEBUG & 0x04
        fprintf(stderr, "CH2 wavegen step: %u vol: %u out: %u freq: %u wave: %02x\n\n", ch2_step, ch2_vol, ch2_out, freq, reg_nr21 & 0xC0);
#endif
    }

    return ch2_out;
}

uint8_t run_ch3() {
    if(!(reg_nr52 & 0x04)) return 0;

    // Length timer
    if(timer_wait(&ch3_len_timer)) {
        timer_start(&ch3_len_timer, 16384);
        if((reg_nr34 & 0x40) && ch3_len > 0) {
            if(--ch3_len == 0) reg_nr52 &= ~0x04;
#if DEBUG & 0x04
            fprintf(stderr, "CH3 len %u\n", ch3_len);
#endif
        }
    }

    // Wave gen
    uint16_t freq = reg_nr33 | (reg_nr34 & 0x07) << 8;
    if(timer_wait(&ch3_freq_timer)) {
        timer_start(&ch3_freq_timer, (2048 - freq) * 2);

        switch(reg_nr32 & 0x60) {
            case 0x00: 
                ch3_out = 0;
                break;
            case 0x20:
                ch3_out = ch3_wave;
                break;
            case 0x40:
                ch3_out = ch3_wave >> 1;
                break;
            case 0x60:
                ch3_out = ch3_wave >> 2;
        }

        if(ch3_step % 2) {
            ch3_wave = (waveram[ch3_step / 2] >> 4) & 0x0F;
        } else {
            ch3_wave = waveram[ch3_step / 2] & 0x0F;
        }
        if(++ch3_step >= 32) ch3_step = 0;
#if DEBUG & 0x04
        fprintf(stderr, "CH3 wavegen step: %u out: %u freq: %u wave: %02x\n\n", ch3_step, ch3_out, freq, ch3_wave);
#endif
    }

    return ch3_out;
}

int8_t run_ch1_dac(uint8_t ch1_res) {
    if(reg_nr12 & 0xF8) {
        return 127 - (254 * ch1_res) / 15;
    }
    return 0;
}

int8_t run_ch2_dac(uint8_t ch2_res) {
    if(reg_nr22 & 0xF8) {
        return 127 - (254 * ch2_res) / 15;
    }
    return 0;
}

int8_t run_ch3_dac(uint8_t ch3_res) {
    if(reg_nr30 & 0x80) {
        return 127 - (254 * ch3_res) / 15;
    }
    return 0;
}

void init_apu() {
}

void run_apu(void (*do_sample)(int8_t, int8_t)) {
    if(reg_nr14 & 0x80) {
        reg_nr14 &= ~0x80;
        trigger_ch1();
    }
    if(reg_nr24 & 0x80) {
        reg_nr24 &= ~0x80;
        trigger_ch2();
    }
    if(reg_nr34 & 0x80) {
        reg_nr34 &= ~0x80;
        trigger_ch3();
    }
    int8_t ch1_sample = run_ch1_dac(run_ch1());
    int8_t ch2_sample = run_ch2_dac(run_ch2());
    int8_t ch3_sample = run_ch3_dac(run_ch3());

    int left_sum = 0, right_sum = 0, li = 0, ri = 0;
    if(reg_nr51 & 0x01) {
        right_sum += ch1_sample;
        ri++;
    }
    if(reg_nr51 & 0x02) {
        right_sum += ch2_sample;
        ri++;
    }
    if(reg_nr51 & 0x04) {
        right_sum += ch3_sample;
        ri++;
    }
    int right = ri ? right_sum / ri : 0;
    uint8_t vol_right = (reg_nr50 & 0x07) + 1;
    right = (right / vol_right) * 8;

    if(reg_nr51 & 0x10) {
        left_sum += ch1_sample;
        li++;
    }
    if(reg_nr51 & 0x20) {
        left_sum += ch2_sample;
        li++;
    }
    if(reg_nr51 & 0x40) {
        left_sum += ch3_sample;
        li++;
    }
    int left = li ? left_sum / li : 0;
    uint8_t vol_left = ((reg_nr50 >> 4) & 0x07) + 1;
    left = (left / vol_left) * 8;
    
    do_sample((int8_t)left, (int8_t)right);
}