#ifndef __ModelManager_h
#define __ModelManager_h

#include "Vertex.h"
#include "GameObject.h"
#include "ViewMatrix.h"
#include "Renderer.h"
#include "ASEModel.h"
#include "RenderModel.h"

//------------------------------------------------------------------------------

#define MAX_OBJ_TEXTURES 600
#define MAX_ANIMATIONS	110

typedef struct {
  ushort index;
} BITMAP_INDEX;

typedef struct {
  ubyte	flags;				//values defined above
  ubyte	pad[3];				//keep alignment
  int		lighting;			//how much light this casts
  int		damage;				//how much damage being against this does (for lava)
  short	nEffectInfo;		//the eclip that changes this, or -1
  short	destroyed;			//bitmap to show when destroyed, or -1
  short	slide_u, slide_v; //slide rates of texture, stored in 8:8 int
} TMAP_INFO;

typedef struct tAnimationInfo {
  int		nPlayTime;  //total time (in seconds) of clip
  int		nFrameCount;
  int		nFrameTime; //time (in seconds) of each frame
  int		nFlags;
  short	nSound;
  ushort	frames [VCLIP_MAX_FRAMES];
  int		nLightValue;
} tAnimationInfo;

typedef struct tEffectAnimationInfo {
  tAnimationInfo  vc;					//imbedded vclip
  int		nTimeLeft;						//for sequencing
  int		nFrameCount;					//for sequencing
  short	nChangingWallTexture;	   //Which element of Textures array to replace.
  short	nChangingObjectTexture;		//Which element of ObjBitmapPtrs array to replace.
  int		flags;							//see above
  int		nCriticalAnimation;			//use this clip instead of above one when mine critical
  int		nDestTexture;					//use this bitmap when monitor destroyed
  int		nDestAnimation;				//what vclip to play when exploding
  int		nDestEffect;					//what eclip to play when exploding
  int		nDestSize;						//3d size of explosion
  int		nSound;							//what sound this makes
  int		nSegment, nSide;				//what segP & side, for one-shot clips
} tEffectAnimationInfo;

typedef struct tWallAnimationInfo {
  int		 nPlayTime;
  short	 nFrameCount;
  short	 frames [MAX_CLIP_FRAMES_D2];
  short	 nOpenSound;
  short	 nCloseSound;
  short	 nFlags;
  char	 filename [13];
  char	 pad;
} tWallAnimationInfo;

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
	short			nTexture;
	rgbColord	color;
	ushort		nGlow;
	ushort		index [MAX_POLYMODEL_POLY_POINTS];
	tTexCoord2d	texCoords [MAX_POLYMODEL_POLY_POINTS];

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

class CModelManager {
	friend class CPolyModel;
	friend class CModelRenderPoly;

	private:
		CGameObject*		m_object;
		CModelRenderData	m_data;
		CModelRenderPoly*	m_face;
		CRenderer*			m_renderer;
		CViewMatrix*		m_viewMatrix;
		CDC*					m_pDC;
		CVertex				m_offset;
		CDoubleVector		m_normal;
		CLongVector			m_screenRect [2];
		ushort				m_textureIndex [2][MAX_OBJ_TEXTURES];
		ushort				m_modelTextureIndex [2][MAX_OBJ_TEXTURES];
		tAnimationInfo		m_animations [MAX_ANIMATIONS];
		CVertex				m_screenPoly [MAX_POLYMODEL_POINTS];
		int					m_nPolyPoints;
		int					m_lastObjType;
		int					m_lastObjId;
		int					m_nModel;
		int					m_nGlow;
		int					m_nTextures;
		int					m_nIndices;

#if _DEBUG
		CStaticArray< CPolyModel, MAX_POLYGON_MODELS > m_polyModels [2];
		CStaticArray< ASE::CModel, MAX_POLYGON_MODELS > m_aseModels;
		CStaticArray< OOF::CModel, MAX_POLYGON_MODELS > m_oofModels;
		CStaticArray< RenderModel::CModel, MAX_POLYGON_MODELS > m_renderModels;
#else
		CPolyModel	m_polyModels [2][MAX_POLYGON_MODELS];
		ASE::CModel m_aseModels [MAX_POLYGON_MODELS];
		OOF::CModel m_oofModels [MAX_POLYGON_MODELS];
		RenderModel::CModel m_renderModels [MAX_POLYGON_MODELS];
#endif

	public:
		CModelManager () : m_nGlow(-1), m_lastObjType(-1), m_lastObjId(-1) {}

		int Setup (CGameObject* objP, CRenderer* renderer, CDC* pDC);

		void Reset (void);

		void Draw (void);

		inline int CheckNormal (CFixVector& a, CFixVector& b) { return m_object->CheckNormal (*m_viewMatrix, a, b); }

		inline int CheckNormal (CVertex& a, CVertex& b) { return m_object->CheckNormal (*m_viewMatrix, a, b); }

		void SetPoints (int start, int end);

		void Polygon (ushort* index, int nVertices, tTexCoord2d* texCoords, rgbColord* color, short nTexture);

		inline tAnimationInfo* Animation (int i) { return (i < MAX_ANIMATIONS) ? &m_animations [i] : null; }

		inline int ScreenRad (void) {
			int dx = abs (m_screenRect [1].x - m_screenRect [0].x);
			int dy = abs (m_screenRect [1].y - m_screenRect [0].y);
			return (dx < dy) ? dy / 2 : dx / 2;
			}

		void ReadCustomModelData (ubyte* buffer, long bufSize, int bCustom = 1);

		void ReadMod (char* pszFolder);

		void LoadMod (void);

		int BuildModel (CObject* objP, int nModel, CPolyModel* pp, CDynamicArray<CTexture*>& modelTextures);

	private:
		int Model (void);

		int ReadModelData (char* filename, char *szSubFile, bool bCustom);

		void ReadCustomModelData (CMemoryFile& mf, int bCustom);

		void ReadCustomModelData (char* filename, char *szSubFile, bool bCustom);
	};

extern CModelManager modelManager;

//-----------------------------------------------------------------------

#endif //__ModelManager_h
