// render.cpp

#include "stdafx.h"
#include <math.h>

#include "types.h"
#include "matrix.h"
#include "global.h"
#include "mine.h"
#include "segment.h"
#include "GameObject.h"
#include "dle-xp.h"
#include "dlcdoc.h"
#include "mainfrm.h"
#include "mineview.h"

extern byte sideVertTable [6][4];
int bEnableDeltaShading = 0;

//------------------------------------------------------------------------------

int LineLineIntersection (APOINT& vi, APOINT& v1, APOINT& v2, APOINT& v3, APOINT& v4)
{
	double A1 = v2.y - v1.y;
	double B1 = v1.x - v2.x;
	double A2 = v4.y - v3.y;
	double B2 = v3.x - v4.x;

double det = A1 * B2 - A2 * B1;
if (det == 0)
	return 0;

double C1 = A1 * v1.x + B1 * v1.y;
double C2 = A2 * v3.x + B2 * v3.y;

vi.x = (long) ((B2 * C1 - B1 * C2) / det);
vi.y = (long) ((A1 * C2 - A2 * C1) / det);
return 1;
}

#if 0

//------------------------------------------------------------------------------
// Compute intersection of perpendicular to p1,p2 through p3 with p1,p2.

long PointLineIntersection (long& x0, long& y0, long x1, long y1, long x2, long y2, long x3, long y3)
{
long dx = x2 - x1;
long dy = y2 - y1;
double u = (x3 - x2) * dx + (y3 - y2) * dy;
double d = _hypot ((double) dx, (double) dy);
u /= d * d;
x0 = (long) (x1 + u * dx + 0.5);
y0 = (long) (y1 + u * dy + 0.5);
return (long) (_hypot ((double) (x3 - x0), (double) (y3 - y0)) + 0.5);
}

//------------------------------------------------------------------------------

inline double Dot (APOINT& v0, APOINT& v1)
{
return (double) v0.x * (double) v1.x + (double) v0.y * (double) v1.y;
}

//------------------------------------------------------------------------------

static bool PointInTriangle (APOINT& A, APOINT& B, APOINT& C, APOINT& P)
{
APOINT v0 = {C.x - A.x, C.y - A.y};
APOINT v1 = {B.x - A.x, B.y - A.y};
APOINT v2 = {P.x - A.x, P.y - A.y};

// Compute dot products
double dot00 = Dot (v0, v0);
double dot01 = Dot (v0, v1);
double dot02 = Dot (v0, v2);
double dot11 = Dot (v1, v1);
double dot12 = Dot (v1, v2);

// Compute barycentric coordinates
double invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
double v = (dot00 * dot12 - dot01 * dot02) * invDenom;
// Check if point is in triangle
return (u > 0.0) && (v > 0.0) && (u + v < 1.0);
}

//------------------------------------------------------------------------------
// v1, v2, v3: triangle corners (z known)
// v0: reference point (z needed)

inline depthType InterpolateZ (APOINT& v0, APOINT& v1, APOINT& v2, APOINT& v3)
{
	APOINT vi;

int i = LineLineIntersection (vi, v1, v2, v3, v0);
depthType zi = (depthType) (v1.z + (v2.z - v1.z) * _hypot (vi.x - v1.x, vi.y - v1.y) / _hypot (v2.x - v1.x, v2.y - v1.y));
return (depthType) (v3.z + (zi - v3.z) * _hypot (v0.x - v3.x, v0.y - v3.y) / _hypot (vi.x - v3.x, vi.y - v3.y));
}

//------------------------------------------------------------------------------

inline depthType CMineView::Z (CTexture& tex, APOINT* a, int x, int y)
{
	APOINT v0 = {x, y};

return InterpolateZ (v0, a [0], a [2], a [PointInTriangle (a [0], a [1], a [2], v0) ? 1 : 3]);
}

#endif

//------------------------------------------------------------------------------

