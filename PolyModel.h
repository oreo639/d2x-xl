#ifndef __poly_h
#define __poly_h

#include "Vertex.h"
#include "GameObject.h"
#include "ViewMatrix.h"

//------------------------------------------------------------------------------

typedef struct {
  ushort index;
} BITMAP_INDEX;

typedef struct {
  byte	flags;		    //values defined above
  byte	pad[3];	    //keep alignment
  int		lighting;	    //how much light this casts
  int		damage;	    //how much damage being against this does (for lava)
  short	eclip_num;	    //the eclip that changes this, or -1
  short	destroyed;	    //bitmap to show when destroyed, or -1
  short	slide_u, slide_v;   //slide rates of texture, stored in 8:8 int
} TMAP_INFO;

typedef struct VCLIP {
  int		play_time;  //total time (in seconds) of clip
  int	num_frames;
  int		frame_time; //time (in seconds) of each frame
  int	flags;
  short	sound_num;
  ushort	frames[VCLIP_MAX_FRAMES];
  int		light_value;
} VCLIP;

typedef struct ECLIP {
  VCLIP   vc;			   //imbedded vclip
  int		 time_left;		   //for sequencing
  int	 frame_count;		   //for sequencing
  short	 changing_wall_texture;	   //Which element of Textures array to replace.
  short	 changing_object_texture;  //Which element of ObjBitmapPtrs array to replace.
  int	 flags;			   //see above
  int	 critClip;		   //use this clip instead of above one when mine critical
  int	 dest_bm_num;		//use this bitmap when monitor destroyed
  int	 dest_vclip;		//what vclip to play when exploding
  int	 dest_eclip;		//what eclip to play when exploding
  int	 destSize;		//3d size of explosion
  int	 sound_num;		//what sound this makes
  int	 nSegment,nSide;	//what segP & side, for one-shot clips
} ECLIP;

typedef struct WCLIP {
  int		 play_time;
  short	 num_frames;
  short	 frames[MAX_CLIP_FRAMES_D2];
  short	 open_sound;
  short	 close_sound;
  short	 flags;
  char	 filename[13];
  char	 pad;
} WCLIP;

//------------------------------------------------------------------------------
// the followin numbers are set using the original D2 robots

#define MAX_POLYMODEL_POINTS			416
#define MAX_POLYMODEL_POLYS			300
#define MAX_POLYMODEL_POLY_POINTS   13
#define MIN_POLYMODEL_POLY_POINTS	3
#define MAX_POLY_MODEL_SIZE			32000

//------------------------------------------------------------------------------

class CModelRenderPoly {
public:
	ushort		nVerts;
	CVertex		offset;
	CVertex		normal;
	ushort		nBaseTex;
	ushort		color;
	ushort		nGlow;
	ushort		verts [MAX_POLYMODEL_POLY_POINTS];
	tUVL			uvls [MAX_POLYMODEL_POLY_POINTS];

	void Draw (void);
};

//------------------------------------------------------------------------------

class CModelRenderData {
	public:
		ushort				nPoints;
		CVertex				points [MAX_POLYMODEL_POINTS];
		ushort				nPolys;
		CModelRenderPoly	polys [MAX_POLYMODEL_POLYS];
	};

//------------------------------------------------------------------------------

typedef struct tSubModel {
  int 			ptr;
  CFixVector 	offset;
  CFixVector 	norm;		// norm for sep plane
  CFixVector 	pnt;		// point on sep plane
  int 			rad;		// radius for each submodel
  byte 			parent;  // what is parent for each submodel
  CFixVector 	vMin;
  CFixVector   vMax;
} tSubModel;

//------------------------------------------------------------------------------
//used to describe a polygon model

typedef struct tPolyModel {
  int				nModels;
  int 			dataSize;
  byte*			renderData;
  tSubModel		subModels [MAX_SUBMODELS];
  CFixVector 	vMin, vMax;			  // min, max for whole model
  int				rad;
  byte			textureCount;
  ushort			firstTexture;
  byte			simplerModel;			  // alternate model with less detail (0 if none, nModel+1 else)
} tPolyModel;

//------------------------------------------------------------------------------

class CPolyModel {
public:
	tPolyModel		m_info;
	CViewMatrix*	m_view;
	CDC*				m_pDC;
	int				m_pt, m_pt0;

	CPolyModel () { Clear (); }
	
	~CPolyModel () { Release (); }
	
	inline void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }

	inline void Release (void) {
		if (m_info.renderData) {
			delete m_info.renderData;
			m_info.renderData = null;
			}
		}

	void Read (CFileManager& fp, bool bRenderData = false);

	void Write (CFileManager& fp, bool bRenderData = false);

	inline void Draw (CViewMatrix* view, CDC* pDC) {
		if (m_info.renderData) {
			m_view = view;
			m_pDC = pDC;
			Render (m_info.renderData);
			}
		}

	private:
		void SetPoints (int start, int end);

		void Render (byte* p);
};

//------------------------------------------------------------------------------

class CModelRenderer {
	friend class CPolyModel;
	friend class CModelRenderPoly;

	private:
		CGameObject*		m_object;
		CPolyModel*			m_model;
		CModelRenderData	m_data;
		CModelRenderPoly*	m_face;
		CViewMatrix*		m_view;
		CDC*					m_pDC;
		CVertex				m_offset;
		CDoubleVector		m_normal;
		APOINT				m_screenPoly [MAX_POLYMODEL_POINTS];
		int					m_nPolyPoints;
		int					m_lastObjType;
		int					m_lastObjId;
		int					m_nGlow;

#if _DEBUG
		CStaticArray< CPolyModel, MAX_POLYGON_MODELS > m_polyModels;
#else
		CPolyModel	m_polyModels [MAX_POLYGON_MODELS];
#endif

	public:
		CModelRenderer () : m_nGlow(-1), m_lastObjType(-1), m_lastObjId(-1) {}

		int Setup (CGameObject* objP, CViewMatrix* view, CDC* pDC);

		inline void Draw (void) { 
			if (m_model)
				m_model->Draw (m_view, m_pDC);
			};

		int CheckNormal (CFixVector& a, CFixVector& b) { return m_object->CheckNormal (*m_view, a, b); }

		int CheckNormal (CVertex& a, CVertex& b) { return m_object->CheckNormal (*m_view, a, b); }

	private:
		CPolyModel* Model (void);

		int ReadModelData (char* filename, char *szSubFile, bool bCustom);
};

extern CModelRenderer modelRenderer;

//-----------------------------------------------------------------------

#endif //__poly_h
