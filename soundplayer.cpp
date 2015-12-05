//////////////////////////////////////////////////////////////////////////////
// soundplayer --- an fmgon sound player
// Copyright (C) 2015 Katayama Hirofumi MZ. All Rights Reserved.
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "soundplayer.h"

#define CLOCK       4000000
#define SAMPLERATE  44100

//////////////////////////////////////////////////////////////////////////////

float VskNote::get_sec(int tempo, int length) const {
    float sec;
    assert(tempo != 0);
    // NOTE: 24 is the length of a quarter note
    if (m_dot) {
        sec = float(length * (60.0 * 2 * 1.5 / 24)) / tempo;
    } else {
        sec = float(length * (60.0 * 2 / 24)) / tempo;
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

    if (m_setting.m_fm) {
        int ch = FM_CH1;

        int tone = -1;

        for (auto& note : m_notes) {
            // do key on
            if (note.m_key != -1) {
                // change tone if necessary
                if (tone != note.m_tone) {
                    const auto new_tone = note.m_tone;
                    assert((0 <= new_tone) && (new_tone < NUM_TONES));
                    auto& timbre = m_setting.m_timbre;
                    timbre.set(ym2203_tone_table[new_tone]);
                    ym.set_timbre(ch, &timbre);
                    tone = new_tone;
                }

                ym.set_pitch(ch, note.m_octave, note.m_key);
                ym.set_volume(ch, 15);
                ym.key_on(ch);
            }

            // render sound
            auto sec = note.m_sec;
            auto nsamples = int(SAMPLERATE * sec);
            memset(&data[isample], 0, nsamples * 2 * sizeof(FM_SAMPLETYPE));
            ym.mix(&data[isample], nsamples * 2);
            ym.count(uint32_t(sec * 1000 * 1000));
            isample += nsamples;

            // do key off
            ym.key_off(ch);
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
                ym.key_on(ch);
            }

            // render sound
            auto sec = note.m_sec;
            auto nsamples = int(SAMPLERATE * sec);
            memset(&data[isample], 0, nsamples * 2 * sizeof(FM_SAMPLETYPE));
            ym.mix(&data[isample], nsamples * 2);
            ym.count(uint32_t(sec * 1000 * 1000));
            isample += nsamples;

            // do key off
            ym.key_off(ch);
            ym.mix(NULL, 0);
            ym.count(0);
        }
    }

    // reverb of 1sec
    {
        auto sec = 1;
        auto nsamples = int(SAMPLERATE * sec);
        ym.mix(&data[isample], nsamples * 2);
        ym.count(uint32_t(sec * 1000 * 1000));
        isample += nsamples;
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

                auto msec = uint32_t(goal * 1000.0 / 2);
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
    int main(void) {
        alutInit(NULL, NULL);

        auto phrase = make_shared<VskPhrase>();
        phrase->m_setting.m_tempo = 120;
        phrase->m_setting.m_octave = 4;

        // NOTE: 24 is the length of a quarter note
        phrase->m_setting.m_length = 24;
        phrase->m_setting.m_tone = 15;   // @7 FLUTE

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
