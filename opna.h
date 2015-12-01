// ---------------------------------------------------------------------------
//	OPN/A/B interface with ADPCM support
//	Copyright (C) cisc 1998, 2003.
// ---------------------------------------------------------------------------
//	$Id: opna.h,v 1.33 2003/06/12 13:14:37 cisc Exp $

#ifndef FM_OPNA_H
#define FM_OPNA_H

#include "fmgen.h"
#include "fmtimer.h"
#include "psg.h"

// ---------------------------------------------------------------------------
//	class OPN/OPNA
//	OPN/OPNA �ɗǂ��������𐶐����鉹�����j�b�g
//	
//	interface:
//	bool Init(uint32_t clock, uint32_t rate, bool, const char* path);
//		�������D���̃N���X���g�p����O�ɂ��Ȃ炸�Ă�ł������ƁD
//		OPNA �̏ꍇ�͂��̊֐��Ń��Y���T���v����ǂݍ���
//
//		clock:	OPN/OPNA/OPNB �̃N���b�N���g��(Hz)
//
//		rate:	�������� PCM �̕W�{���g��(Hz)
//
//		path:	���Y���T���v���̃p�X(OPNA �̂ݗL��)
//				�ȗ����̓J�����g�f�B���N�g������ǂݍ���
//				������̖����ɂ� '\' �� '/' �Ȃǂ����邱��
//
//		�Ԃ�l	�������ɐ�������� true
//
//	bool LoadRhythmSample(const char* path)
//		(OPNA ONLY)
//		Rhythm �T���v����ǂݒ����D
//		path �� Init �� path �Ɠ����D
//		
//	bool SetRate(uint32_t clock, uint32_t rate, bool)
//		�N���b�N�� PCM ���[�g��ύX����
//		�������� Init ���Q�Ƃ̂��ƁD
//	
//	void Mix(FM_SAMPLETYPE* dest, int nsamples)
//		Stereo PCM �f�[�^�� nsamples ���������C dest �Ŏn�܂�z���
//		������(���Z����)
//		�Edest �ɂ� sample*2 ���̗̈悪�K�v
//		�E�i�[�`���� L, R, L, R... �ƂȂ�D
//		�E�����܂ŉ��Z�Ȃ̂ŁC���炩���ߔz����[���N���A����K�v������
//		�EFM_SAMPLETYPE �� short �^�̏ꍇ�N���b�s���O���s����.
//		�E���̊֐��͉��������̃^�C�}�[�Ƃ͓Ɨ����Ă���D
//		  Timer �� Count �� GetNextEvent �ő��삷��K�v������D
//	
//	void Reset()
//		���������Z�b�g(������)����
//
//	void SetReg(uint32_t reg, uint32_t data)
//		�����̃��W�X�^ reg �� data ����������
//	
//	uint32_t GetReg(uint32_t reg)
//		�����̃��W�X�^ reg �̓��e��ǂݏo��
//		�ǂݍ��ނ��Ƃ��o���郌�W�X�^�� PSG, ADPCM �̈ꕔ�CID(0xff) �Ƃ�
//	
//	uint32_t ReadStatus()/ReadStatusEx()
//		�����̃X�e�[�^�X���W�X�^��ǂݏo��
//		ReadStatusEx �͊g���X�e�[�^�X���W�X�^�̓ǂݏo��(OPNA)
//		busy �t���O�͏�� 0
//	
//	bool Count(uint32_t t)
//		�����̃^�C�}�[�� t [�ʕb] �i�߂�D
//		�����̓�����Ԃɕω�����������(timer �I�[�o�[�t���[)
//		true ��Ԃ�
//
//	uint32_t GetNextEvent()
//		�����̃^�C�}�[�̂ǂ��炩���I�[�o�[�t���[����܂łɕK�v��
//		����[�ʕb]��Ԃ�
//		�^�C�}�[����~���Ă���ꍇ�� ULONG_MAX ��Ԃ��c �Ǝv��
//	
//	void SetVolumeFM(int db)/SetVolumePSG(int db) ...
//		�e�����̉��ʂ��{�|�����ɒ��߂���D�W���l�� 0.
//		�P�ʂ͖� 1/2 dB�C�L���͈͂̏���� 20 (10dB)
//
namespace FM
{
	//	OPN Base -------------------------------------------------------
	class OPNBase : public Timer
	{
	public:
		OPNBase();
		
