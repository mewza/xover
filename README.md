As the description says, the xoverT is a simple dsp code that implements 
very accurate and fast multiband (up to 4 bands) crossover utilizing templates
and therefore, CLANG's native dynamic intrinsict vector construct.

This is a much more sophisticated method than utilizing nested LP/HP filters.
So, how to use it:

declare:

    // for iOS and OS X include simd for simd_double8, ..4, ..2 vectors
    // already defined and many math functions provided via simd::sinf(some_simd_vector)
    // for example. If you're going to use simd_double8 vectors, make sure to change 
    // xover.h defines to reflect that... left as excercise for the user :)
    
    #include <simd/simd.h>
    
    // 4 = number of bands (got moved from Init into template parameter
    xoverT<double8,4,44100> cross;

in the further init or reset:

    // setup the bands 1-85, 85-700, 700-3k, 3k-22kHz
    
    // NOTE: indexing into bands has been changed from 0..N-1 
    // to 1..N corresponding to actual bands not split points 
    // which are N-1 of them (3 in this case for 4-band crossover)
    
    cross.set_freq( 1, 85 );
    cross.set_freq( 2, 700 );
    cross.set_freq( 3, 3000 );

    // NOTE: you don't need to call cross.reset() it is called after each
    // set_freq() call. But you can anyway, it won't hurt it
    
    cross.reset();

your sample iteration loop would look like this:

< BIG SAMPLE LOOP BEGIN >

    // make sure to set those up of course
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
    
< BIG SAMPLE LOOP END >

that's it!

P.S. Never code just for money only. Code because you enjoy it!  
Money will (should) come after you find doing what you love... and
lets not forget - watch out for the evil gnomes, they are out there to get you!
(especially if you're born on the territory of Russia or the former USSR)

You probably already know who THEY are, after the last round of pandemics ;-)