inline double CMineView::ZRange (int x0, int x1, int y, double& z)
{
	APOINT p0 = {x0, y}, p1 = {x1, y}, vi [4];
	int h = -1, j = 0;
	double zi [4];

#ifdef _DEBUG
if (y == 517)
	y = y;
#endif
for (int i = 0; i < 4; i++) {
	APOINT v0 = m_screenCoord [i];
	APOINT v1 = m_screenCoord [(i + 1) % 4];
#if 0
	if (v0.x < v1.x) {
		if ((x1 < v0.x) || (x0 > v1.x))
			continue;
		}
	else {
		if ((x1 < v1.x) || (x0 > v0.x))
			continue;
		}
	if (v0.y < v1.y) {
		if ((y < v0.y) || (y > v1.y))
			continue;
		}
	else {
		if ((y < v1.y) || (y > v0.y))
			continue;
		}
#endif
	APOINT v;
	if (!LineLineIntersection (v, v0, v1, p0, p1))
		continue;
	if ((j > 0) && (v.x == vi [j - 1].x) && (v.y == vi [j - 1].y))
		continue;
#if 1
	if (v0.x < v1.x) {
		if ((v.x < v0.x) || (v.x > v1.x))
			continue;
		}
	else {
		if ((v.x < v1.x) || (v.x > v0.x))
			continue;
		}
	if (v0.y < v1.y) {
		if ((v.y < v0.y) || (v.y > v1.y))
			continue;
		}
	else {
		if ((v.y < v1.y) || (v.y > v0.y))
			continue;
		}
#else
	if (Dot (v0, v1) > 0.0)
		continue;
#endif
	v0.x -= v.x;
	v0.y -= v.y;
	v1.x -= v.x;
	v1.y -= v.y;
	double l1 = _hypot (v0.x, v0.y);
	double l2 = _hypot (v1.x, v1.y);
	vi [j] = v;
	zi [j++] = v0.z + (v1.z - v0.z) * l1 / (l1 + l2);
	}
if (j > 3)
	j = 2;
if (j == 0) {
	z = MAX_DEPTH;
	return 0.0;
	}
if (j == 1) {
	z = zi [0];
	return 0.0;
	}
if (vi [0].x <= vi [1].x) {
	z = zi [0];
	return (zi [1] - z) / (double) (x1 - x0);
	}
else {
	z = zi [1];
	return (zi [0] - z) / (double) (x1 - x0);
	}	
}

//------------------------------------------------------------------------------

inline double CMineView::ZRange (void)
{
#if 0
	APOINT p0 = {m_x0, m_y}, p1 = {m_x1, m_y}, vi [4];
	int h = -1, j = 0;
	double zi [4];

for (int i = 0; i < 4; i++) {
	APOINT v0 = m_screenCoord [i];
	APOINT v1 = m_screenCoord [(i + 1) % 4];
#if 0
	if (v0.x < v1.x) {
		if ((m_x1 < v0.x) || (m_x0 > v1.x))
			continue;
		}
	else {
		if ((m_x1 < v1.x) || (m_x0 > v0.x))
			continue;
		}
	if (v0.y < v1.y) {
		if ((m_y < v0.y) || (m_y > v1.y))
			continue;
		}
	else {
		if ((m_y < v1.y) || (m_y > v0.y))
			continue;
		}
#endif
	APOINT v;
	if (!LineLineIntersection (v, v0, v1, p0, p1))
		continue;
	if ((j > 0) && (v.x == vi [j - 1].x) && (v.y == vi [j - 1].y))
		continue;
#if 1
	if (v0.x < v1.x) {
		if ((v.x < v0.x) || (v.x > v1.x))
			continue;
		}
	else {
		if ((v.x < v1.x) || (v.x > v0.x))
			continue;
		}
	if (v0.y < v1.y) {
		if ((v.y < v0.y) || (v.y > v1.y))
			continue;
		}
	else {
		if ((v.y < v1.y) || (v.y > v0.y))
			continue;
		}
#else
	if (Dot (v0, v1) > 0.0)
		continue;
#endif
	v0.x -= v.x;
	v0.y -= v.y;
	v1.x -= v.x;
	v1.y -= v.y;
	double l1 = _hypot (v0.x, v0.y);
	double l2 = _hypot (v1.x, v1.y);
	vi [j] = v;
	zi [j++] = v0.z + (v1.z - v0.z) * l1 / (l1 + l2);
	}
if (j == 0) {
	m_z0 = m_z1 = MAX_DEPTH;
	return 0.0;
	}
if (j == 1) {
	m_z0 = m_z1 = zi [0];
	return 0.0;
	}
if (vi [0].x <= vi [1].x) {
	m_z0 = zi [0];
	m_z1 = zi [1];
	}
else {
	m_z0 = zi [0];
	m_z1 = zi [1];
	}	
return (m_z1 - m_z0) / (double) (m_x1 - m_x0);
#else
return 0.0;
#endif
}

