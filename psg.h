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
//  PSG に良く似た音を生成する音源ユニット
//  
//  interface:
//  bool SetClock(uint32_t clock, uint32_t rate)
//      初期化．このクラスを使用する前にかならず呼んでおくこと．
//      PSG のクロックや PCM レートを設定する
//
//      clock:  PSG の動作クロック
//      rate:   生成する PCM のレート
//      retval  初期化に成功すれば true
//
//  void Mix(Sample* dest, int nsamples)
//      PCM を nsamples 分合成し， dest で始まる配列に加える(加算する)
//      あくまで加算なので，最初に配列をゼロクリアする必要がある
//  
//  void Reset()
//      リセットする
//
//  void SetReg(uint32_t reg, uint8_t data)
//      レジスタ reg に data を書き込む
//  
//  uint32_t GetReg(uint32_t reg)
//      レジスタ reg の内容を読み出す
//  
//  void SetVolume(int db)
//      各音源の音量を調節する
//      単位は約 1/2 dB
//
class PSG {
public:
    typedef PSG_SAMPLETYPE Sample;
    
    enum {
        noisetablesize = 1 << 11,   // ←メモリ使用量を減らしたいなら減らして
        toneshift = 24,
        envshift = 22,
        noiseshift = 14,
        oversampling = 2,       // ← 音質より速度が優先なら減らすといいかも
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
