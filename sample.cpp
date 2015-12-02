#include "stdafx.h"
#include "YM2203.h"

#define CLOCK       8000000
#define SAMPLERATE  44100

#define ALUT_STATIC
#include <AL/alut.h>

#if defined(_WIN32) && defined(_MSC_VER)
    #pragma comment(lib, "OpenAL32.lib")
    #pragma comment(lib, "alut.lib")
#endif

static const short table[5][10] = {
    {58, 15, 0, 0, 0, 0, 0, 0, 0, 0},
    {31, 12, 4, 10, 1, 32, 0, 12, 0, 0},
    {31, 2, 4, 6, 15, 57, 3, 15, 1, 0},
    {31, 12, 4, 6, 0, 30, 0, 1, 0, 0},
    {31, 5, 7, 7, 2, 0, 2, 3, -1, 0},
};

int main(void) {
    YM2203 ym;
    ALuint buffers[1];
    ALuint sources[1];
    alutInit(NULL, NULL);
    alGenBuffers(1, buffers);
    alGenSources(1, sources);

    YM2203_Timbre timbre(table);

    int sec = 3;
    FM_SAMPLETYPE *buf = new FM_SAMPLETYPE[sec * SAMPLERATE * 2];
    memset(buf, 0, sizeof(FM_SAMPLETYPE) * sec * SAMPLERATE * 2);

    ym.init(CLOCK, SAMPLERATE);
    ym.set_timbre(FM_CH1, &timbre);
    ym.set_tone_or_noise(SSG_CH_A, TONE_MODE);
    ym.set_volume(SSG_CH_A, 15);
    ym.set_volume(FM_CH1, 15);

    ym.set_pitch(FM_CH1, 4, KEY_C);
    ym.set_pitch(SSG_CH_A, 5, KEY_C);

    ym.note_on(FM_CH1);
    ym.note_on(SSG_CH_A);
    ym.mix(buf, SAMPLERATE);
    ym.count(1 * 1000 * 1000);
    ym.note_off(FM_CH1);
    ym.note_off(SSG_CH_A);

    ym.set_pitch(FM_CH1, 4, KEY_D);
    ym.set_pitch(SSG_CH_A, 5, KEY_D);

    ym.note_on(FM_CH1);
    ym.note_on(SSG_CH_A);
    ym.mix(buf + SAMPLERATE * 1 * 2, SAMPLERATE);
    ym.count(1 * 1000 * 1000);
    ym.note_off(FM_CH1);
    ym.note_off(SSG_CH_A);

    ym.set_pitch(FM_CH1, 4, KEY_E);
    ym.set_pitch(SSG_CH_A, 5, KEY_E);

    ym.note_on(FM_CH1);
    ym.note_on(SSG_CH_A);
    ym.mix(buf + SAMPLERATE * 2 * 2, SAMPLERATE);
    ym.count(1 * 1000 * 1000);
    ym.note_off(FM_CH1);
    ym.note_off(SSG_CH_A);

    alBufferData(buffers[0], AL_FORMAT_STEREO16, buf,
        sizeof(FM_SAMPLETYPE) * sec * SAMPLERATE * 2,
        SAMPLERATE);
    alSourcei(sources[0], AL_BUFFER, buffers[0]);
    alSourcePlay(sources[0]);
    alutSleep(4);

    alDeleteSources(1, sources);
    alDeleteBuffers(1, buffers);
    delete[] buf;
    alutExit();
    return 0;
} // main
