//////////////////////////////////////////////////////////////////////////////
// soundplayer --- an fmgon sound player
// Copyright (C) 2015 Katayama Hirofumi MZ. All Rights Reserved.
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "soundplayer.h"

#define CLOCK       8000000
#define SAMPLERATE  44100

#define LFO_INTERVAL 150

class LFOctrl {
    int m_waveform;
    int m_qperiod; // quarter of period
    int m_count;
    int m_phase; // 0, 1, 2 or 3
    float m_adj_p_max;
    float m_adj_v_max[4];
    float m_adj_p_diff;
    float m_adj_v_diff[4];
public:
    float m_adj_p; // for pitch
    float m_adj_v[4]; // for volume

public:
    LFOctrl() { }

    void init_for_timbre(YM2203_Timbre *p_timbre) {
        int i;
        m_waveform = p_timbre->waveForm;
        m_qperiod = (p_timbre->speed)?900*LFO_INTERVAL/(4*p_timbre->speed):0;
        //m_count = 0;
        m_phase = 0;
        m_adj_p_max = p_timbre->pmd * (float)p_timbre->pms / 2.0; // TBD
        for (i=0; i<4; i++)
            m_adj_v_max[i] = p_timbre->amd * (float)p_timbre->ams[i] / 2; // TBD
        init_for_phase(true);
    }

    void init_for_keyon(YM2203_Timbre *p_timbre) {
        if (p_timbre->sync) {
            m_phase = 0;
            init_for_phase();
        }
    }

    void increment() {
        int i;
        if (0 == m_qperiod) {
            return;
        }
        m_count++;
        if (m_count < m_qperiod) {
            m_adj_p += m_adj_p_diff;
            for(i=0; i<4; i++) {
                m_adj_v[i] += m_adj_v_diff[i];
            }
        } else {
            m_phase = (m_phase + 1) & 3;
            init_for_phase();
        }
    }

private:
    void init_for_phase(bool flag_first = false) {
        int i;
        m_count = 0;
        if (flag_first) {
            switch (m_waveform) {
            case 0: // saw
                m_adj_p = 0;
                for(i=0; i<4; i++) {
                    m_adj_v[i] = 0;
                }
                m_adj_p_diff = m_adj_p_max / (m_qperiod * 2);
                for(i=0; i<4; i++) {
                    m_adj_v_diff[i] = m_adj_v_max[i] / (m_qperiod * 2);
                }
                break;
            case 1: // square
                m_adj_p = -m_adj_p_max;
                for(i=0; i<4; i++) {
                    m_adj_v[i] = -m_adj_v_max[i];
                }
                m_adj_p_diff = 0;
                for(i=0; i<4; i++) {
                    m_adj_v_diff[i] = 0;
                }
                break;
            case 2: // triangle
                m_adj_p = 0;
                for(i=0; i<4; i++) {
                    m_adj_v[i] = 0;
                }
                m_adj_p_diff = m_adj_p_max / m_qperiod;
                for(i=0; i<4; i++) {
                    m_adj_v_diff[i] = m_adj_v_max[i] / m_qperiod;
                }
                break;
            default: // sample and hold
                //m_adj_p = m_adj_p_max * (rand() * 2.0 / RAND_MAX - 1);
                //for(i=0; i<4; i++) m_adj_v[i] = m_adj_v_max[i] * (rand() * 2.0 / RAND_MAX - 1);
                m_adj_p_diff = 0;
                for(i=0; i<4; i++) {
                    m_adj_v_diff[i] = 0;
                }
                break;
            }
        }
        switch (m_waveform) {
        case 0: // saw
            if (0 == m_phase) {
                m_adj_p = 0;
                for(i=0; i<4; i++) {
                    m_adj_v[i] = 0;
                }
            } else if (2 == m_phase) {
                m_adj_p = -m_adj_p;
                for(i=0; i<4; i++) {
                    m_adj_v[i] = -m_adj_v[i];
                }
            }
            break;
        case 1: // square
            if (0 == (m_phase & 1)) {
                m_adj_p = -m_adj_p;
                for(i=0; i<4; i++) {
                    m_adj_v[i] = -m_adj_v[i];
                }
            }
            break;
        case 2: // triangle
            if (0 == m_phase) {
                m_adj_p = 0;
                for(i=0; i<4; i++) {
                    m_adj_v[i] = 0;
                }
            } else if (1 == (m_phase & 1)) {
                m_adj_p_diff = -m_adj_p_diff;
                for(i=0; i<4; i++) {
                    m_adj_v_diff[i] = -m_adj_v_diff[i];
                }
            }
            break;
        default: // sample and hold
            if (0 == (m_phase & 1)) {
                m_adj_p = m_adj_p_max * (rand() * 2.0 / RAND_MAX - 1);
                for(i=0; i<4; i++) {
                    m_adj_v[i] = m_adj_v_max[i] * (rand() * 2.0 / RAND_MAX - 1);
                }
            }
            break;
        }
    }
}; // LFOctrl


