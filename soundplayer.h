//////////////////////////////////////////////////////////////////////////////
// soundplayer --- an fmgon sound player
// Copyright (C) 2015 Katayama Hirofumi MZ. All Rights Reserved.
//////////////////////////////////////////////////////////////////////////////

#ifndef FMGON_SOUNDPLAYER_H
#define FMGON_SOUNDPLAYER_H

//////////////////////////////////////////////////////////////////////////////
// OpenAL + ALUT --- portable audio libraries

#define ALUT_STATIC
#include <AL/alut.h>

#if defined(_WIN32) && defined(_MSC_VER)
    #pragma comment(lib, "OpenAL32.lib")
    #pragma comment(lib, "alut.lib")
#endif

//////////////////////////////////////////////////////////////////////////////
// pevent --- portable event objects

#ifdef _WIN32
    #include "win32_pevent.h"
#else
    #include "pevent.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// YM2203

#include "YM2203.h"

//////////////////////////////////////////////////////////////////////////////
// VskNote

struct VskNote {
    int         m_tempo;
    int         m_octave;
    int         m_tone;
    int         m_key;
    bool        m_dot;
    float       m_length;
    char        m_sign;
    float       m_sec;
    float       m_gate;
    float       m_volume;   // in 0 to 15
    int         m_quantity; // in 0 to 8

    VskNote(int tempo, int octave, int tone, int note,
            bool dot = false, float length = 24, char sign = 0,
            float volume = 8, int quantity = 8,
            int envelop_type = 1, uint16_t envelop_interval = 255)
    {
        m_tempo = tempo;
        m_octave = octave;
        m_tone = tone;
        m_dot = dot;
        m_length = length;
        m_sign = sign;
        m_sec = get_sec(m_tempo, m_length);
        set_key_from_char(note);
        m_volume = volume;
        m_quantity = quantity;
    }

    float get_sec(int tempo, float length) const;
    void set_key_from_char(char note);

private:
    VskNote();
}; // struct VskNote

//////////////////////////////////////////////////////////////////////////////
// VskSoundSetting

struct VskSoundSetting {
    int                 m_tempo;
    int                 m_octave;   // 
    float               m_length;   // 24 is the length of a quarter note
    int                 m_tone;     // see YM2203_Timbre
    YM2203_Timbre       m_timbre;   // see YM2203_Timbre
    bool                m_fm;       // whether it is FM or not?
    float               m_volume;   // in 0 to 15
    int                 m_quantity; // in 0 to 8

    VskSoundSetting(int tempo = 120, int octave = 4, float length = 24,
                    int tone = 0, bool fm = false) :
        m_tempo(tempo), m_octave(octave), m_length(length), m_tone(tone),
        m_fm(fm)
    {
        m_volume = 8;
        m_quantity = 8;
    }

    void reset() {
        m_tempo = 120;
        m_octave = 4;
        m_length = 24;
        m_tone = 0;
        m_fm = false;
        m_volume = 8;
        m_quantity = 8;
    }
}; // struct VskSoundSetting

//////////////////////////////////////////////////////////////////////////////
// VskPhrase

struct VskSoundPlayer;

struct VskPhrase {
    float                           m_goal = 0;
    ALuint                          m_buffer = -1;
    ALuint                          m_source = -1;
    VskSoundSetting                 m_setting;
    std::vector<VskNote>            m_notes;

    VskPhrase() { }
    VskPhrase(const VskSoundSetting& setting) : m_setting(setting) { }

    ~VskPhrase() {
        destroy();
    }

    void add_note(char note) {
        add_note(note, false);
    }
    void add_note(char note, bool dot) {
        add_note(note, dot, m_setting.m_length);
    }
    void add_note(char note, bool dot, float length) {
        add_note(note, dot, length, 0);
    }
    void add_note(char note, bool dot, float length, char sign) {
        add_note(note, dot, length, sign, m_setting.m_quantity);
    }
    void add_note(char note, bool dot, float length, char sign, int quantity) {
        add_note(m_setting.m_tone, note, dot, length, sign, quantity);
    }
    void add_note(int tone, char note, bool dot, float length, char sign,
                  int quantity)
    {
        m_notes.emplace_back(
            m_setting.m_tempo, m_setting.m_octave,
            tone, note, dot, length, sign, m_setting.m_volume, quantity);
    }
    void add_key(int key) {
        add_key(key, false);
    }
    void add_key(int key, bool dot) {
        add_key(key, dot, m_setting.m_length);
    }
    void add_key(int key, bool dot, float length) {
        add_key(key, dot, length, 0);
    }
    void add_key(int key, bool dot, float length, char sign) {
        add_key(key, dot, length, sign, m_setting.m_quantity);
    }
    void add_key(int key, bool dot, float length, char sign, int quantity) {
        add_key(m_setting.m_tone, key, dot, length, sign, quantity);
    }
    void add_key(int tone, int key, bool dot, float length, char sign,
                 int quantity)
    {
        if (key == 96) {
            key = 0;
        }
        VskNote note(m_setting.m_tempo, 0,
            tone, 0, dot, length, sign, m_setting.m_volume, quantity);
        note.m_key = key;
        m_notes.push_back(note);
    }

    void calc_total();
    void realize(VskSoundPlayer *player);
    void destroy();
}; // struct VskPhrase

//////////////////////////////////////////////////////////////////////////////
// VskScoreBlock

typedef std::vector<shared_ptr<VskPhrase>>  VskScoreBlock;

//////////////////////////////////////////////////////////////////////////////

struct VskSoundPlayer {
    bool                                m_playing_music;
    PE_event                            m_stopping_event;
    std::deque<VskScoreBlock>           m_melody_line;
    std::mutex                          m_lock;
    std::vector<shared_ptr<VskNote>>    m_notes;
    YM2203                              m_ym;

    VskSoundPlayer() : m_playing_music(false),
                       m_stopping_event(false, false) { init_beep(); }

    virtual ~VskSoundPlayer() {
        free_beep();
    }

    void play(VskScoreBlock& block);
    bool wait_for_stop(uint32_t milliseconds = -1);
    bool play_and_wait(VskScoreBlock& block, uint32_t milliseconds = -1);
    void stop();

    void beep(int i);

protected:
    ALuint  m_beep_buffer;
    ALuint  m_beep_source;

    void init_beep();
    void free_beep();
}; // struct VskSoundPlayer

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef FMGON_SOUNDPLAYER_H