		bool	Init(uint32_t c, uint32_t r);
		virtual void Reset();
		
		void	SetVolumeFM(int db);
		void	SetVolumePSG(int db);
		void	SetLPFCutoff(uint32_t freq) {}	// obsolete

	protected:
		void	SetParameter(Channel4* ch, uint32_t addr, uint32_t data);
		void	SetPrescaler(uint32_t p);
		void	RebuildTimeTable();
		
		int		fmvolume;
		
		uint32_t	clock;				// OPN �N���b�N
		uint32_t	rate;				// FM �����������[�g
		uint32_t	psgrate;			// FMGen  �o�̓��[�g
		uint32_t	status;
		Channel4* csmch;
		

		static  uint32_t lfotable[8];
	
	private:
		void	TimerA();
		uint8_t	prescale;
		
	protected:
		Chip	chip;
		PSG		psg;
	};

	//	OPN2 Base ------------------------------------------------------
	class OPNABase : public OPNBase
	{
	public:
		OPNABase();
		~OPNABase();
		
		uint32_t	ReadStatus() { return status & 0x03; }
		uint32_t	ReadStatusEx();
		void	SetChannelMask(uint32_t mask);
	
	private:
		virtual void Intr(bool) {}

		void	MakeTable2();
	
	protected:
		bool	Init(uint32_t c, uint32_t r, bool);
		bool	SetRate(uint32_t c, uint32_t r, bool);

		void	Reset();
		void 	SetReg(uint32_t addr, uint32_t data);
		void	SetADPCMBReg(uint32_t reg, uint32_t data);
		uint32_t	GetReg(uint32_t addr);	
	
	protected:
		void	FMMix(Sample* buffer, int nsamples);
		void 	Mix6(Sample* buffer, int nsamples, int activech);
		
		void	MixSubS(int activech, ISample**);
		void	MixSubSL(int activech, ISample**);

		void	SetStatus(uint32_t bit);
		void	ResetStatus(uint32_t bit);
		void	UpdateStatus();
		void	LFO();

		void	DecodeADPCMB();
		void	ADPCMBMix(Sample* dest, uint32_t count);

		void	WriteRAM(uint32_t data);
		uint32_t	ReadRAM();
		int		ReadRAMN();
		int		DecodeADPCMBSample(uint32_t);
		
	// FM �����֌W
		uint8_t	pan[6];
		uint8_t	fnum2[9];
		
		uint8_t	reg22;
		uint32_t	reg29;		// OPNA only?
		
		uint32_t	stmask;
		uint32_t	statusnext;

		uint32_t	lfocount;
		uint32_t	lfodcount;
		
		uint32_t	fnum[6];
		uint32_t	fnum3[3];
		
	// ADPCM �֌W
		uint8_t*	adpcmbuf;		// ADPCM RAM
		uint32_t	adpcmmask;		// �������A�h���X�ɑ΂���r�b�g�}�X�N
		uint32_t	adpcmnotice;	// ADPCM �Đ��I�����ɂ��r�b�g
		uint32_t	startaddr;		// Start address
		uint32_t	stopaddr;		// Stop address
		uint32_t	memaddr;		// �Đ����A�h���X
		uint32_t	limitaddr;		// Limit address/mask
		int		adpcmlevel;		// ADPCM ����
		int		adpcmvolume;
		int		adpcmvol;
		uint32_t	deltan;			// ��N
		int		adplc;			// ���g���ϊ��p�ϐ�
		int		adpld;			// ���g���ϊ��p�ϐ������l
		uint32_t	adplbase;		// adpld �̌�
		int		adpcmx;			// ADPCM �����p x
		int		adpcmd;			// ADPCM �����p ��
		int		adpcmout;		// ADPCM ������̏o��
		int		apout0;			// out(t-2)+out(t-1)
		int		apout1;			// out(t-1)+out(t)

