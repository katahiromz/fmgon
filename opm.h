// ---------------------------------------------------------------------------
//  OPM-like Sound Generator
//  Copyright (C) cisc 1998, 2003.
// ---------------------------------------------------------------------------
//  $Id: opm.h,v 1.14 2003/06/07 08:25:53 cisc Exp $

#ifndef FMGON_OPM_H
#define FMGON_OPM_H

#include "fmgen.h"
#include "fmtimer.h"
#include "psg.h"

// ---------------------------------------------------------------------------
//  class OPM
//  OPM �ɗǂ�����(?)���𐶐����鉹�����j�b�g
//  
//  interface:
//  bool Init(uint32_t clock, uint32_t rate, bool);
//      �������D���̃N���X���g�p����O�ɂ��Ȃ炸�Ă�ł������ƁD
//      ����: ���`�⊮���[�h�͔p�~����܂���
//
//      clock:  OPM �̃N���b�N���g��(Hz)
//
//      rate:   �������� PCM �̕W�{���g��(Hz)
//
//              
//      �Ԓl    �������ɐ�������� true
//
//  bool SetRate(uint32_t clock, uint32_t rate, bool)
//      �N���b�N�� PCM ���[�g��ύX����
//      �������� Init �Ɠ��l�D
//  
//  void Mix(Sample* dest, int nsamples)
//      Stereo PCM �f�[�^�� nsamples ���������C dest �Ŏn�܂�z���
//      ������(���Z����)
//      �Edest �ɂ� sample*2 ���̗̈悪�K�v
//      �E�i�[�`���� L, R, L, R... �ƂȂ�D
//      �E�����܂ŉ��Z�Ȃ̂ŁC���炩���ߔz����[���N���A����K�v������
//      �EFM_SAMPLETYPE �� short �^�̏ꍇ�N���b�s���O���s����.
//      �E���̊֐��͉��������̃^�C�}�[�Ƃ͓Ɨ����Ă���D
//        Timer �� Count �� GetNextEvent �ő��삷��K�v������D
//  
//  void Reset()
//      ���������Z�b�g(������)����
//
//  void SetReg(uint32_t reg, uint32_t data)
//      �����̃��W�X�^ reg �� data ����������
//  
//  uint32_t ReadStatus()
//      �����̃X�e�[�^�X���W�X�^��ǂݏo��
//      busy �t���O�͏�� 0
//  
//  bool Count(uint32_t t)
//      �����̃^�C�}�[�� t [10^(-6) �b] �i�߂�D
//      �����̓�����Ԃɕω�����������(timer �I�[�o�[�t���[)
//      true ��Ԃ�
//
//  uint32_t GetNextEvent()
//      �����̃^�C�}�[�̂ǂ��炩���I�[�o�[�t���[����܂łɕK�v��
//      ����[�ʕb]��Ԃ�
//      �^�C�}�[����~���Ă���ꍇ�� 0 ��Ԃ��D
//  
//  void SetVolume(int db)
//      �e�����̉��ʂ��{�|�����ɒ��߂���D�W���l�� 0.
//      �P�ʂ͖� 1/2 dB�C�L���͈͂̏���� 20 (10dB)
//
//  ���z�֐�:
//  virtual void Intr(bool irq)
//      IRQ �o�͂ɕω����������ꍇ�Ă΂��D
//      irq = true:  IRQ �v��������
//      irq = false: IRQ �v����������
//
namespace FM
{
    //  YM2151(OPM) ----------------------------------------------------
    class OPM : public Timer
    {
    public:
        OPM();
        ~OPM() {}

        bool        Init(uint32_t c, uint32_t r, bool=false);
        bool        SetRate(uint32_t c, uint32_t r, bool);
        void        SetLPFCutoff(uint32_t freq);
        void        Reset();
        
        void        SetReg(uint32_t addr, uint32_t data);
        uint32_t    GetReg(uint32_t addr);
        uint32_t    ReadStatus() {
            return (status & 0x03);
        }
        
        void        Mix(Sample* buffer, int nsamples);
        
        void        SetVolume(int db);
        void        SetChannelMask(uint32_t mask);
        
    private:
        virtual void Intr(bool) {}
    
    private:
        enum
        {
            OPM_LFOENTS = 512,
        };
        
        void        SetStatus(uint32_t bit);
        void        ResetStatus(uint32_t bit);
        void        SetParameter(uint32_t addr, uint32_t data);
        void        TimerA();
        void        RebuildTimeTable();
        void        MixSub(int activech, ISample**);
        void        MixSubL(int activech, ISample**);
        void        LFO();
        uint32_t    Noise();
        
        int         fmvolume;

        uint32_t    clock;
        uint32_t    rate;
        uint32_t    pcmrate;

        uint32_t    pmd;
        uint32_t    amd;
        uint32_t    lfocount;
        uint32_t    lfodcount;

        uint32_t    lfo_count_;
        uint32_t    lfo_count_diff_;
        uint32_t    lfo_step_;
        uint32_t    lfo_count_prev_;

        uint32_t    lfowaveform;
        uint32_t    rateratio;
        uint32_t    noise;
        int32_t     noisecount;
        uint32_t    noisedelta;
        
        bool        interpolation;
        uint8_t     lfofreq;
        uint8_t     status;
        uint8_t     reg01;

        uint8_t     kc[8];
        uint8_t     kf[8];
        uint8_t     pan[8];

        Channel4    ch[8];
        Chip        chip;

        static void BuildLFOTable();
        static int amtable[4][OPM_LFOENTS];
        static int pmtable[4][OPM_LFOENTS];

    public:
        int     dbgGetOpOut(int c, int s) {
            return ch[c].op[s].dbgopout_;
        }
        Channel4* dbgGetCh(int c) {
            return &ch[c];
        }
    };
} // namespace FM

#endif  // ndef FMGON_OPM_H
