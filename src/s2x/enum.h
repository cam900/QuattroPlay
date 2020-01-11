#ifndef S2X_ENUM_H_INCLUDED
#define S2X_ENUM_H_INCLUDED

// not quite accurate but i don't care
enum {
    S2X_TRACK_STATUS_BUSY = 0x8000,
    S2X_TRACK_STATUS_START = 0x4000,
    S2X_TRACK_STATUS_FADE = 0x2000,
    S2X_TRACK_STATUS_ATTENUATE = 0x1000,
    S2X_TRACK_STATUS_SUB = 0x800,
    S2X_TRACK_STATUS_CJUMP = 0x400,
};

enum {
    S2X_VOICE_TYPE_NONE = 0,
    S2X_VOICE_TYPE_PCM,
    S2X_VOICE_TYPE_PCMLINK,
    S2X_VOICE_TYPE_SE, // pcm sound effects
    S2X_VOICE_TYPE_FM,
    S2X_VOICE_TYPE_WSG,
};

enum {
    S2X_CHN_OFFSET = 6,
    S2X_CHN_FRQ    = 0x06 - S2X_CHN_OFFSET, // confirmed
    S2X_CHN_WAV    = 0x07 - S2X_CHN_OFFSET, // (or VOICE for FM)
    S2X_CHN_VOL    = 0x08 - S2X_CHN_OFFSET,
    S2X_CHN_TRS    = 0x0c - S2X_CHN_OFFSET,
    S2X_CHN_DTN    = 0x0d - S2X_CHN_OFFSET,
    S2X_CHN_PIT    = 0x0e - S2X_CHN_OFFSET,
    S2X_CHN_PITRAT = 0x0f - S2X_CHN_OFFSET,
    S2X_CHN_LEG    = 0x10 - S2X_CHN_OFFSET,
    S2X_CHN_GTM    = 0x11 - S2X_CHN_OFFSET,
    S2X_CHN_PITDEP = 0x12 - S2X_CHN_OFFSET,
    S2X_CHN_ENV    = 0x13 - S2X_CHN_OFFSET,
    S2X_CHN_DEL    = 0x14 - S2X_CHN_OFFSET,
    S2X_CHN_LFO    = 0x15 - S2X_CHN_OFFSET,
    S2X_CHN_PAN    = 0x16 - S2X_CHN_OFFSET,
    S2X_CHN_PANENV = 0x17 - S2X_CHN_OFFSET,
    S2X_CHN_C18    = 0x18 - S2X_CHN_OFFSET,
    S2X_CHN_VOF    = 0x19 - S2X_CHN_OFFSET, // confirmed
    S2X_CHN_VNO    = 0x20 - S2X_CHN_OFFSET,
    S2X_CHN_PTA    = 0x22 - S2X_CHN_OFFSET,
    //...
    S2X_CHN_MAX    = 0x23 - S2X_CHN_OFFSET,
};

enum {
    S2X_TYPE_SYSTEM2 = 0,
    S2X_TYPE_SYSTEM1,
    S2X_TYPE_SYSTEM1_ALT,
    S2X_TYPE_SYSTEM86,
    S2X_TYPE_NA,
    S2X_TYPE_EM,
    S2X_TYPE_MAX,
};

enum {
    // FM volume handlign
    // 0=calculate FM volume by adding/subtracting (default) (example: Dragon Saber)
    // 1=by multiplying. (example: Final Lap)
    S2X_CFG_FM_VOL = 1<<0,
    // PCM envelope type
    // 0=use table envelope (default) (example: Dragon Saber)
    // 1=use ADSR envelope (example: Assault)
    S2X_CFG_PCM_ADSR = 1<<1,
    // 'new' ADSR envelope type (used in Metal Hawk and later games)
    S2X_CFG_PCM_NEWADSR = 1<<2,
    // invert pan for PCM channels
    S2X_CFG_PCM_PAN = 1<<3,
    // System 1 sound driver
    S2X_CFG_SYSTEM1 = 1<<4,
    // invert pan for FM channels
    S2X_CFG_FM_PAN = 1<<5,
    // WSG header type
    S2X_CFG_WSG_TYPE = 1<<6,
    // WSG command 0b handling
    S2X_CFG_WSG_CMD0B = 1<<7,
    // Dragon Spirit X68K hack for WSG/voice samples
    S2X_CFG_DSPIRIT = 1<<8,
    // link mode enabled (assault, finallap)
    S2X_CFG_PCM_LINKMODE = 1<<9, // 0 disables
    S2X_CFG_PCM_LINKMODE2 = 1<<10, // 0xff disables
};

#endif // S2X_ENUM_H_INCLUDED
