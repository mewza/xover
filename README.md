As the description says, the XOVER is a dsp code that implements 
very accurate and fast multiband (4 bands) crossover based on 
CLANG's native intrinsict vector construct of double precision float
of arbitrary length (double8, double4, double2, etc)

This is a much more sophisticated method than utilizing nested LP/HP filters.
So, how to use it:

declare:
    
    static const int bands = 3;
    xoverT<double8,44100> cross;

constructor:

    cross.Init( bands );
 
in the further init or reset:

    // setup the bands 0-85, 85-700, 700-3000, 3000-44khz

    cross.set_freq( 0, 85 );
    cross.set_freq( 1, 700 );
    cross.set_freq( 2, 3000 );

make sure to call this after Init() or bad things will happen (or not...)

    cross.reset();

your sample iteration loop would look like this:

< BIG SAMPLE LOOP BEGIN >

    // make sure to set those up of course
    double8 *in, *out; 

    // C++ is nifty isn't it?
    double8 outLR = 0.0_v;

    // read from input buffer and split into 4 samples accessible via get_output()

    cross.process_T( *in++ );

    for (int i=0; i <= bands; i++)
    {
        double8 LR = cross.get_output(i);

        // .. do some  fancy dsp processing here to LR ...
    
        outLR += LR;     
    }

    // write into output buffer

    *out++ = outLR;

< BIG SAMPLE LOOP END >

that's it!

~ DemoSense ~

P.S. Never code just for money only. Code becaue you enjoy it!  
Money will (should) come after you find doing what you love... and
most important- watch out for the evil gnomes, they are out there to get you!
You probably already know who THEY are, after the last round of pandemics =)
I suppose if you're still walking the Earth, you're already an evil gnome dodger.

