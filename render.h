// render.h

void TextureMap(INT32 resolution,
				CDSegment *segment,
				INT16 nSide,
				UINT8 *bmData,
				UINT16 bmWidth,
				UINT16 bmHeight,
				UINT8 *light_index,
				UINT8 *pScrnMem,
				APOINT* scrn,
				UINT16 width,
				UINT16 height,
				UINT16 rowOffset);

double dround_off(double value, double round);
/*
double CalcLength(tFixVector &center1,tFixVector &center2);
void CalcCenter(CMine &mine, tFixVector &center,INT16 nSegment,INT16 nSide);
void CalcOrthoVector(CMine *mine, tFixVector &result,INT16 nSegment,INT16 nSide);
*/