		uint32_t	adpcmreadbuf;	// ADPCM ���[�h�p�o�b�t�@
		bool	adpcmplay;		// ADPCM �Đ���
		int8_t	granuality;		
		bool	adpcmmask_;

		uint8_t	control1;		// ADPCM �R���g���[�����W�X�^�P
		uint8_t	control2;		// ADPCM �R���g���[�����W�X�^�Q
		uint8_t	adpcmreg[8];	// ADPCM ���W�X�^�̈ꕔ��

		int		rhythmmask_;

		Channel4 ch[6];

		static void	BuildLFOTable();
		static int amtable[FM_LFOENTS];
		static int pmtable[FM_LFOENTS];
		static int32_t tltable[FM_TLENTS+FM_TLPOS];
		static bool	tablehasmade;
	};

	//	YM2203(OPN) ----------------------------------------------------
	class OPN : public OPNBase
	{
	public:
		OPN();
		virtual ~OPN() {}
		
		bool	Init(uint32_t c, uint32_t r, bool=false, const char* =0);
		bool	SetRate(uint32_t c, uint32_t r, bool=false);
		
		void	Reset();
		void 	Mix(Sample* buffer, int nsamples);
		void 	SetReg(uint32_t addr, uint32_t data);
		uint32_t	GetReg(uint32_t addr);
		uint32_t	ReadStatus() { return status & 0x03; }
		uint32_t	ReadStatusEx() { return 0xff; }
		
		void	SetChannelMask(uint32_t mask);
		
		int		dbgGetOpOut(int c, int s) { return ch[c].op[s].dbgopout_; }
		int		dbgGetPGOut(int c, int s) { return ch[c].op[s].dbgpgout_; }
		Channel4* dbgGetCh(int c) { return &ch[c]; }
	
	private:
		virtual void Intr(bool) {}
		
		void	SetStatus(uint32_t bit);
		void	ResetStatus(uint32_t bit);
		
		uint32_t	fnum[3];
		uint32_t	fnum3[3];
		uint8_t	fnum2[6];
		
		Channel4 ch[3];
	};

	//	YM2608(OPNA) ---------------------------------------------------
	class OPNA : public OPNABase
	{
	public:
		OPNA();
		virtual ~OPNA();
		
		bool	Init(uint32_t c, uint32_t r, bool  = false, const char* rhythmpath=0);
		bool	LoadRhythmSample(const char*);
	
		bool	SetRate(uint32_t c, uint32_t r, bool = false);
		void 	Mix(Sample* buffer, int nsamples);

		void	Reset();
		void 	SetReg(uint32_t addr, uint32_t data);
		uint32_t	GetReg(uint32_t addr);

		void	SetVolumeADPCM(int db);
		void	SetVolumeRhythmTotal(int db);
		void	SetVolumeRhythm(int index, int db);

		uint8_t*	GetADPCMBuffer() { return adpcmbuf; }

		int		dbgGetOpOut(int c, int s) { return ch[c].op[s].dbgopout_; }
		int		dbgGetPGOut(int c, int s) { return ch[c].op[s].dbgpgout_; }
		Channel4* dbgGetCh(int c) { return &ch[c]; }

		
	private:
		struct Rhythm
		{
			uint8_t	pan;		// �ς�
			int8_t	level;		// �����傤
			int		volume;		// �����傤�����Ă�
			int16_t*	sample;		// ����Ղ�
			uint32_t	size;		// ������
			uint32_t	pos;		// ����
			uint32_t	step;		// ���Ă��Ղ�
			uint32_t	rate;		// ����Ղ�̂�[��
		};
	
