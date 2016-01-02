//////////////////////////////////////////////////////////////////////////////
// fmgengen YM2203 (OPN) emulator
// Copyright (C) 2015 Katayama Hirofumi MZ (katayama.hirofumi.mz@gmail.com).
//////////////////////////////////////////////////////////////////////////////

#ifndef FMGENGEN_YM2203_H
#define FMGENGEN_YM2203_H

#include "opna.h"
#include "YM2203_Timbre.h"

//////////////////////////////////////////////////////////////////////////////
// Number of channels

#define FM_CH1          0   // FM channel 1
#define FM_CH2          1   // FM channel 2
#define FM_CH3          2   // FM channel 3
#define SSG_CH_A        3   // SSG channel A
#define SSG_CH_B        4   // SSG channel B
#define SSG_CH_C        5   // SSG channel C

//////////////////////////////////////////////////////////////////////////////
// Number of Key 

#define KEY_C           0   // C
#define KEY_CS          1   // C#
#define KEY_D           2   // D
#define KEY_DS          3   // D#
#define KEY_E           4   // E
#define KEY_F           5   // F
#define KEY_FS          6   // F#
#define KEY_G           7   // G
#define KEY_GS          8   // G#
#define KEY_A           9   // A
#define KEY_AS          10  // A#
#define KEY_B           11  // B
#define KEY_NUM         12  // 12 keys in a octave

//////////////////////////////////////////////////////////////////////////////
// Total number of channels

#define FM_CH_NUM       3   // a YM2203 has 3 FM channels
#define SSG_CH_NUM      3   // a YM2203 has 3 SSG channels
#define ALL_CH_NUM      (FM_CH_NUM + SSG_CH_NUM)

//////////////////////////////////////////////////////////////////////////////
// Tone/Noise mode of SSG channels

#define TONE_MODE       0   // tone output mode (default)
#define NOISE_MODE      1   // noise output mode
#define TONE_NOISE_MODE 2   // tone & noise output mode

//////////////////////////////////////////////////////////////////////////////
// register address (SSG)

#define ADDR_SSG_TONE_FREQ_L    0x00
#define ADDR_SSG_TONE_FREQ_H    0x01
#define ADDR_SSG_NOISE_FREQ     0x06
#define ADDR_SSG_MIXING         0x07
#define ADDR_SSG_LEVEL_ENV      0x08
#define ADDR_SSG_ENV_FREQ_L     0x0B
#define ADDR_SSG_ENV_FREQ_H     0x0C
#define ADDR_SSG_ENV_TYPE       0x0D

//////////////////////////////////////////////////////////////////////////////
// register address (FM)

#define ADDR_FM_LFO_ON_SPEED    0x22
#define ADDR_FM_KEYON           0x28
#define ADDR_FM_PRESCALER_1     0x2D
#define ADDR_FM_PRESCALER_2     0x2E
#define ADDR_FM_PRESCALER_3     0x2F
#define ADDR_FM_DETUNE_MULTI    0x30
#define ADDR_FM_TL              0x40
#define ADDR_FM_AR_KEYSCALE     0x50
#define ADDR_FM_DR              0x60
#define ADDR_FM_SR              0x70
#define ADDR_FM_SL_RR           0x80
#define ADDR_FM_FREQ_L          0xA0
#define ADDR_FM_FREQ_H          0xA4
#define ADDR_FM_FB_ALGORITHM    0xB0
#define ADDR_FM_LR_AMS_PMS      0xB4

//////////////////////////////////////////////////////////////////////////////
// YM2203

struct YM2203 {
    YM2203();
    ~YM2203() { }

    void init(uint32_t clock, uint32_t rate);
    void set_pitch(int ch, int octave, int key, int adj = 0);

    // assert((0 <= volume) && (volume <= 15));
    void set_volume(int ch, int volume, int adj[4]);
    void set_volume(int ch, int volume) {
        int adj[4] = {0, 0, 0, 0};
        set_volume(ch, volume, adj);
    }
    void note_on(int ch);
    void note_off(int ch);

    bool load_rhythm_data(const char *path) {
        return m_opna.LoadRhythmSample(path);
    }
    void mix(FM_SAMPLETYPE *dest, int nsamples) {
        m_opna.Mix(dest, nsamples);
    }
    bool count(uint32_t microsec) {
        return m_opna.Count(microsec);
    }
    uint32_t get_next_event() {
        return m_opna.GetNextEvent();
    }
    void reset() {
        m_opna.Reset();
    }

    // SSG APIs
    void set_envelope(int ch, int type, uint16_t interval);
    void set_tone_or_noise(int ch, int mode);

    // FM APIs
    void set_timbre(int ch, YM2203_Timbre *timbre);

protected:
    FM::OPNA        m_opna;
    YM2203_Timbre * m_fm_timbres[FM_CH_NUM];
    uint8_t         m_fm_volumes[FM_CH_NUM];
    bool            m_ssg_enveloped[SSG_CH_NUM];
    uint8_t         m_ssg_tone_noise[SSG_CH_NUM];
    uint8_t         m_ssg_key_on;
    uint8_t         m_ssg_envelope_type;

    void write_reg(uint32_t addr, uint32_t data) {
        m_opna.SetReg(addr, data);
    }

    static const uint16_t FM_PITCH_TABLE[KEY_NUM];
    static const uint16_t SSG_PITCH_TABLE[KEY_NUM];
}; // struct YM2203

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef FMGENGEN_YM2203_H

//////////////////////////////////////////////////////////////////////////////
