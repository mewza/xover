As the description says, the XOVER is a dsp code that implements 
very accurate multiband (4 bands) crossover that utilizes fast 
CLANG's native vector arithmetics using intrinsict vector construct 
of double precision float of arbitrary length vector (double8, double4, double2, etc)
(definitions included)

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

// make sure to call this or bad things will happen (or not...)
cross.reset(); 

your sample iteration loop would look like this:

< BIG SAMPLE LOOP BEGIN >

double8 *in, *out; // make sure to set those up
double8 outLR = 0.0_v; // nifty isn't it? :-)

cross.process_T( *in++ );
for (int i=0; i <= bands; i++)
{
     double8 LR = cross.get_output(i);
     // .. do some additional fancy dsp processing here to LR ...
     outLR += LR;
}
*out++ = outLR;

< BIG SAMPLE LOOP END >

that's it!  Enjoy the FUN!!!

~ DemoSense ~
