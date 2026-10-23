// TODO: clean up code, timer phase alignment

#include <stdbool.h>
#include <stdint.h>
#include <timer.h>
#include <stdio.h>

uint8_t
    reg_nr50, reg_nr51, reg_nr52,
    reg_nr10, reg_nr11, reg_nr12, reg_nr13, reg_nr14,
    reg_nr21, reg_nr22, reg_nr23, reg_nr24,
    reg_nr30, reg_nr31, reg_nr32, reg_nr33, reg_nr34,
    reg_nr41, reg_nr42, reg_nr43, reg_nr44;
bool
    wrote_nr14, wrote_nr24, wrote_nr34, wrote_nr44;
uint8_t waveram[16];

struct timer fs_timer;
uint8_t fs_step;

struct timer ch1_freq_timer;
uint8_t ch1_sw_ctr, ch1_vol_ctr, ch1_len_ctr;
uint8_t ch1_step, ch1_vol, ch1_out;
uint16_t ch1_shadow;
bool ch1_sw_en;

struct timer ch2_freq_timer;
uint8_t ch2_vol_ctr, ch2_len_ctr;
uint8_t ch2_step, ch2_vol, ch2_out;

struct timer ch3_freq_timer;
uint8_t ch3_len_ctr;
uint8_t ch3_step, ch3_wave, ch3_out;

void trigger_ch1() {
    if(!(reg_nr12 & 0xF8)) return;
    reg_nr52 |= 0x01;

    uint16_t freq = reg_nr13 | (reg_nr14 & 0x07) << 8;
    timer_start(&ch1_freq_timer, (2048 - freq) * 4);

    uint8_t vol_pace = reg_nr12 & 0x07;
    ch1_vol_ctr = vol_pace ? vol_pace : 8;

    uint8_t sw_pace = (reg_nr10 >> 4) & 0x07;
    ch1_sw_ctr = sw_pace ? sw_pace : 8;

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

    if(ch1_len_ctr == 0) ch1_len_ctr = 64; // TODO: check for write
#if DEBUG & 0x04
    fprintf(stderr, "CH1 triggered\n");
#endif
}

void trigger_ch2() {
    if(!(reg_nr22 & 0xF8)) return;
    reg_nr52 |= 0x02;

    uint16_t freq = reg_nr23 | (reg_nr24 & 0x07) << 8;
    timer_start(&ch2_freq_timer, (2048 - freq) * 4);

    uint8_t vol_pace = reg_nr22 & 0x07;
    ch2_vol_ctr = vol_pace ? vol_pace : 8;

    ch2_vol = (reg_nr22 >> 4) & 0x0F;
    ch2_step = 0;

    if(ch2_len_ctr == 0) ch2_len_ctr = 64; // TODO: check for write
#if DEBUG & 0x04
    fprintf(stderr, "CH2 triggered\n");
#endif
}

void trigger_ch3() {
    if(!(reg_nr30 & 0x80)) return;
    reg_nr52 |= 0x04;

    uint16_t freq = reg_nr33 | (reg_nr34 & 0x07) << 8;
    timer_start(&ch3_freq_timer, (2048 - freq) * 2);

    ch3_step = 1;

    if(ch3_len_ctr == 0) ch3_len_ctr = 64; // TODO: check for write
#if DEBUG & 0x04
    fprintf(stderr, "CH3 triggered\n");
#endif
}

void do_vol(uint8_t channel, uint8_t nrx2, uint8_t* vol_ctr, uint8_t* vol) {
    if(!(reg_nr52 & (1 << (channel - 1)))) return;

    // Volume sweep
    uint8_t vol_pace = nrx2 & 0x07;
    if(*vol_ctr > 0 && --(*vol_ctr) == 0) {
        *vol_ctr = vol_pace ? vol_pace : 8;

        int v = (nrx2 & 0x08) ? *vol + 1 : *vol - 1;
        if(vol_pace && v >= 0 && v < 16)
            *vol = (uint8_t)v;
#if DEBUG & 0x04
        fprintf(stderr, "CH%d envelope pace: %u\n", channel, vol_pace);
#endif
    }
}

void do_len(uint8_t channel, uint8_t nrx4, uint8_t* len_ctr) {
    if(!(reg_nr52 & (1 << (channel - 1)))) return;

    // Length timer
    if(nrx4 & 0x40) {
        if(*len_ctr > 0 && --(*len_ctr) == 0)
            reg_nr52 &= ~(1 << (channel - 1));
#if DEBUG & 0x04
        fprintf(stderr, "CH%d len %u\n", channel, *len_ctr);
#endif
    }
}

