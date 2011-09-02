#include <math.h>
#include <stdlib.h>

#include "descent.h"
#include "maths.h"
#include "perlin.h"

//static long randSeed = 0;

//------------------------------------------------------------------------------

#define RAND_HALF	((double (RAND_MAX) + 1.0) * 0.5)

inline double CPerlin::Noise (int v)
{
v = (v << 13) ^ v;
return 1.0 - ((v * (v * v * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0;    
}

//------------------------------------------------------------------------------

double CPerlin::LinearInterpolate (double a, double b, double x)
{
return a * (1.0 - x) + b * x;
}

//------------------------------------------------------------------------------

double CPerlin::CosineInterpolate (double a, double b, double x)
{
double f = (1.0 - cos (x * Pi)) * 0.5;
return  a * (1.0 - f) + b * f;
}

//------------------------------------------------------------------------------

double CPerlin::CubicInterpolate (double v0, double v1, double v2, double v3, double x)
{
#if DBG
double p = (v3 - v2) - (v0 - v1);
double q = (v0 - v1) - p;
double r = v2 - v0;
double x2 = x * x;
return v1 + r * x + q * x2 + p * x2 * x;
#else
double p = (v3 - v2) - (v0 - v1);
double x2 = x * x;
return v1 + (v2 - v0) * x + (v0 - v1 - p) * x2 + p * x2 * x;
#endif
}

//------------------------------------------------------------------------------

double CPerlin::SmoothedNoise (int v)
{
return Noise (v) / 2  +  Noise (v-1) / 4  +  Noise (v+1) / 4;
}

//------------------------------------------------------------------------------

double CPerlin::InterpolatedNoise (double v, int octave)
{
v *= double (1 << octave);
int i = int (v);
double v1 = SmoothedNoise (i);
double v2 = SmoothedNoise (i + 1);
#if 1
return CosineInterpolate (v1, v2, v - i);
#else
double v0 = SmoothedNoise (i - 1);
double v3 = SmoothedNoise (i + 2);
return CubicInterpolate (v0, v1, v2, v3, v - int (v));
#endif
}

//------------------------------------------------------------------------------

double CPerlin::ComputeNoise (double v)
{
double total = 0, amplitude = m_amplitude;
#if 0 //DBG
octaves = 1;
#endif
v += m_nOffset;
for (int i = 0; i < m_octaves; i++) {
#if DBG
	double n = InterpolatedNoise (v, i);
	total += n * amplitude;
#else
	total += InterpolatedNoise (v, i) * amplitude;
#endif
	amplitude *= m_persistence;
	}
return total;
}

//------------------------------------------------------------------------------

double CPerlin::Noise (int x, int y)
{
int n = (int) x + (int) y * 57;
n = (n << 13) ^ n;
int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
return 1.0 - (double) nn / 1073741824.0;
}

//------------------------------------------------------------------------------

double CPerlin::SmoothedNoise (int x, int y)
{
double corners = (Noise (x-1, y-1) + Noise (x+1, y-1) + Noise (x-1, y+1) + Noise (x+1, y+1)) / 16;
double sides = (Noise (x-1, y) + Noise (x+1, y) + Noise (x, y-1) + Noise (x, y+1)) / 8;
double center = Noise (x, y) / 4;
return corners + sides + center;
}

//------------------------------------------------------------------------------

double CPerlin::InterpolatedNoise (double x, double y, int octave)
{
#if 1
x *= double (1 << octave);
int xInt = int (x);
double xFrac = x - xInt;
y *= double (1 << octave);
int yInt = int (y);
double yFrac = y - yInt;
#else
double xInt, yInt,
		 xFrac = modf (x, &xInt),
		 yFrac = modf (x, &yInt);
#endif
double v1 = SmoothedNoise (xInt, yInt);
double v2 = SmoothedNoise (xInt+1, yInt);
double v3 = SmoothedNoise (xInt, yInt+1);
double v4 = SmoothedNoise (xInt+1, yInt+1);
double i1 = CosineInterpolate (v1, v2, xFrac);
double i2 = CosineInterpolate (v3, v4, xFrac);
return CosineInterpolate (i1, i2, yFrac);
}

//------------------------------------------------------------------------------

double CPerlin::ComputeNoise (double x, double y)
{
double total = 0, amplitude = m_amplitude;
for (int i = 0; i < m_octaves; i++) {
	total += InterpolatedNoise (x, y, i) * amplitude;
	amplitude *= m_persistence;
	}
return total;
}

//------------------------------------------------------------------------------

bool CPerlin::Setup (double amplitude, double persistence, int octaves, int nDimensions, int nOffset)
{
m_amplitude = (amplitude > 0.0) ? amplitude : 1.0;
m_persistence = (persistence > 0.0) ? persistence : 2.0 / 3.0;
m_octaves = (octaves > 0) ? octaves : 6;
m_nOffset = (nOffset < 0) ? (rand () * rand ()) & 0xFFFF : nOffset;
return true;
}

//------------------------------------------------------------------------------


