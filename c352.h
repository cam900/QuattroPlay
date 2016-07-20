/*
    C352 chip emulator for QuattroPlay
    written by ctr with help from MAME source code (written by R.Belmont)

    perhaps internal registers are readable at 0x100-0x1ff
*/
#ifndef C352_H_INCLUDED
#define C352_H_INCLUDED

#include "stdint.h"

#define C352_VOICES 32

enum {
    C352_FLG_BUSY       = 0x8000,   // channel is busy
    C352_FLG_KEYON      = 0x4000,   // Keyon
    C352_FLG_KEYOFF     = 0x2000,   // Keyoff
    C352_FLG_LOOPTRG    = 0x1000,   // Loop Trigger
    C352_FLG_LOOPHIST   = 0x0800,   // Loop History
    C352_FLG_FM         = 0x0400,   // Frequency Modulation (according to MAME)
    C352_FLG_PHASERL    = 0x0200,   // Rear  left, invert phase (could be other order for Front/Rear...)
    C352_FLG_PHASEFL    = 0x0100,   // Front left, invert phase
    C352_FLG_PHASEFR    = 0x0080,   // F/R   right invert phase
    C352_FLG_LDIR       = 0x0040,   // loop direction
    C352_FLG_LINK       = 0x0020,   // linked sample (sample addresses are reloaded at the end of current sample)
    C352_FLG_NOISE      = 0x0010,   // play noise instead of sample
    C352_FLG_MULAW      = 0x0008,   // sample is mulaw instead of linear 8-bit PCM
    C352_FLG_FILTER     = 0x0004,   // don't apply interpolation
    C352_FLG_REVLOOP    = 0x0003,   // loop backwards
    C352_FLG_LOOP       = 0x0002,   // loop forward
    C352_FLG_REVERSE    = 0x0001    // play sample backwards
};

typedef struct {

    uint32_t pos;
	uint32_t counter;

	int16_t sample;
    int16_t last_sample;

    uint16_t vol_f;
    uint16_t vol_r;
    uint16_t freq;
    uint16_t flags;

    uint16_t wave_bank;
    uint16_t wave_start;
    uint16_t wave_end;
    uint16_t wave_loop;

} C352_Voice;

typedef struct {

    uint32_t rate;

    C352_Voice v[C352_VOICES];
    double out[4];

    uint16_t control1; // unknown purpose for both
    uint16_t control2;

    uint8_t* wave;
    uint32_t wave_mask;

    uint16_t random;

    int16_t mulaw[256];

    // special
    uint32_t mute_mask;
    uint8_t mute_rear;

} C352;



int C352_init(C352 *c,uint32_t clk);

// should be run with the C352_rate
void C352_update(C352 *c);

int16_t C352_update_voice(C352 *c, int v);

void C352_write(C352 *c, uint16_t addr, uint16_t data);
uint16_t C352_read(C352 *c, uint16_t addr);

#endif // C352_H_INCLUDED
