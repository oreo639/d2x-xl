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

//------------------------------------------------------------------------

inline depthType CMineView::Z (CTexture& tex, APOINT* a, int i)
{
	int x = i % tex.m_info.width;
	int y = i / tex.m_info.width;
	double scale = (double) y / (double) tex.m_info.height;
	double z1 = (double) a [0].z + (double) (a [1].z - a [0].z) * scale;
	double z2 = (double) a [2].z + (double) (a [3].z - a [2].z) * scale;

return (depthType) (z1 + (z2 - z1) * (double) x / (double) tex.m_info.width);
}

//------------------------------------------------------------------------------

inline void CMineView::Blend (CBGR& dest, CBGRA& src, depthType& depth, depthType z, short brightness)
{
if (brightness == 0)
	return;
if (src.a == 0)
	return;

if (depth < z)
	return;
if (!m_bIgnoreDepth)
	depth = z;

if (brightness == 32767) {
	if (src.a == 255) {
		dest = src;
		return;
		}

	int b = 255 - src.a;
	dest.r = (byte) (((int) dest.r * b + (int) src.r * src.a) / 255);
	dest.g = (byte) (((int) dest.g * b + (int) src.g * src.a) / 255);
	dest.b = (byte) (((int) dest.b * b + (int) src.b * src.a) / 255);
	return;
	}

if (src.a == 255) {
	dest.r = (byte) ((int) src.r * brightness / 32767);
	dest.g = (byte) ((int) src.g * brightness / 32767);
	dest.b = (byte) ((int) src.b * brightness / 32767);
	return;
	}

int b = 255 - src.a;
int a = src.a * brightness;
dest.r = (byte) (((int) dest.r * b + (int) src.r * a / 32767) / 255);
dest.g = (byte) (((int) dest.g * b + (int) src.g * a / 32767) / 255);
dest.b = (byte) (((int) dest.b * b + (int) src.b * a / 32767) / 255);
}

//------------------------------------------------------------------------
// RenderFace()
//------------------------------------------------------------------------

void CMineView::RenderFace (CSegment* segP, short nSide, CTexture& tex, ushort width, ushort height, ushort rowOffset)
{
	int h, i, j, k;
	APOINT a [4];
	APOINT minPt, maxPt;
	CDoubleMatrix A, IA, B, UV;
	//double A [3][3], IA [3][3], B [3][3], UV [3][3]; // transformation matrices
	CUVL *uvls;
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
	a [i] = m_viewPoints [nVertex];
	}
// determin min/max points
minPt.x = minPt.y = 32767; // some number > any screen resolution
maxPt.x = maxPt.y = -32767;
for (i = 0; i < 4; i++) {
	long h = a [i].x;
	minPt.x = min (minPt.x, h);
	maxPt.x = max (maxPt.x, h);
	h = a [i].y;
	minPt.y = min (minPt.y, h);
	maxPt.y = max (maxPt.y, h);
	}

// clip min/max with screen min/max
minPt.x = max (minPt.x, 0);
maxPt.x = min (maxPt.x, width - 1);
minPt.y = max (minPt.y, 0);
maxPt.y = min (maxPt.y, height - 1);


// map unit square into texture coordinate
A.Square2Quad (a);

// calculate adjoint matrix (same as inverse)
IA = A.Adjoint ();

// store uv coordinates into b []
// fill in texture
APOINT b [4];  // Descent's (u,v) coordinates for textures
uvls = segP->m_sides [nSide].m_info.uvls;
for (i = 0; i < 4; i++) {
	b [i].x = uvls [i].u;
	b [i].y = uvls [i].v;
	}
	
	// define texture light
for (i = 0; i < 4; i++) {
	light [i] = uvls [i].l;
	// clip light
	if (light [i] & 0x8000){
		light [i] = 0x7fff;
		}
	}
	// reduce texture light if current side is on a delta light
	// first make sure we have allocated space for delta lights