		void	RhythmMix(Sample* buffer, uint32_t count);

	// ���Y�������֌W
		Rhythm	rhythm[6];
		int8_t	rhythmtl;		// ���Y���S�̂̉���
		int		rhythmtvol;		
		uint8_t	rhythmkey;		// ���Y���̃L�[
	};

	//	YM2610/B(OPNB) ---------------------------------------------------
	class OPNB : public OPNABase
	{
	public:
		OPNB();
		virtual ~OPNB();
		
		bool	Init(uint32_t c, uint32_t r, bool = false,
					 uint8_t *_adpcma = 0, int _adpcma_size = 0,
					 uint8_t *_adpcmb = 0, int _adpcmb_size = 0);
	
		bool	SetRate(uint32_t c, uint32_t r, bool = false);
		void 	Mix(Sample* buffer, int nsamples);

		void	Reset();
		void 	SetReg(uint32_t addr, uint32_t data);
		uint32_t	GetReg(uint32_t addr);
		uint32_t	ReadStatusEx();

		void	SetVolumeADPCMATotal(int db);
		void	SetVolumeADPCMA(int index, int db);
		void	SetVolumeADPCMB(int db);

//		void	SetChannelMask(uint32_t mask);
		
	private:
		struct ADPCMA
		{
			uint8_t	pan;		// �ς�
			int8_t	level;		// �����傤
			int		volume;		// �����傤�����Ă�
			uint32_t	pos;		// ����
			uint32_t	step;		// ���Ă��Ղ�

			uint32_t	start;		// �J�n
			uint32_t	stop;		// �I��
			uint32_t	nibble;		// ���� 4 bit
			int		adpcmx;		// �ϊ��p
			int		adpcmd;		// �ϊ��p
		};
	
		int		DecodeADPCMASample(uint32_t);
		void	ADPCMAMix(Sample* buffer, uint32_t count);
		static void InitADPCMATable();
		
	// ADPCMA �֌W
		uint8_t*	adpcmabuf;		// ADPCMA ROM
		int		adpcmasize;
		ADPCMA	adpcma[6];
		int8_t	adpcmatl;		// ADPCMA �S�̂̉���
		int		adpcmatvol;		
		uint8_t	adpcmakey;		// ADPCMA �̃L�[
		int		adpcmastep;
		uint8_t	adpcmareg[32];
 
		static int jedi_table[(48+1)*16];

		Channel4 ch[6];
	};

	//	YM2612/3438(OPN2) ----------------------------------------------------
	class OPN2 : public OPNBase
	{
	public:
		OPN2();
		virtual ~OPN2() {}
		
		bool	Init(uint32_t c, uint32_t r, bool=false, const char* =0);
		bool	SetRate(uint32_t c, uint32_t r, bool);
		
		void	Reset();
		void 	Mix(Sample* buffer, int nsamples);
		void 	SetReg(uint32_t addr, uint32_t data);
		uint32_t	GetReg(uint32_t addr);
		uint32_t	ReadStatus() { return status & 0x03; }
		uint32_t	ReadStatusEx() { return 0xff; }
		
		void	SetChannelMask(uint32_t mask);
		
	private:
		virtual void Intr(bool) {}
		
		void	SetStatus(uint32_t bit);
		void	ResetStatus(uint32_t bit);
		
		uint32_t	fnum[3];
		uint32_t	fnum3[3];
		uint8_t	fnum2[6];
		
	// ���`��ԗp���[�N
		int32_t	mixc, mixc1;
		
		Channel4 ch[3];
	};
}

// ---------------------------------------------------------------------------

inline void FM::OPNBase::RebuildTimeTable()
{
	int p = prescale;
	prescale = -1;
	SetPrescaler(p);
}

inline void FM::OPNBase::SetVolumePSG(int db)
{
	psg.SetVolume(db);
}

#endif // FM_OPNA_H
