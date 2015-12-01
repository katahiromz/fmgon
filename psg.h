// ---------------------------------------------------------------------------
//  PSG-like sound generator
//  Copyright (C) cisc 1997, 1999.
// ---------------------------------------------------------------------------
//  $Id: psg.h,v 1.8 2003/04/22 13:12:53 cisc Exp $

#ifndef PSG_H
#define PSG_H

#define PSG_SAMPLETYPE      int16_t     // int32_t or int16_t

// ---------------------------------------------------------------------------
//  class PSG
//  PSG �ɗǂ��������𐶐����鉹�����j�b�g
//  
//  interface:
//  bool SetClock(uint32_t clock, uint32_t rate)
//      �������D���̃N���X���g�p����O�ɂ��Ȃ炸�Ă�ł������ƁD
//      PSG �̃N���b�N�� PCM ���[�g��ݒ肷��
//
//      clock:  PSG �̓���N���b�N
//      rate:   �������� PCM �̃��[�g
//      retval  �������ɐ�������� true
//
//  void Mix(Sample* dest, int nsamples)
//      PCM �� nsamples ���������C dest �Ŏn�܂�z��ɉ�����(���Z����)
//      �����܂ŉ��Z�Ȃ̂ŁC�ŏ��ɔz����[���N���A����K�v������
//  
//  void Reset()
//      ���Z�b�g����
//
//  void SetReg(uint32_t reg, uint8_t data)
//      ���W�X�^ reg �� data ����������
//  
//  uint32_t GetReg(uint32_t reg)
//      ���W�X�^ reg �̓��e��ǂݏo��
//  
//  void SetVolume(int db)
//      �e�����̉��ʂ𒲐߂���
//      �P�ʂ͖� 1/2 dB
//
class PSG {
public:
    typedef PSG_SAMPLETYPE Sample;
    
    enum {
        noisetablesize = 1 << 11,   // ���������g�p�ʂ����炵�����Ȃ猸�炵��
        toneshift = 24,
        envshift = 22,
        noiseshift = 14,
        oversampling = 2,       // �� ������葬�x���D��Ȃ猸�炷�Ƃ�������
    };

public:
    PSG();
    ~PSG();

    void        Mix(Sample* dest, int nsamples);
    void        SetClock(int clock, int rate);
    
    void        SetVolume(int vol);
    void        SetChannelMask(int c);
    
    void        Reset();
    void        SetReg(uint32_t regnum, uint8_t data);

    uint32_t    GetReg(uint32_t regnum) {
        return reg[regnum & 0x0f];
    }

protected:
    void        MakeNoiseTable();
    void        MakeEnvelopTable();
    static void StoreSample(Sample& dest, int32_t data);
    
    uint8_t             reg[16];

    const uint32_t*     envelop;
    uint32_t            olevel[3];
    uint32_t            scount[3], speriod[3];
    uint32_t            ecount, eperiod;
    uint32_t            ncount, nperiod;
    uint32_t            tperiodbase;
    uint32_t            eperiodbase;
    uint32_t            nperiodbase;
    int                 volume;
    int                 mask;

    static uint32_t     enveloptable[16][64];
    static uint32_t     noisetable[noisetablesize];
    static int          EmitTable[32];
}; // class PSG

#endif // PSG_H
