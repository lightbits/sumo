/* so_noise - v0.1

Changelog
====================================================================
26. september 2015
    Converted to so code format for code reuse purposes.

How to compile
====================================================================
This file contains both the header file and the implementation file.
To create the implementation in one source file include this header:

    #define SO_NOISE_IMPLEMENTATION
    #include "so_noise.h"
*/

#ifndef SO_NOISE_HEADER_INCLUDE
#define SO_NOISE_HEADER_INCLUDE

// return: A 1D hash value in [0.0f, 1.0f]
float        noise1f(int x);

// return: A 2D hash value in [0.0f, 1.0f]
float        noise2f(int x, int y);

// return: A random number with a period of (2^128) - 1
// more:   http://en.wikipedia.org/wiki/Xorshift
unsigned int xor128();

// return: A uniformly distributed value in [0.0f, 1.0f]
float        frand();

#endif // SO_FBO_HEADER_INCLUDE
#ifdef SO_NOISE_IMPLEMENTATION

float noise1f(int x)
{
    x = (x<<13) ^ x;
    return ( 1.0 - ( (x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float noise2f(int x, int y)
{
    int n = x + y * 57;
    n = (n<<13) ^ n;
    return ( 1.0 - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

// https://www.shadertoy.com/view/MsV3z3
// float noise_weyl_hash(int x, int y)
// {
//     x = 0x3504f333*x*x+y;
//     y = 0xf1bbcdcb*y*y+c.x;
//     int r = x*y;
//     return float(r)*(2.0f/8589934592.0f)+0.5f;
// }

unsigned int xor128()
{
    static unsigned int x = 123456789;
    static unsigned int y = 362436069;
    static unsigned int z = 521288629;
    static unsigned int w = 88675123;
    unsigned int t;

    t = x ^ (x << 11);
    x = y; y = z; z = w;
    return w = w ^ (w >> 19) ^ (t ^ (t >>8));
}

float frand()
{
    return xor128() / float(4294967295.0f);
}

#endif // SO_NOISE_IMPLEMENTATION
