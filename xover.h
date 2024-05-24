//
//  xover.h
//  Fancy Crossover for any type of intrinsic vector (double8, double4, double2, etc..)
//
//  Created by DemoSense on 1/25/24.
//  Copyright © 2024 DSP VooDoo, Inc. All rights reserved.
//

#pragma once

typedef double double2 __attribute__((vector_size(sizeof(double)*2))) __attribute__((aligned(16)));
typedef double double4 __attribute__((vector_size(sizeof(double)*4))) __attribute__((aligned(16)));
typedef double double8 __attribute__((vector_size(sizeof(double)*8))) __attribute__((aligned(16)));

class const1 {
public:
    double v;
    constexpr const1 (double value) : v(value) {}
    inline operator double2() const {
        return double2{v, v};
    }
    inline operator double4() const {
        return double4{v, v, v, v};
    }
    inline operator double8() const {
        return double8{v, v, v, v, v, v, v, v};
    }
    inline constexpr const1 operator-() const { return const1(-v); }
};

inline constexpr const1 operator""_v(long double d) {
    return const1(double(d));
}

template <typename T, int srate, int bands, const1 Q = 0.25f>
class xoverT
{
    static const int MAX_BANDS = 4;
    static const int MAX_FLT = MAX_BANDS * 2;
    
    typedef struct {
        double k;
        double g;
        T sT[2];
    } linear_svf;
    
public:
    
    xoverT() {
        for (int b=0; b <= bands; b++) {
            freq[b]     = 1.;
            level[b]    = 1.;
            ku[b]       = Q;
        }
        // terminate upper w/ srate/2
        if (srate == MSS_MIXRATE * 4)
            set_freq(bands,srate/8);
        else
            set_freq(bands,srate/2);
    }
    
    __inline void reset()
    {
        for (int b=0; b <= bands; b++)
            calc_lr4(b, freq[b], ku[b]);
        
        memset(out, 0, sizeof(out));
    }
    
    // b = band, f = frequency, iku = resonance
    __inline void set_freq(unsigned b, float f, float q = Q)
    {
        if (b <= 0 || b > bands)
            return;
        freq[b-1] = f;
        ku[b-1] = q;
        
        reset();
    }
    
    // gain per band (b = band, l = level)
    __inline void set_level(int b, float l) {
        if (b <= 0 || b > bands)
            return;
        level[b-1] = l;
    }
    
    // main procsesing function
    void process(T LR)
    {
        T loT;
        for (int b=0; b <= bands; b++)
        {
            if (b < bands) {
                run_lr4(b, LR, loT, LR);
                out[b] = loT;
            } else
                out[b] = LR;
        }
    }

    __inline T get_output(unsigned b) 
    {
        if (b <= 0 || b > bands)
            return 0.0;
        return out[b-1] * level[b-1];
    }
    
protected:
    
    double     freq[MAX_BANDS+1], level[MAX_BANDS+1], ku[MAX_BANDS+1];
    linear_svf smp1[MAX_FLT], smp2[MAX_FLT];
    T           out[MAX_BANDS+1];
    
    __inline T run_linear_svf_xover(linear_svf *svf, const T &in, float mixlow, float mixhigh)
    {
        T v1, v2, v3;
        const double g = svf->g;
        const double k = svf->k;
        const T s0 = (&svf[0])->sT[0];
        const T s1 = (&svf[1])->sT[1];
        const double g2 = g * g;
        const T vhigh = in * mixhigh;
        const T vband = in * 0.75;
        const T vlow = in * mixlow;
        
        v1 = in;
        v2 = -1. / (1. + g2 + g*k) * (-s0 + g*s1 - g*k*s0 + g2*vband + g*vhigh - g*vlow - g2*k*vlow);
        v3 = -1. / (1. + g2 + g*k) * (-g*s0 - s1 - g*vband + g2*vhigh + g*k*vhigh - g2*vlow);
        (&svf[0])->sT[0] = 2. * v2- s0;
        (&svf[1])->sT[1] = 2. * v3 - s1;
        
        return vhigh + v3;
    }
    
    __inline void run_lr4(unsigned band, const T &in, T &outlo, T &outhi)
    {
        outlo = run_linear_svf_xover(&smp1[band], in, 1., 0.);
        outhi = run_linear_svf_xover(&smp2[band], in, 0., 1.);
    }
    
    __inline void linear_svf_set_xover(linear_svf *svf, float cutoff, float q)
    {
        svf->k = 2.-2.*q;
        svf->g = tan(M_PI * cutoff / (float)srate);
        svf->sT[0] = svf->sT[1] = 0.0;
    }
    
    __inline void calc_lr4(unsigned band, float f, float q)
    {
        linear_svf_set_xover(&smp1[band], f, q);
        linear_svf_set_xover(&smp2[band], f, q);
    }
};