//////////////////////////////////////////////////////////////////////////////

float VskNote::get_sec(int tempo, int length) const {
    float sec;
    assert(tempo != 0);
    // NOTE: 24 is the length of a quarter note
    if (m_dot) {
        sec = float(length * (60.0 * 1.5 / 24)) / tempo;
    } else {
        sec = float(length * (60.0 / 24)) / tempo;
    }
    return sec;
} // VskNote::get_sec

void VskNote::set_key_from_char(char ch) {
    if (ch != 'R') {
        static const char keys[KEY_NUM + 1] = "C+D+EF+G+A+B";

        const char *ptr = strchr(keys, ch);
        assert(ptr != NULL);
        assert(*ptr == ch);
        m_key = int(ptr - keys);

        switch (m_sign) {
        case '+': case '#':
            if (m_key == KEY_B) {
                m_key = KEY_C;
                if (m_octave < 7) {
                    ++m_octave;
                }
            } else {
                --m_key;
            }
            break;
        case '-':
            if (m_key == KEY_C) {
                m_key = KEY_C;
                if (m_octave > 0) {
                    --m_octave;
                }
            } else {
                ++m_key;
            }
            break;
        default:
            break;
        }
    } else {
        m_key = -1;
    }
} // VskNote::char_to_key

//////////////////////////////////////////////////////////////////////////////

void VskPhrase::destroy() {
    if (m_buffer != ALuint(-1)) {
        alDeleteBuffers(1, &m_buffer);
        m_buffer = ALuint(-1);
    }
    if (m_source != ALuint(-1)) {
        alDeleteSources(1, &m_source);
        m_source = ALuint(-1);
    }
} // VskPhrase::destroy

void VskPhrase::calc_total() {
    float gate = 0;
    for (auto& note : m_notes) {
        note.m_gate = gate;
        gate += note.m_sec;
    }
    m_goal = gate;
} // VskPhrase::calc_total