void do_ch1_sw() {
    if(!(reg_nr52 & 0x01)) return;

    // Freq sweep
    uint8_t sw_pace = (reg_nr10 >> 4) & 0x07;
    if(ch1_sw_ctr > 0 && --ch1_sw_ctr == 0) {
        ch1_sw_ctr = sw_pace ? sw_pace : 8;

        if(ch1_sw_en && sw_pace != 0) {
            uint8_t shift = reg_nr10 & 0x07;
            uint16_t freq_next = (reg_nr10 & 0x08) ?
                ch1_shadow - (ch1_shadow >> shift) :
                ch1_shadow + (ch1_shadow >> shift);

            if(freq_next > 0x07FF) reg_nr52 &= ~0x01;
            else if(shift != 0) {
                ch1_shadow = freq_next;
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
}

uint8_t run_ch1() {
    if(!(reg_nr52 & 0x01)) return 0;

    if(wrote_nr14 && (reg_nr14 & 0x40)) {
        ch1_len_ctr = 64 - (reg_nr11 & 0x3F);
        wrote_nr14 = false;
    }

    uint16_t freq = reg_nr13 | (reg_nr14 & 0x07) << 8;

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

    if(wrote_nr24 && (reg_nr24 & 0x40)) {
        ch2_len_ctr = 64 - (reg_nr21 & 0x3F);
        wrote_nr24 = false;
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

    if(wrote_nr34 && (reg_nr34 & 0x40)) {
        ch3_len_ctr = 64 - (reg_nr31 & 0x3F);
        wrote_nr34 = false;
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
    } else {
        reg_nr52 &= ~0x01;
    }
    return 0;
}

int8_t run_ch2_dac(uint8_t ch2_res) {
    if(reg_nr22 & 0xF8) {
        return 127 - (254 * ch2_res) / 15;
    } else {
        reg_nr52 &= ~0x02;
    }
    return 0;
}

int8_t run_ch3_dac(uint8_t ch3_res) {
    if(reg_nr30 & 0x80) {
        return 127 - (254 * ch3_res) / 15;
    } else {
        reg_nr52 &= ~0x04;
    }
    return 0;
}

void run_fs() {
    if(timer_wait(&fs_timer)) {
        timer_start(&fs_timer, 8192);
        switch(fs_step) {
            case 0:
            case 4:
                do_len(1, reg_nr14, &ch1_len_ctr);
                do_len(2, reg_nr24, &ch2_len_ctr);
                do_len(3, reg_nr34, &ch3_len_ctr);
                break;
            case 2:
            case 6:
                do_len(1, reg_nr14, &ch1_len_ctr);
                do_len(2, reg_nr24, &ch2_len_ctr);
                do_len(3, reg_nr34, &ch3_len_ctr);
                do_ch1_sw();
                break;
            case 7:
                do_vol(1, reg_nr12, &ch1_vol_ctr, &ch1_vol);
                do_vol(2, reg_nr22, &ch2_vol_ctr, &ch2_vol);
                break;

        }
        if(++fs_step >= 8) fs_step = 0;
    }
}

void apu_power_off() {
    reg_nr50 = reg_nr51 =
    reg_nr10 = reg_nr12 = reg_nr13 = reg_nr14 =
    reg_nr22 = reg_nr23 = reg_nr24 =
    reg_nr30 = reg_nr32 = reg_nr33 = reg_nr34 =
    reg_nr42 = reg_nr43 = reg_nr44 = 0;
    reg_nr11 &= 0x3F;
    reg_nr21 &= 0x3F;
    reg_nr41 &= 0x3F;
    reg_nr52 = 0;
    wrote_nr14 = wrote_nr24 = wrote_nr34 = wrote_nr44 = false;
}

void apu_power_on() {
    fs_step = 0;
}

void init_apu() {
}

void run_apu(void (*do_sample)(int8_t, int8_t)) {
    if(!(reg_nr52 & 0x80)) return;

    if(wrote_nr14 && (reg_nr14 & 0x80)) trigger_ch1();
    if(wrote_nr24 && (reg_nr24 & 0x80)) trigger_ch2();
    if(wrote_nr34 && (reg_nr34 & 0x80)) trigger_ch3();

    run_fs();

    int8_t ch1_sample = run_ch1_dac(run_ch1());
    int8_t ch2_sample = run_ch2_dac(run_ch2());
    int8_t ch3_sample = run_ch3_dac(run_ch3());

    if(wrote_nr14) wrote_nr14 = false;
    if(wrote_nr24) wrote_nr24 = false;
    if(wrote_nr34) wrote_nr34 = false;
    if(wrote_nr44) wrote_nr44 = false;

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