#ifndef __poly_h
#define __poly_h

/* the followin numbers are set using the original D2 robots */
#define MAX_POLYMODEL_POINTS			416
#define MAX_POLYMODEL_POLYS			300
#define MAX_POLYMODEL_POLY_POINTS   13
#define MIN_POLYMODEL_POLY_POINTS	3
#define MAX_POLY_MODEL_SIZE			32000

typedef struct tModelRenderPoly {
public:
	UINT16		n_verts;
	CVertex		offset;
	CVertex		normal;
	UINT16		nBaseTex;
	UINT16		color;
	UINT16		glow_num;
	UINT16		verts [MAX_POLYMODEL_POLY_POINTS];
	tUVL			uvls [MAX_POLYMODEL_POLY_POINTS];

	void Draw (CViewMatrix* view, CDC* pDC);
} tModelRenderPoly;

typedef struct tModelRenderData {
	UINT16				n_points;
	CVertex				points [MAX_POLYMODEL_POINTS];
	UINT16				n_polys;
	tModelRenderPoly	polys [MAX_POLYMODEL_POLYS];
} tModelRenderData;

typedef struct tSubModel {
  INT32 			ptr;
  CFixVector 	offset;
  CFixVector 	norm;		// norm for sep plane
  CFixVector 	pnt;		// point on sep plane
  FIX 			rad;		// radius for each submodel
  UINT8 			parent;  // what is parent for each submodel
  CFixVector 	vMin;
  CFixVector   vMax;
} tSubModel;

//used to describe a polygon model
typedef struct tPolyModel {
  INT32			nModels;
  INT32 			dataSize;
  UINT8*			renderData;
  tSubModel		subModels [MAX_SUBMODELS];
  CFixVector 	vMin, vMax;			  // min, max for whole model
  FIX				rad;
  UINT8			textureCount;
  UINT16			firstTexture;
  UINT8			simplerModel;			  // alternate model with less detail (0 if none, nModel+1 else)
//  CFixVector min, max;
} tPolyModel;

class CPolyModel {
public:
	tPolyModel		m_info;
	CViewMatrix*	m_view;
	CDC*				m_pDC;

	CPolyModel () { Clear (); }
	~CPolyModel () { Release (); }
	inline void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		}
	inline void Release (void) {
		if (m_info.renderData) {
			delete m_info.renderData;
			m_info.renderData = NULL;
			}
		}
	void Read (FILE* fp, bool bRenderData = false);
	void Write (FILE* fp, bool bRenderData = false);
	inline void CPolyModel::Draw (CViewMatrix* view, CDC* pDC) {
		if (m_info.renderData) {
			m_view = view;
			m_pDC = pDC;
			Render (m_info.renderData);
			}
		}

	private:
		void SetModelPoints (INT32 start, INT32 end);
		void Render (UINT8* p);
};

//void interp_model_data (void *model_data, CFixVector* offset, tModelRenderData *model, UINT16 call_level) ;

#endif //__poly_h