//------------------------------------------------------------------------------

inline bool CMineView::Blend (CBGR& dest, CBGRA& src, depthType& depth, depthType z, short brightness)
{
if (brightness == 0)
	return false;
if (src.a == 0)
	return false;

if (depth <= z)
	return false;
#ifdef _DEBUG
if (depth != MAX_DEPTH)
	z = z;
#endif
if (!m_bIgnoreDepth)
	depth = z;

if (brightness == 32767) {
	if (src.a == 255) {
		dest = src;
		return true;
		}

	int b = 255 - src.a;
	dest.r = (byte) (((int) dest.r * b + (int) src.r * src.a) / 255);
	dest.g = (byte) (((int) dest.g * b + (int) src.g * src.a) / 255);
	dest.b = (byte) (((int) dest.b * b + (int) src.b * src.a) / 255);
	return true;
	}

if (src.a == 255) {
	dest.r = (byte) ((int) src.r * brightness / 32767);
	dest.g = (byte) ((int) src.g * brightness / 32767);
	dest.b = (byte) ((int) src.b * brightness / 32767);
	return true;
	}

int b = 255 - src.a;
int a = src.a * brightness;
dest.r = (byte) (((int) dest.r * b + (int) src.r * a / 32767) / 255);
dest.g = (byte) (((int) dest.g * b + (int) src.g * a / 32767) / 255);
dest.b = (byte) (((int) dest.b * b + (int) src.b * a / 32767) / 255);
return true;
}

//------------------------------------------------------------------------
// RenderFace()
//------------------------------------------------------------------------

