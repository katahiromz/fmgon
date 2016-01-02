//////////////////////////////////////////////////////////////////////////////
// fmgengen YM2203 emulator
// Copyright (C) 2015 Katayama Hirofumi MZ (katayama.hirofumi.mz@gmail.com).
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "YM2203.h"

//////////////////////////////////////////////////////////////////////////////

/*static*/ const uint16_t YM2203::FM_PITCH_TABLE[KEY_NUM] = {
    617, 654, 693, 734, 778, 824, 873, 925, 980, 1038, 1100, 1165
};

/*static*/ const uint16_t YM2203::SSG_PITCH_TABLE[KEY_NUM] = {
    7645, 7215, 6810, 6428, 6067, 5727, 5405, 5102, 4816, 4545, 4290, 4050
};

//////////////////////////////////////////////////////////////////////////////

YM2203::YM2203() {
    // initial value
    m_fm_timbres[FM_CH1] = NULL;
    m_fm_timbres[FM_CH2] = NULL;
    m_fm_timbres[FM_CH3] = NULL;
    m_fm_volumes[FM_CH1] = 0;
    m_fm_volumes[FM_CH2] = 0;
    m_fm_volumes[FM_CH3] = 0;
    m_ssg_enveloped[SSG_CH_A] = false;
    m_ssg_enveloped[SSG_CH_B] = false;
    m_ssg_enveloped[SSG_CH_C] = false;
    m_ssg_tone_noise[SSG_CH_A] = 0x01;
    m_ssg_tone_noise[SSG_CH_B] = 0x02;
    m_ssg_tone_noise[SSG_CH_C] = 0x04;
}

void YM2203::init(uint32_t clock, uint32_t rate) {
    m_opna.Init(clock, rate);
    m_opna.Reset();
    m_ssg_key_on = 0x3F;
    uint32_t addr = ADDR_SSG_MIXING;
    uint32_t data = 0x3F;
    write_reg(addr, data);
}

void YM2203::note_on(int ch) {
    uint32_t addr, data;
    if ((FM_CH1 <= ch) && (ch <= FM_CH3)) {
        if (m_fm_timbres[ch] == NULL) {
            assert(0);
            return;
        }
        addr = ADDR_FM_KEYON;
        data = (m_fm_timbres[ch]->opMask << 4) | ch;
        write_reg(addr, data);
    } else if ((SSG_CH_A <= ch) && (ch <= SSG_CH_C)) {
        m_ssg_key_on &= ~m_ssg_tone_noise[ch - SSG_CH_A];
        addr = ADDR_SSG_MIXING;
        data = m_ssg_key_on;
        write_reg(addr, data);
        if (m_ssg_enveloped[ch - SSG_CH_A]) {
            switch (m_ssg_envelope_type) {
            case 9: case 15:
                addr = ADDR_SSG_ENV_TYPE;
                data = m_ssg_envelope_type;
                write_reg(addr, data);
                break;
            default:
                break;
            }
        }
    } else {
        assert(0);
    }
}

void YM2203::note_off(int ch) {
    uint32_t addr, data;
    if ((FM_CH1 <= ch) && (ch <= FM_CH3)) {
        addr = ADDR_FM_KEYON;
        data = 0 | ch;
        write_reg(addr, data);
    } else if ((SSG_CH_A <= ch) && (ch <= SSG_CH_C)) {
        m_ssg_key_on |= m_ssg_tone_noise[ch - SSG_CH_A];
        addr = ADDR_SSG_MIXING;
        data = m_ssg_key_on;
        write_reg(addr, data);
    } else {
        assert(0);
    }
}

void YM2203::set_pitch(int ch, int octave, int key, int adj) {
    uint32_t addr, data;
    if ((FM_CH1 <= ch) && (ch <= FM_CH3)) {
        addr = ADDR_FM_FREQ_H + ch;
        data = (
            ((octave & 0x07) << 3) |
            (((FM_PITCH_TABLE[key] + adj) >> 8) & 0x07)
        );
        write_reg(addr, data);
        addr = ADDR_FM_FREQ_L + ch;
        data = (uint8_t)(FM_PITCH_TABLE[key] + adj);
        write_reg(addr, data);
    } else if ((SSG_CH_A <= ch) && (ch <= SSG_CH_C)) {
        uint16_t ssg_f = SSG_PITCH_TABLE[key];
        if (octave > 0) {
            ssg_f >>= (octave - 1);
            ssg_f = (ssg_f >> 1) + (ssg_f & 0x0001);
        }
        addr = ADDR_SSG_TONE_FREQ_L + (ch - SSG_CH_A) * 2;
        data = ssg_f;
        write_reg(addr, data);
        addr = ADDR_SSG_TONE_FREQ_H + (ch - SSG_CH_A) * 2;
        data = (ssg_f >> 8) & 0x0F;
        write_reg(addr, data);
    } else {
        assert(0);
    }
}