void VskPhrase::realize(VskSoundPlayer *player) {
    destroy();
    calc_total();

    // initialize YM2203
    YM2203& ym = player->m_ym;
    ym.init(CLOCK, SAMPLERATE);
    ym.reset();

    // create wave data
    uint32_t isample = 0;
    auto size = uint32_t((m_goal + 1) * SAMPLERATE * 2);
    unique_ptr<FM_SAMPLETYPE[]> data(new FM_SAMPLETYPE[size]);
    memset(&data[0], 0, size * sizeof(FM_SAMPLETYPE));

    if (m_setting.m_fm) {
        int ch = FM_CH1;

        int tone = -1;
        LFOctrl lc;

        for (auto& note : m_notes) {
            // do key on
            auto& timbre = m_setting.m_timbre;
            if (note.m_key != -1) {
                // change tone if necessary
                if (tone != note.m_tone) {
                    const auto new_tone = note.m_tone;
                    assert((0 <= new_tone) && (new_tone < NUM_TONES));
                    timbre.set(ym2203_tone_table[new_tone]);
                    ym.set_timbre(ch, &timbre);
                    lc.init_for_timbre(&timbre);
                    tone = new_tone;
                }

                ym.set_pitch(ch, note.m_octave, note.m_key);
                ym.set_volume(ch, 15);
                ym.note_on(ch);
            }

            lc.init_for_keyon(&timbre);

            // render sound
            auto sec = note.m_sec;
            auto nsamples = int(SAMPLERATE * sec);
            {
                int unit;
                while (nsamples) {
                    unit = SAMPLERATE/LFO_INTERVAL;
                    if (unit > nsamples) {
                        unit = nsamples;
                    }
                    ym.mix(&data[isample * 2], unit);
                    isample += unit;
                    if (note.m_key != -1) {
                        lc.increment();
                        ym.set_volume(ch, 15, lc.m_adj_v[0], lc.m_adj_v[1], lc.m_adj_v[2], lc.m_adj_v[3]);
                        ym.set_pitch(ch, note.m_octave, note.m_key, lc.m_adj_p);
                    }
                    nsamples -= unit;
                }
            }
            ym.count(uint32_t(sec * 1000 * 1000));
            isample += nsamples;

            // do key off
            ym.note_off(ch);
            ym.mix(NULL, 0);
            ym.count(0);
        }
    } else {
        int ch = SSG_CH_A;

        ym.set_tone_or_noise(ch, TONE_MODE);

        for (auto& note : m_notes) {
            // do key on
            if (note.m_key != -1) {
                ym.set_pitch(ch, note.m_octave, note.m_key);
                ym.set_volume(ch, 15);
                ym.note_on(ch);
            }

            // render sound
            auto sec = note.m_sec;
            auto nsamples = int(SAMPLERATE * sec);
            ym.mix(&data[isample * 2], nsamples);
            ym.count(uint32_t(sec * 1000 * 1000));
            isample += nsamples;

            // do key off
            ym.note_off(ch);
            ym.mix(NULL, 0);
            ym.count(0);
        }
    }

    // reverb of 1sec
    {
        auto sec = 1;
        auto nsamples = int(SAMPLERATE * sec);
        ym.mix(&data[isample * 2], nsamples);
        ym.count(uint32_t(sec * 1000 * 1000));
    }

    // generate an OpenAL buffer
    alGenBuffers(1, &m_buffer);
    assert(m_buffer != ALuint(-1));
    alBufferData(m_buffer, AL_FORMAT_STEREO16, &data[0],
        sizeof(FM_SAMPLETYPE) * size, SAMPLERATE);

    // generate an OpenAL source
    alGenSources(1, &m_source);
    assert(m_source != ALuint(-1));
    alSourcei(m_source, AL_BUFFER, m_buffer);
} // VskPhrase::realize

//////////////////////////////////////////////////////////////////////////////

bool VskSoundPlayer::wait_for_stop(uint32_t milliseconds) {
    return m_stopping_event.wait_for_event(milliseconds);
}

bool VskSoundPlayer::play_and_wait(VskScoreBlock& block, uint32_t milliseconds) {
    play(block);
    return wait_for_stop(milliseconds);
}

void VskSoundPlayer::play(VskScoreBlock& block) {
    int i = 0;
    for (auto& phrase : block) {
        if (phrase) {
            phrase->realize(this);
        }
        ++i;
    }

    m_lock.lock();
    m_melody_line.push_back(block);
    m_lock.unlock();

    if (m_playing_music) {
        return;
    }

    m_playing_music = false;
    m_stopping_event.pulse();
    m_playing_music = true;

    std::thread(
        [this](int dummy) {
            VskScoreBlock phrases;
            for (;;) {
                // get the next block
                m_lock.lock();
                if (m_melody_line.empty()) {
                    m_lock.unlock();
                    m_stopping_event.pulse();
                    break;
                }
                phrases = m_melody_line.front();
                m_melody_line.pop_front();
                m_lock.unlock();

                // get the goal
                float goal = 0;
                for (auto& phrase : phrases) {
                    if (phrase) {
                        if (goal < phrase->m_goal) {
                            goal = phrase->m_goal;
                        }
                    }
                }

                // play phrases
                for (auto& phrase : phrases) {
                    if (phrase) {
                        alSourcePlay(phrase->m_source);
                    }
                }

                auto msec = uint32_t(goal * 1000.0);
                m_stopping_event.wait_for_event(msec);
            }
            if (m_playing_music) {
                m_playing_music = false;
                alutSleep(1.0);
            }
        },
        0
    ).detach();
} // VskSoundPlayer::play