if (bEnableDeltaShading) {
	CLightDeltaIndex *lightDeltaIndices;
	int dlIdxCount = lightManager.DeltaIndexCount ();
	CLightDeltaValue* lightDeltaValues;
	if (!lightManager.LightStatus (face.m_nSegment, face.m_nSide)->bIsOn &&
		 (lightDeltaIndices = lightManager.LightDeltaIndex (0)) &&
		 (lightDeltaValues = lightManager.LightDeltaValue (0))) {
		// search delta light index to see if current side has a light
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
//square2quad_matrix(UV,b);
UV.Square2Quad (b);
//scale_matrix (UV, 1.0 / 2048.0);
UV.Scale (1.0 / 2048.0);
//multiply_matrix (B, IA, UV);
B = IA * UV;

//#pragma omp parallel
{
//#	pragma omp for private (scanLight, deltaLight)
for (int y = minPt.y; y < maxPt.y; y++) {
	// Determine min and max x for this y.
	// Check each of the four lines of the quadrilaterial
	// to figure out the min and max x
	int x0 = maxPt.x; // start out w/ min point all the way to the right
	int x1 = minPt.x; // and max point to the left
	for (i = 0; i < 4; i++) {
		// if line intersects this y then update x0 & x1
		int j = (i + 1) % 4; // j = other point of line
		long yi = a [i].y;
		long yj = a [j].y;
		if ((y >= yi && y <= yj) || (y >= yj && y <= yi)) {
			double w = yi - yj;
			if (w != 0.0) { // avoid divide by zero
				double di = (double) (y - yi);
				double dj = (double) (y - yj);
				int x = (int) (((double) a [i].x * dj - (double) a [j].x * di) / w);
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
				
				i = (uint) (height - y - 1) * (uint) rowOffset + x0;
				CBGR* screenBufP = m_renderBuffer + i;
				depthType* depthBufP = m_depthBuffer + i;
				
				int k = (x1 - x0);
				if (k > 0) {
					CBGR* pixelP = screenBufP;
					depthType* zBufP = depthBufP;
					if (bEnableShading) {
						do {
							u += du;
							u %= m;
							v += dv;
							v %= m;
							// a fade value denotes the brightness of a color
							// scanLight / 4 is the index in the fadeTables which consists of 34 tables with 256 entries each
							// so for each color there are 34 fade (brightness) values ranges from 1/34 to 34/34
							// actually the fade tables contain palette indices denoting the dimmed color corresponding to the 
							// fade value
							// We don't need this anymore here since we're rendering RGB and can compute the brightness directly
							// from scanLight, the maximum of which is 8191
							// byte fade = fadeTables [j + ((scanLight / 4) & 0x1f00)];
							i = (u / 1024) + ((v / vd) & vm);
#if 1
							Blend (*pixelP, tex.m_info.bmData [i], *zBufP, Z (tex, a, i), scanLight);
#else
							if (tex.m_info.bmData [i].a > 0) {
								CBGR c = tex.m_info.bmData [i];
								pixelP->r = (byte) ((int) (c.r) * fade / 8191);
								pixelP->g = (byte) ((int) (c.g) * fade / 8191);
								pixelP->b = (byte) ((int) (c.b) * fade / 8191);
								}
#endif
							pixelP++;
							zBufP++;
							scanLight += deltaLight;
							} while (--k);
						} 
					else {
						do {
							u += du;
							u %= m;
							v += dv;
							v %= m;
							i = (u / 1024) + ((v / vd) & vm);
#if 1
								Blend (*pixelP, tex.m_info.bmData [i], *zBufP, Z (tex, a, i));
#else
							if (tex.m_info.bmData [i].a > 0)
								*pixelP = tex.m_info.bmData [i];
#endif
							pixelP++;
							zBufP++;
							} while (--k);
						}
					}
				}
			} // end of 32 byte loop
		}
	}
} // omp parallel
}
