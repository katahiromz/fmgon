//////////////////////////////////////////////////////////////////////////////
// fmtest.cpp --- test program of fmgon
// Copyright (C) 2015 Katayama Hirofumi MZ. All Rights Reserved.
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "soundplayer.h"

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
    alutInit(NULL, NULL);

    auto phrase = make_shared<VskPhrase>();

    phrase->m_setting.m_fm = true;
    if (argc <= 1) {
        phrase->m_setting.m_tone = 15;  // @15 DESCENT
    } else {
        phrase->m_setting.m_tone = atoi(argv[1]);
    }

#if 1
    // CMD PLAY "@15T150L8O2CEGO3CEGO4CRRCO3GECO2GECR"
    phrase->m_setting.m_tempo = 150;
    // NOTE: 24 is the length of a quarter note

    phrase->m_setting.m_length = 12;
    phrase->m_setting.m_octave = 3;
    phrase->add_note('C');
    phrase->add_note('E');
    phrase->add_note('G');
    phrase->m_setting.m_octave = 4;
    phrase->add_note('C');
    phrase->add_note('E');
    phrase->add_note('G');
    phrase->m_setting.m_octave = 5;
    phrase->add_note('C');
    phrase->add_note('R');
    phrase->add_note('R');
    phrase->add_note('C');
    phrase->m_setting.m_octave = 4;
    phrase->add_note('G');
    phrase->add_note('E');
    phrase->add_note('C');
    phrase->m_setting.m_octave = 3;
    phrase->add_note('G');
    phrase->add_note('E');
    phrase->add_note('C');
    phrase->add_note('R');
#else
    // KAERU NO UTA
    // CMD PLAY "@15T150O4L4CDEFEDCREFGAGFERCRCRCRCRL8CCDDEEFFL4EDCR"
    phrase->m_setting.m_tempo = 120;
    phrase->m_setting.m_octave = 3;
    // NOTE: 24 is the length of a quarter note
    phrase->m_setting.m_length = 24;

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
#endif

    VskScoreBlock block;
    block.push_back(phrase);

    VskSoundPlayer player;
    player.play_and_wait(block);

    alutExit();

    return 0;
} // main

//////////////////////////////////////////////////////////////////////////////
