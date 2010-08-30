// render.cpp

#include "stdafx.h"
#include <math.h>

#include "types.h"
#include "matrix.h"
#include "global.h"
#include "mine.h"
#include "segment.h"
#include "object.h"
#include "dle-xp.h"
#include "dlcdoc.h"
#include "mainfrm.h"
#include "mineview.h"

extern byte sideVertTable [6][4];
int bEnableDeltaShading = 0;

//------------------------------------------------------------------------
// TextureMap()
//------------------------------------------------------------------------

void TextureMap (CSegment *segP, short nSide,
					  byte *bmData, ushort bmWidth, ushort bmHeight,
					  byte *light_index,
					  byte *pScrnMem, APOINT* scrn,
					  ushort width, ushort height, ushort rowOffset)
{
	
	int h, i, j, k;
	int x, y;
	LONG yi,  yj;
	POINT a [4];
	POINT minpt, maxpt;
	CDoubleMatrix A, IA, B, UV;
	//double A [3][3], IA [3][3], B [3][3], UV [3][3]; // transformation matrices
	double w;
	byte *ptr;
	CUVL *uvls;
	bool bD2XLights = (theMine->LevelVersion () >= 15) && (theMine->GameInfo ().fileInfo.version >= 34);
	
	// TEMPORARY
	CSideKey face (short (segP - theMine->Segments (0)), nSide);
	short flick_light = theMine->GetFlickeringLight (face.m_nSegment, face.m_nSide);
	short dscan_light,scan_light;
	short light [4];
	ushort bmWidth2;
	bool bEnableShading = (light_index != null);

bmHeight = bmWidth;
bmWidth2 = bmWidth / 2;

// define 4 corners of texture to be displayed on the screen
for (i = 0; i < 4; i++) {
	short nVertex = segP->m_info.verts [sideVertTable [nSide][i]];
	a [i].x = scrn [nVertex].x;
	a [i].y = scrn [nVertex].y;
	}
	
	// determin min/max points
minpt.x = minpt.y = 10000; // some number > any screen resolution
maxpt.x = maxpt.y = 0;
for (i = 0; i < 4; i++) {
	minpt.x = min (minpt.x, a [i].x);
	maxpt.x = max (maxpt.x, a [i].x);
	minpt.y = min (minpt.y, a [i].y);
	maxpt.y = max (maxpt.y, a [i].y);
	}

	// clip min/max with screen min/max
minpt.x = max (minpt.x, 0);
maxpt.x = min (maxpt.x, width);
minpt.y = max (minpt.y, 0);
maxpt.y = min (maxpt.y, height);

// fill in texture
POINT b [4];  // Descent's (u,v) coordinates for textures

// map unit square into texture coordinate
//square2quad_matrix(A,a);
A.Square2Quad (a);

// calculate adjoint matrix (same as inverse)
//adjoint_matrix(A,IA);
IA = A.Adjoint ();

// store uv coordinates into b []
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
	int dlIdxCount = theMine->GameInfo ().lightDeltaIndices.count;
	CLightDeltaValue* lightDeltaValues;
	if (!lightStatus [face.m_nSegment][face.m_nSide].bIsOn &&
		 (lightDeltaIndices = theMine->LightDeltaIndex (0)) &&
		 (lightDeltaValues = theMine->LightDeltaValues (0))) {
		// search delta light index to see if current side has a light
		CLightDeltaIndex	*dli = lightDeltaIndices;
		for (i = 0; i <dlIdxCount; i++, dli++) {
//				if (dli->m_info.nSegment == theMine->current->segP) {
			// loop on each delta light till the segP/side is found
				CLightDeltaValue *dlP = theMine->LightDeltaValues (dli->m_info.index);
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
for (y = minpt.y; y < maxpt.y; y ++) {
	int x0, x1;
	// Determine min and max x for this y.
	// Check each of the four lines of the quadrilaterial
	// to figure out the min and max x
	x0 = maxpt.x; // start out w/ min point all the way to the right
	x1 = minpt.x; // and max point to the left
	for (i = 0; i < 4; i++) {
		// if line intersects this y then update x0 & x1
		j = (i + 1) % 4; // j = other point of line
		yi = a [i].y;
		yj = a [j].y;
		if ((y >= yi && y <= yj) || (y >= yj && y <= yi)) {
			w = yi - yj;
			if (w != 0.0) { // avoid divide by zero
				x = (int)(((double) a [i].x * ((double) y - (double) yj) - (double) a [j].x * ((double)  y - (double)  yi)) / w);
				if (x < x0) {
					scan_light = (int)(((double) light [i] * ((double) y - (double) yj) - (double) light [j] * ((double) y - (double) yi)) / w);
					x0 = x;
				}
				if (x>x1) {
					dscan_light = (int)(((double) light [i] * ((double) y - (double) yj) - (double) light [j] * ((double) y - (double) yi)) / w);
					x1 = x;
				}
			}
		}
	} // end for
	
	// clip
	x0 = max (x0, minpt.x);
	x1 = min (x1, maxpt.x);
	
	// Instead of finding every point using the matrix transformation,
	// just define the end points and delta values then simply
	// add the delta values to u and v
	if (fabs ((double) (x0 - x1)) >= 1.0) {
		double u0, u1, v0, v1, w0, w1, h, scale, x0d, x1d;
		uint u, v, du, dv, m, vd, vm, dx;
		dscan_light = (dscan_light - scan_light)/(x1-x0);
		
		// loop for every 32 bytes
		int end_x = x1;
		for (; x0 < end_x ; x0 += bmWidth2) {
			if (end_x - x0 > bmWidth2)
				x1 = bmWidth2 + x0;
			else
				x1 = end_x;

			scale = (double) max (bmWidth, bmHeight);
			//h = B.uVec[2]*(double) y + B.fVec[2];
			h = B.uVec.v.z * (double) y + B.fVec.v.z;
			x0d = (double) x0;
			x1d = (double) x1;
			w0 = (B.rVec[2] * x0d + h) / scale; // scale factor (64 pixels = 1.0 unit)
			w1 = (B.rVec[2] * x1d + h) / scale;
			if (fabs(w0)>0.0001 && fabs(w1)>0.0001) {
				h = B.uVec.v.x * (double) y + B.fVec.v.x;
				u0 = (B.rVec.v.x * x0d + h) / w0;
				u1 = (B.rVec.v.x * x1d + h) / w1;
				h = B.uVec.v.y * (double) y + B.fVec.v.y;
				v0 = (B.rVec.v.y * x0d + h) / w0;
				v1 = (B.rVec.v.y * x1d + h) / w1;
				
				// use 22.10 integer math
				// the 22 allows for large texture bitmap sizes
				// the 10 gives more then enough accuracy for the delta values

				m = min (bmWidth, bmHeight);
				if (!m)
					m = 64;
				m *= 1024;
				dx = x1-x0;
				if (!dx)
					dx = 1;
				du = ((uint) (((u1 - u0) * 1024.0) / dx) % m);
				// v0 & v1 swapped since bmData is flipped
				dv = ((uint) (((v0 - v1) * 1024.0) / dx) % m);
				u = ((uint) (u0 * 1024.0)) % m;
				v = ((uint) (-v0 * 1024.0)) % m;
				vd = 1024 / bmHeight;
				vm = bmWidth * (bmHeight - 1);
				
				ptr = pScrnMem + (uint)(height - y - 1) * (uint) rowOffset + x0;
				
				int k = (x1 - x0);
				if (y < (height-1) && k > 0)  {
					byte *pixelP;
					pixelP = ptr;
					if (bEnableShading) {
						int scanLightOffset = ((scan_light / 4) & 0x1f00);
						do {
							u += du;
							u %= m;
							v += dv;
							v %= m;
							byte temp = bmData [(u / 1024) + ((v / vd) & vm)];
							if (temp < 254) {
								temp = light_index [temp + scanLightOffset];
								*pixelP = temp;
								}
							pixelP++;
							scan_light += dscan_light;
							} while (--k);
						} 
					else {
						do {
							u += du;
							u %= m;
							v += dv;
							v %= m;
							byte temp = bmData [(u / 1024) + ((v / vd) & vm)];
							if (temp < 254)
								*pixelP = temp;
							pixelP++;
							} while (--k);
						}
					}
				}
			} // end of 32 byte loop
		}
	}
}
