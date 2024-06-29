As the description says, xoverT is a templated C++ class that implements a
multiband crossover utilizing SVF filters (see below) and up to 4 bands are supported.
Because it is templated C++ class it supports SIMD intrinsict vector construct.

For information on SVF (state variable filters) go to this site:
http://www.cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf

This is a sophisticated and better sounding algorithm than the 
common nested LP/HP filters... and now how to use it:

    // for iOS and OS X include simd for simd_double8, ..4, ..2 vectors
    // already defined and many math functions provided via simd::sinf(some_simd_vector)
    // for example. If you're going to use simd_double8 vectors, make sure to change 
    // xover.h defines to reflect that... left as excercise for the user :)
    
    #include <simd/simd.h>
    
    // 4 = number of bands (got moved from Init into template parameter
    // There is no longer need to call Init, and has been simply removed into
    // the constructor, and the N of bands is now a template parameter for easy use
    
    xoverT<double8,44100,4> cross;

in the further init or reset:

    // setup the bands 1-85, 85-700, 700-3k, 3k-22kHz
    
    // NOTE: indexing into bands has been changed from 0..N-1 
    // to 1..N. So for 4-band crossover, you have 3 split points.
    // Also: you do not need to make upper band split point it is done
    // for you and set to a niquist frequency, that is samplerate/2
    // Call this only once in the app per instantiation of xoverT or
    // if or when the crossover frequency changes, but it will call reset
    // and it will zero out the processed frame, just be aware of that
    
    cross.set_freq( 1, 85 );
    cross.set_freq( 2, 700 );
    cross.set_freq( 3, 3000 );

    // NOTE: you don't need to call cross.reset() it is called after each
    // set_freq() call. But you can anyway, it won't hurt it
    
    cross.reset();

your sample iteration loop would look like this:

    // make sure to set those up with PCM data of course (in this case would be -1..1 perhaps,
    // although vertically it doesn't matter it could be -32768.0 to 32767.0. Horizontally you have
    // 44kHz as specified as a template parameter 44100. In oversample mode, you don't need to 
    // multiply frequency in the set_freq() by oversample factor, only the sampling rate.
    
    double8 in[1024], out[1024]; 

    for (int k=0; k<1024; k++) 
    {
        // set outLR to 0 each iteration because it contains sum of the total of bands samples
         
         double8 outLR = 0.0;

        // C++ is nifty isn't it? if you're using simd_doubleX vectors you can initialize
        // them with a constant like this and it will assign that constant to each of the double's
        // within that vector of 8 in this case
        
        cross.process( in[k] );

        for (int i=0; i < bands; i++)
        {
            double8 LR = cross.get_output( i + 1 );
    
            // .. do some  fancy dsp processing here to LR
        
            outLR += LR;     
        }
    
        // write into output buffer
        
        out[k] = outLR;
    }

that's it!

P.S. Never code just for money only. Code because you enjoy it!  
Money will (should) come after you find doing what you love... and
lets not forget - watch out for the evil gnomes, they are out there to get you!
(especially if you're born on the territory of Russia or the former USSR)

You probably already know who THEY are, after the last round of pandemics ;-)
