#include <math.h>
#include <stdlib.h>

#include "descent.h"
#include "maths.h"
#include "perlin.h"

//static long randSeed = 0;

//------------------------------------------------------------------------------

#define RAND_HALF	((double (RAND_MAX) + 1) / 2)

#if CUSTOM_RAND

inline double CPerlin::Random (int x)
{
x = (x << 13) ^ x;
return 1.0 - ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0;    
}

#else

inline double CPerlin::Random (void)
{
return (double (rand ()) - RAND_HALF) / RAND_HALF;
}

#endif

//------------------------------------------------------------------------------

double CPerlin::Noise (double x)
{
#if CUSTOM_RAND > 1
return Random (x);
#else
uint i = (uint) x + 2;
#	if 0
if (!m_valid [i]) {
	m_valid [i] = 1;
	m_random [i] = Random ();
	}
#	endif
return m_random [i];
#endif
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

double CPerlin::SmoothedNoise (int x)
{
return Noise (x) / 2  +  Noise (x-1) / 4  +  Noise (x+1) / 4;
}

//------------------------------------------------------------------------------

double CPerlin::InterpolatedNoise (double x, int octave)
{
x *= 1 << octave;
int xi = int (x);
//int xi = int (x * (m_nNodes - 1) + m_nNodes * octave);
double v1 = SmoothedNoise (xi);
double v2 = SmoothedNoise (xi + 1);
#if 1
return CosineInterpolate (v1, v2, x - xi);
#else
double v0 = SmoothedNoise (xInt - 1);
double v3 = SmoothedNoise (xInt + 2);
return CubicInterpolate (v0, v1, v2, v3, x - xInt);
#endif
}

//------------------------------------------------------------------------------

double CPerlin::ComputeNoise (double x, double persistence, long octaves)
{
double total = 0, frequency = 1.0, amplitude = 1.0;
for (int i = 0; i < octaves; i++) {
#if DBG
	double n = InterpolatedNoise (x, i);
	total += n * amplitude;
#else
	total += InterpolatedNoise (x, i) * amplitude;
#endif
	amplitude *= persistence;
	}
return total;
}

//------------------------------------------------------------------------------

double CPerlin::Noise (double x, double y)
{
return (Noise (x) + Noise (y)) / 2.0;
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
x *= 1 << octave;
int xInt = int (x);
double xFrac = x - xInt;
y *= 1 << octave;
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

double CPerlin::ComputeNoise (double x, double y, double persistence, long octaves)
{
double total = 0, frequency = 1.0, amplitude = 1.0;
for (int i = 0; i < octaves; i++) {
	total += InterpolatedNoise (x, y, i) * amplitude;
	amplitude *= persistence;
	}
return total;
}

//------------------------------------------------------------------------------

bool CPerlin::Setup (int nNodes, int nOctaves, int nDimensions)
{
m_nNodes = nNodes;
nNodes = ((nNodes + 4) << nOctaves) * nDimensions;
if (m_random.Buffer () && (int (m_random.Length ()) < nNodes)) {
	m_random.Destroy ();
	m_valid.Destroy ();
	}
if (!(m_random.Buffer () || (m_random.Create (nNodes) && m_valid.Create (nNodes))))
	return false;
#if 1
for (int i = 0; i < nNodes; i++) 
	m_random [i] = Random ();
#else
m_valid.Clear ();
#endif
return true;
}

//------------------------------------------------------------------------------