void VskSoundPlayer::stop() {
    m_playing_music = false;
    m_stopping_event.pulse();
}

//////////////////////////////////////////////////////////////////////////////
// beep

void VskSoundPlayer::init_beep() {
    m_beep_buffer = alutCreateBufferWaveform(
        ALUT_WAVEFORM_SINE,
        2400,
        0,
        0.5
    );
    //m_beep_buffer = alutCreateBufferFromFile("beep.wav");
    alGenSources(1, &m_beep_source);
    alSourcei(m_beep_source, AL_BUFFER, m_beep_buffer);
} // VskSoundPlayer::init_beep

void VskSoundPlayer::beep(int i) {
    switch (i) {
    case -1:
        alSourceStop(m_beep_source);
        alSourcei(m_beep_source, AL_LOOPING, AL_FALSE);
        alSourcePlay(m_beep_source);
        break;
    case 0:
        alSourceStop(m_beep_source);
        alSourcei(m_beep_source, AL_LOOPING, AL_FALSE);
        break;
    case 1:
        alSourceStop(m_beep_source);
        alSourcei(m_beep_source, AL_LOOPING, AL_TRUE);
        alSourcePlay(m_beep_source);
    }
} // VskSoundPlayer::beep

void VskSoundPlayer::free_beep() {
    alDeleteBuffers(1, &m_beep_buffer);
    alDeleteSources(1, &m_beep_source);
}

//////////////////////////////////////////////////////////////////////////////

#ifdef SOUND_TEST
    int main(int ac, char *av[]) {
        alutInit(NULL, NULL);

        auto phrase = make_shared<VskPhrase>();
        phrase->m_setting.m_tempo = 120;
        phrase->m_setting.m_octave = 3;

        // NOTE: 24 is the length of a quarter note
        phrase->m_setting.m_length = 24;
        phrase->m_setting.m_tone = (ac<2)?15:atoi(av[1]); // @15 DESCENT

        phrase->add_note('C');
        phrase->add_note('D');
        phrase->add_note('E');
        phrase->add_note('F');
        phrase->add_note('E');
        phrase->add_note('D');
        phrase->add_note('C');
        phrase->add_note('R');

        phrase->add_note('E');
        phrase->add_note('F');
        phrase->add_note('G');
        phrase->add_note('A');
        phrase->add_note('G');
        phrase->add_note('F');
        phrase->add_note('E');
        phrase->add_note('R');

        phrase->add_note('C');
        phrase->add_note('R');
        phrase->add_note('C');
        phrase->add_note('R');
        phrase->add_note('C');
        phrase->add_note('R');
        phrase->add_note('C');
        phrase->add_note('R');

        phrase->m_setting.m_length = 12;
        phrase->add_note('C');
        phrase->add_note('C');
        phrase->add_note('D');
        phrase->add_note('D');
        phrase->add_note('E');
        phrase->add_note('E');
        phrase->add_note('F');
        phrase->add_note('F');
        phrase->m_setting.m_length = 24;
        phrase->add_note('E');
        phrase->add_note('D');
        phrase->add_note('C');
        phrase->add_note('R');

        VskScoreBlock block;
        block.push_back(phrase);

        VskSoundPlayer player;
        player.play_and_wait(block);

        alutExit();
    } // main
#endif  // def SOUND_TEST

//////////////////////////////////////////////////////////////////////////////
