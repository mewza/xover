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


template <typename T, int srate>
class xoverT
{
    static const int MAX_BANDS = 3;
    static const int MAX_FLT = 6;
    
    typedef struct {
        double k;
        double g;
        double s[2];
        T sT[2];
    } linear_svf;
    
public:
    
    xoverT() {
        bands = -1;
    }
    
    // NOTE: must be followed by a reset()
    __inline void Init(int b, float lvl = 1.0)
    {
        bands = F_MIN(MAX_BANDS, b + 1);
        
        // enable all bands initially
        for (int b=0; b <= bands; b++) {
            freq[b]     = 0;
            active[b]   = true;
            level[b]    = lvl;
            ku[b]       = 0.25;
        }
        // LOGGER("Init( %d bands ) crossover", bands);
        // active[0] = true;
    }

    // should call reset() after Init()
    __inline void reset()
    {
        for (int i=0; i<3; i++) {
            calc_lr4( freq[i], ku[i], 0, i*2 );
            calc_lr4( freq[i], ku[i], 1, i*2+1 );
        }
        memset(so, 0, sizeof(so));
        memset(out, 0, sizeof(out));
        iT = 0.0_v;
    }
    
    // b = band, f = frequency, iku = resonance
    __inline void set_freq(int b, double f, float iku = 0.25)
    {
        if (b < 0 || b >= bands)
            return;
        freq[b] = f;
        ku[b] = iku;
       // LOGGER("set_freq( %d ): %.3f q: %.3f", b, f, q);
    }
    
    // toggle per band (b = band, a = active)
    __inline void set_active(int b, bool a)
    {
        if (b < 0 || b > bands)
            return;
        active[b] = a;
    }

    // gain per band (b = band, l = level)
    __inline void set_level(int b, double l) {
        if (b < 0 || b > bands)
            return;
        level[b] = l;
    }
    
    // main procsesing function
    void process_T(T LR)
    {
        inT = LR;
        
        iT = 0.0_v;
        for(int b=0; b <= bands; b++)
        {
            int b2=b<<1;
            if (b < bands) {
                run_lr4_T(b2, 0, inT, soT, inT);
                LR = soT; iT = inT;
                out[b] = LR * (active[b] ? level[b] : 0.0) ;
            } else {
                out[b] = iT * (active[b] ? level[b] : 0.0) ;
            }
        }
    }
    
    // main procsesing function
    void process(T LR)
    {
        in[0] = LR[0]; in[1] = LR[1];
        in[2] = LR[2]; in[3] = LR[3];
        in[4] = LR[4]; in[5] = LR[5];
        
        iT = 0.0_v;
        for(int b=0; b <= bands; b++)
        {
            int b2=b*2;
            if (b < bands) {
                run_lr4(b2,   0, in[0], &so[0], &in[0]);
                run_lr4(b2+1, 0, in[1], &so[1], &in[1]);
                
                run_lr4(b2,   1, in[2], &so[2], &in[2]);
                run_lr4(b2+1, 1, in[3], &so[3], &in[3]);
                
                run_lr4(b2,   2, in[4], &so[4], &in[4]);
                run_lr4(b2+1, 2, in[5], &so[5], &in[5]);
                
                LR = T{ so[0], so[1], so[2], so[3], so[4], so[5], 0, 0 };
                iT = T{ in[0], in[1], in[2], in[3], in[4], in[5], 0, 0 };
                
                out[b] = LR * (active[b] ? level[b] : 0.0) ;
            } else {
                out[b] = iT * (active[b] ? level[b] : 0.0) ;
            }
        }
    }

    // access to output per band
    __inline T get_output(int b) {
        return out[b];
    }
    
protected:
    
    double  freq[MAX_BANDS+1],
            level[MAX_BANDS+1],
            ku[MAX_BANDS+1];
    int     bands, active[MAX_BANDS+1];
    linear_svf  smp1[3][2][MAX_FLT],
                smp2[3][2][MAX_FLT];
    double  in[MAX_FLT],
            so[MAX_FLT];
    T       iT, inT, soT,
            out[MAX_BANDS+1];
    
    __inline double run_linear_svf_xover(linear_svf *svf, double in, double mixlow, double mixhigh)
    {
        double v1, v2, v3;
        double g = svf->g;
        double k = svf->k;
        double s0 = svf->s[0];
        double s1 = svf->s[1];
        double g2 = g * g;
        double vhigh = in * mixhigh;
        double vband = in * 0.75;
        double vlow = in * mixlow;
        
        v1 = in;
        v2 = -1. / (1. + g2 + g*k) * (-s0 + g*s1 - g*k*s0 + g2*vband + g*vhigh - g*vlow - g2*k*vlow);
        v3 = -1. / (1. + g2 + g*k) * (-g*s0 - s1 - g*vband + g2*vhigh + g*k*vhigh - g2*vlow);
        svf->s[0] = 2. * v2- s0;
        svf->s[1] = 2. * v3 - s1;
        
        return (double)(vhigh + v3);
    }

    __inline T run_linear_svf_xover_T(linear_svf *svf, T in, double mixlow, double mixhigh)
    {
        T v1, v2, v3;
        double g = svf->g;
        double k = svf->k;
        T s0 = (&svf[0])->sT[0];
        T s1 = (&svf[1])->sT[1];
        double g2 = g * g;
        T vhigh = in * mixhigh;
        T vband = in * 0.75;
        T vlow = in * mixlow;
        
        v1 = in;
        v2 = -1. / (1. + g2 + g*k) * (-s0 + g*s1 - g*k*s0 + g2*vband + g*vhigh - g*vlow - g2*k*vlow);
        v3 = -1. / (1. + g2 + g*k) * (-g*s0 - s1 - g*vband + g2*vhigh + g*k*vhigh - g2*vlow);
        (&svf[0])->sT[0] = 2. * v2- s0;
        (&svf[1])->sT[1] = 2. * v3 - s1;
        
        return vhigh + v3;
    }

    __inline void run_lr4_T(int band, int j, const T &in, T &outlo, T &outhi)
    {
        outlo = run_linear_svf_xover_T(&smp1[j][0][band], in, 1., 0.);
        outhi = run_linear_svf_xover_T(&smp2[j][1][band], in, 0., 1.);
    }
    
    __inline void run_lr4(int k, int j, double in, double *outlo, double *outhi)
    {
        *outlo = run_linear_svf_xover(&smp1[j][0][k], in, 1., 0.);
        *outhi = run_linear_svf_xover(&smp2[j][1][k], in, 0., 1.);
    }
    
    __inline void linear_svf_set_xover(linear_svf *svf, double cutoff, double resonance)
    {
        svf->k = 2. - 2. * resonance;
        svf->g = tan(M_PI * cutoff / (double)srate);
        svf->s[0] = svf->s[1] = 0.0;
        svf->sT[0] = svf->sT[1] = 0.0_v;
    }
    __inline void calc_lr4(double f, double q, int i, int k)
    {
        linear_svf_set_xover(&smp1[i][0][k], f, q);
        linear_svf_set_xover(&smp2[i][1][k], f, q);
    }
};