void CMineView::RenderFace (CSegment* segP, short nSide, CTexture& tex, ushort rowOffset)
{
	int h, i, j, k;
	APOINT minPt, maxPt;
	CDoubleMatrix A, IA, B, UV;
	//double A [3][3], IA [3][3], B [3][3], UV [3][3]; // transformation matrices
	CUVL* uvls;
	bool bD2XLights = (DLE.LevelVersion () >= 15) && (theMine->Info ().fileInfo.version >= 34);
	// TEMPORARY
	CSideKey face (short (segP - segmentManager.Segment (0)), nSide);
	short flickLight = lightManager.VariableLight (face);
	short deltaLight, scanLight;
	short light [4];
	ushort bmWidth2;
	byte* fadeTables = paletteManager.FadeTable ();
	bool bEnableShading = (m_viewMineFlags & eViewMineShading) != 0;
	double scale = (double) max (tex.m_info.width, tex.m_info.height);

tex.m_info.height = tex.m_info.width;
bmWidth2 = tex.m_info.width / 2;

// define 4 corners of texture to be displayed on the screen
for (i = 0; i < 4; i++) {
	ushort nVertex = segP->m_info.verts [sideVertTable [nSide][i]];
	m_screenCoord [i] = m_viewPoints [nVertex];
	}
// determin min/max points
minPt.x = minPt.y = 32767; // some number > any screen resolution
maxPt.x = maxPt.y = -32767;
for (i = 0; i < 4; i++) {
	long h = m_screenCoord [i].x;
	minPt.x = min (minPt.x, h);
	maxPt.x = max (maxPt.x, h);
	h = m_screenCoord [i].y;
	minPt.y = min (minPt.y, h);
	maxPt.y = max (maxPt.y, h);
	}

// clip min/max with screen min/max
minPt.x = max (minPt.x, 0);
maxPt.x = min (maxPt.x, m_viewWidth - 1);
minPt.y = max (minPt.y, 0);
maxPt.y = min (maxPt.y, m_viewHeight - 1);


// map unit square into texture coordinate
A.Square2Quad (m_screenCoord);

// calculate adjoint matrix (same as inverse)
IA = A.Adjoint ();

// store uv coordinates into m_texCoord []
// fill in texture
APOINT m_texCoord [4];  // Descent's (u,v) coordinates for textures
uvls = segP->m_sides [nSide].m_info.uvls;
for (i = 0; i < 4; i++) {
	m_texCoord [i].x = uvls [i].u;
	m_texCoord [i].y = uvls [i].v;
	}
	
	// define texture light
for (i = 0; i < 4; i++) {
	light [i] = uvls [i].l;
	// clip light
	if (light [i] & 0x8000){
		light [i] = 0x7fff;
		}
	}
	// reduce texture light if current side is on m_screenCoord delta light
	// first make sure we have allocated space for delta lights
if (bEnableDeltaShading) {
	CLightDeltaIndex *lightDeltaIndices;
	int dlIdxCount = lightManager.DeltaIndexCount ();
	CLightDeltaValue* lightDeltaValues;
	if (!lightManager.LightStatus (face.m_nSegment, face.m_nSide)->bIsOn &&
		 (lightDeltaIndices = lightManager.LightDeltaIndex (0)) &&
		 (lightDeltaValues = lightManager.LightDeltaValue (0))) {
		// search delta light index to see if current side has m_screenCoord light
		CLightDeltaIndex	*dli = lightDeltaIndices;
		for (i = 0; i < dlIdxCount; i++, dli++) {
//				if (dli->m_info.nSegment == current->segP) {
			// loop on each delta light till the segP/side is found
				CLightDeltaValue *dlP = lightManager.LightDeltaValue (dli->m_info.index);
				h = dli->m_info.count;
				for (j = 0; j < h; j++, dlP++) {
					if (*dlP == face) {
						for (k = 0; k < 4; k++) {
							short dlight = dlP->m_info.vertLight [k];
							if (dlight >= 0x20)
								dlight = 0x7fff;
							else
								dlight <<= 10;
							if (light [k] > dlight)
								light [k] -= dlight;
							else
								light [k] = 0;
							}
						}
//					}
				}
			}
		}
	}
	// map unit square into uv coordinates
	// uv of 0x800 = 64 pixels in texture
	// therefore uv of 32 = 1 pixel
//square2quad_matrix(UV,m_texCoord);
UV.Square2Quad (m_texCoord);
//scale_matrix (UV, 1.0 / 2048.0);
UV.Scale (1.0 / 2048.0);
//multiply_matrix (B, IA, UV);
B = IA * UV;

#ifndef _DEBUG
#pragma omp parallel
#endif
{
#ifndef _DEBUG
#	pragma omp for private (scanLight, deltaLight)
#endif
for (int y = minPt.y; y < maxPt.y; y++) {
	int i;
	// Determine min and max x for this y.
	// Check each of the four lines of the quadrilaterial
	// to figure out the min and max x
	int x0 = maxPt.x; // start out w/ min point all the way to the right
	int x1 = minPt.x; // and max point to the left
	for (i = 0; i < 4; i++) {
		// if line intersects this y then update x0 & x1
		int j = (i + 1) % 4; // j = other point of line
		long yi = m_screenCoord [i].y;
		long yj = m_screenCoord [j].y;
		if ((y >= yi && y <= yj) || (y >= yj && y <= yi)) {
			double w = yi - yj;
			if (w != 0.0) { // avoid divide by zero
				double di = (double) (y - yi);
				double dj = (double) (y - yj);
				int x = (int) (((double) m_screenCoord [i].x * dj - (double) m_screenCoord [j].x * di) / w);
				if (x < x0) {
					scanLight = (int) (((double) light [i] * dj - (double) light [j] * di) / w);
					x0 = x;
					}
				if (x > x1) {
					deltaLight = (int) (((double) light [i] * dj - (double) light [j] * di) / w);
					x1 = x;
					}
				}
			}
		} // end for
	
	// clip
	x0 = max (x0, minPt.x);
	x1 = min (x1, maxPt.x);
	
	// Instead of finding every point using the matrix transformation,
	// just define the end points and delta values then simply
	// add the delta values to u and v
	if (abs (x0 - x1) > 0) {
		double u0, u1, v0, v1, w0, w1, h, x0d, x1d;
		uint u, v, du, dv, m, vd, vm, dx;
		deltaLight = (deltaLight - scanLight) / (x1 - x0);
		
		// loop for every 32 bytes
		for (int xEnd = x1; x0 < xEnd ; x0 += bmWidth2) {
			if (xEnd - x0 > bmWidth2)
				x1 = bmWidth2 + x0;
			else
				x1 = xEnd;

			h = B.uVec.v.z * (double) y + B.fVec.v.z;
			x0d = (double) x0;
			x1d = (double) x1;
			w0 = (B.rVec.v.z * x0d + h) / scale; // scale factor (64 pixels = 1.0 unit)
			w1 = (B.rVec.v.z * x1d + h) / scale;
			if ((fabs (w0) > 0.0001) && (fabs (w1) > 0.0001)) {
				h = B.uVec.v.x * (double) y + B.fVec.v.x;
				u0 = (B.rVec.v.x * x0d + h) / w0;
				u1 = (B.rVec.v.x * x1d + h) / w1;
				h = B.uVec.v.y * (double) y + B.fVec.v.y;
				v0 = (B.rVec.v.y * x0d + h) / w0;
				v1 = (B.rVec.v.y * x1d + h) / w1;
				
				// use 22.10 integer math
				// the 22 allows for large texture bitmap sizes
				// the 10 gives more than enough accuracy for the delta values

				m = min (tex.m_info.width, tex.m_info.height);
				if (!m)
					m = 64;
				m *= 1024;
				dx = x1 - x0;
				if (!dx)
					dx = 1;
				du = ((uint) (((u1 - u0) * 1024.0) / dx) % m);
				// v0 & v1 swapped since tex.m_info.bmData is flipped
				dv = ((uint) (((v0 - v1) * 1024.0) / dx) % m);
				u = ((uint) (u0 * 1024.0)) % m;
				v = ((uint) (-v0 * 1024.0)) % m;
				vd = 1024 / tex.m_info.height;
				vm = tex.m_info.width * (tex.m_info.height - 1);
				
				i = (uint) y/*(m_viewHeight - y - 1)*/ * (uint) rowOffset + x0;
				double z, dz = ZRange (x0, x1, y, z);
				
				if (bEnableShading) {
					for (int x = x0; x < x1; x++, i++) {
						u += du;
						u %= m;
						v += dv;
						v %= m;
						// m_screenCoord fade value denotes the brightness of m_screenCoord color
						// scanLight / 4 is the index in the fadeTables which consists of 34 tables with 256 entries each
						// so for each color there are 34 fade (brightness) values ranges from 1/34 to 34/34
						// actually the fade tables contain palette indices denoting the dimmed color corresponding to the 
						// fade value
						// We don't need this anymore here since we're rendering RGB and can compute the brightness directly
						// from scanLight, the maximum of which is 8191
						// byte fade = fadeTables [j + ((scanLight / 4) & 0x1f00)];
#if 1
						Blend (m_renderBuffer [i], tex.m_info.bmData [(u / 1024) + ((v / vd) & vm)], m_depthBuffer [i], (depthType) z, scanLight);
						z += dz;
#else
						int t = (u / 1024) + ((v / vd) & vm);
						if (tex.m_info.bmData [t].m_screenCoord > 0) {
							CBGR c = tex.m_info.bmData [t];
							CBGR* pixel = m_renderBuffer + i;
							pixelP->r = (byte) ((int) (c.r) * fade / 8191);
							pixelP->g = (byte) ((int) (c.g) * fade / 8191);
							pixelP->b = (byte) ((int) (c.b) * fade / 8191);
							pixelP->m_texCoord = (byte) ((int) (c.m_texCoord) * fade / 8191);
							}
#endif
						scanLight += deltaLight;
						} 
					}
				else {
					for (int x = x0; x < x1; x++, i++) {
						u += du;
						u %= m;
						v += dv;
						v %= m;
#if 1
						Blend (m_renderBuffer [i], tex.m_info.bmData [(u / 1024) + ((v / vd) & vm)], m_depthBuffer [i], (depthType) z);
						z += dz;
#else
						int t = (u / 1024) + ((v / vd) & vm);
						if (tex.m_info.bmData [t].m_screenCoord > 0)
							m_renderBuffer [i] = tex.m_info.bmData [t];
#endif
						}
					}
				}
			} // end of 32 byte loop
		}
	}
} // omp parallel
}
