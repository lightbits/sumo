#ifndef _noise_h_
#define _noise_h_

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

// http://en.wikipedia.org/wiki/Xorshift
// Returns a random number with a period of 2^128 - 1
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

// Returns a single precision floating-point value uniformly over the interval [0.0, 1.0]
float frand()
{
    return xor128() / float(4294967295.0f);
}

#endif
