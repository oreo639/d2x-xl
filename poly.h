#ifndef __poly_h
#define __poly_h

/* the followin numbers are set using the original D2 robots */
#define MAX_POLY_MODEL_POINTS 416
#define MAX_POLYS             300
#define MAX_POLY_POINTS       13
#define MIN_POLY_POINTS       3
#define MAX_POLY_MODEL_SIZE   32000
#define UVL CUVL

typedef struct tRenderModelData {
public:
	  UINT16			n_verts;
	  CFixVector	offset;
	  CFixVector	normal;
	  UINT16			nBaseTex;
	  UINT16			color;
	  UINT16			glow_num;
	  UINT16			verts [MAX_POLY_POINTS];
	  UVL				uvls [MAX_POLY_POINTS];

	void Read (FILE* fp);
	void Write (FILE* fp);
} tRenderModelData;

typedef struct tRenderModel {
public:
	UINT16				n_points;
	CVertex				points [MAX_POLY_MODEL_POINTS];
	UINT16				n_polys;
	tRenderModelData*	modelData;
} tRenderModel;

void interp_model_data(
    void *model_data,
    CFixVector* offset,
    MODEL *model,
    UINT16 call_level
);

#endif //__poly_h