void YM2203::set_volume(int ch, int volume, int adj[4]) {
    static const uint8_t OP_OFFSET[] = {0x00, 0x08, 0x04, 0x0C};
    uint32_t addr, data;

    assert((0 <= volume) && (volume <= 15));

    if ((FM_CH1 <= ch) && (ch <= FM_CH3)) {
        if (m_fm_timbres[ch] == NULL) {
            assert(0);
            return;
        }

        uint8_t algorithm = m_fm_timbres[ch]->algorithm;
        uint8_t attenate = uint8_t(uint8_t(15 - volume) * 3);

        addr = ADDR_FM_TL + uint8_t(ch) + OP_OFFSET[OPERATOR_4];
        data = ((m_fm_timbres[ch]->tl[OPERATOR_4] + attenate - adj[3]) & 0x7F);
        write_reg(addr, data);

        if (algorithm >= ALGORITHM_4) {
            addr = ADDR_FM_TL + uint8_t(ch) + OP_OFFSET[OPERATOR_2];
            data = ((m_fm_timbres[ch]->tl[OPERATOR_2] + attenate - adj[1]) & 0x7F);
            write_reg(addr, data);
        }

        if (algorithm >= ALGORITHM_5) {
            addr = ADDR_FM_TL + uint8_t(ch) + OP_OFFSET[OPERATOR_3];
            data = ((m_fm_timbres[ch]->tl[OPERATOR_3] + attenate - adj[2]) & 0x7F);
            write_reg(addr, data);
        }

        if (algorithm == ALGORITHM_7) {
            addr = ADDR_FM_TL + (uint8_t)ch + OP_OFFSET[OPERATOR_1];
            data = ((m_fm_timbres[ch]->tl[OPERATOR_1] + attenate - adj[0]) & 0x7F);
            write_reg(addr, data);
        }
    } else if ((SSG_CH_A <= ch) && (ch <= SSG_CH_C)) {
        addr = ADDR_SSG_LEVEL_ENV + (ch - SSG_CH_A);
        data = volume & 0x0F;
        write_reg(addr, data);
    } else {
        assert(0);
    }
}

void YM2203::set_envelope(int ch, int type, uint16_t interval) {
    uint32_t addr, data;

    if ((ch < SSG_CH_A) || (ch > SSG_CH_C)) {
        assert(0);
        return;
    }

    addr = ADDR_SSG_LEVEL_ENV + (ch - SSG_CH_A);
    data = 0x10;
    write_reg(addr, data);
    m_ssg_enveloped[ch - SSG_CH_A] = true;

    addr = ADDR_SSG_ENV_TYPE;
    data = (type & 0x0F);
    write_reg(addr, data);
    m_ssg_envelope_type = data;

    addr = ADDR_SSG_ENV_FREQ_L;
    data = (interval & 0xFF);
    write_reg(addr, data);

    addr = ADDR_SSG_ENV_FREQ_H;
    data = (interval >> 8) & 0xFF;
    write_reg(addr, data);
}

void YM2203::set_tone_or_noise(int ch, int mode) {
    static const uint8_t TONE_MASK[3] = {0x01, 0x02, 0x04};
    static const uint8_t NOISE_MASK[3] = {0x08, 0x10, 0x20};

    if ((ch < SSG_CH_A) || (ch > SSG_CH_C)) {
        assert(0);
        return;
    }
    ch -= SSG_CH_A;

    switch(mode){
    case TONE_MODE:
        m_ssg_tone_noise[ch] = TONE_MASK[ch];
        break;
    case NOISE_MODE:
        m_ssg_tone_noise[ch] = NOISE_MASK[ch];
        break;
    case TONE_NOISE_MODE:
        m_ssg_tone_noise[ch] = TONE_MASK[ch] + NOISE_MASK[ch];
        break;
    }
}

void YM2203::set_timbre(int ch, YM2203_Timbre *timbre) {
    static const uint8_t OP_OFFSET[] = {0x00, 0x08, 0x04, 0x0C};

    if ((ch < 0) || (ch >= FM_CH_NUM)) {
        assert(0);
        return;
    }

    uint32_t addr, data;
    for (int op = OPERATOR_1; op <= OPERATOR_4; ++op) {
        uint8_t tl = timbre->tl[op];
        uint8_t ar = timbre->ar[op];
        uint8_t dr = timbre->dr[op];
        uint8_t sr = timbre->sr[op];
        uint8_t sl = timbre->sl[op];
        uint8_t rr = timbre->rr[op];
        int8_t sDetune = timbre->detune[op];
        uint8_t detune;
        if (sDetune >= 0) {
            detune = uint8_t(sDetune);
        } else {
            detune = uint8_t(4-sDetune);
        }
        uint8_t multiple = timbre->multiple[op];
        uint8_t keyScale = timbre->keyScale[op];
        uint8_t offset = (uint8_t)ch + OP_OFFSET[op];

        addr = ADDR_FM_DETUNE_MULTI + offset;
        data = ((detune & 0x07) << 4) | (multiple & 0x0F);
        write_reg(addr, data);

        addr = ADDR_FM_TL + offset;
        data = (tl & 0x7F);
        write_reg(addr, data);

        addr = ADDR_FM_AR_KEYSCALE + offset;
        data = (((keyScale & 0x03) << 6) | (ar & 0x1F));
        write_reg(addr, data);

        addr = ADDR_FM_DR + offset;
        data = (dr & 0x1F);
        write_reg(addr,data);

        addr = ADDR_FM_SR + offset;
        data = (sr & 0x1F);
        write_reg(addr, data);

        addr = ADDR_FM_SL_RR + offset;
        data = (((sl & 0x0F) << 4) | (rr & 0x0F));
        write_reg(addr, data);
    }

    addr = ADDR_FM_FB_ALGORITHM + ch;
    data = (((timbre->feedback & 0x07) << 3) | (timbre->algorithm & 0x07));
    write_reg(addr, data);

    // TODO: LFO
    //addr = ADDR_FM_LFO_ON_SPEED;
    //data = ...
    //write_reg(addr, data);
    //
    //addr = ADDR_FM_LR_AMS_PMS;
    //data = ...;
    //write_reg(addr, data);

    m_fm_timbres[ch] = timbre;
} // YM2203::set_timbre

//////////////////////////////////////////////////////////////////////////////
