#ifndef __lightmap_h
#define __lightmap_h

#include "ogl_defs.h"
#include "carray.h"
#include "color.h"
#include "math.h"
#include "gr.h"
#include "menu.h"
#include "ogl_texture.h"

//------------------------------------------------------------------------------

#define MAX_LIGHTMAP_WIDTH	256
#define LIGHTMAP_WIDTH		lightmapWidth [gameOpts->render.nLightmapQuality]
#define LIGHTMAP_BUFWIDTH	512
#define LIGHTMAP_ROWSIZE	(LIGHTMAP_BUFWIDTH / LIGHTMAP_WIDTH)
#define LIGHTMAP_BUFSIZE	(LIGHTMAP_ROWSIZE * LIGHTMAP_ROWSIZE)

//------------------------------------------------------------------------------

typedef struct tLightmapInfo {
	CFixVector	vPos;
	CFixVector	vDir;  //currently based on face normals
	GLfloat		color [3];
	//float		bright;
	double		range;
	int			nIndex;  //(seg*6)+CSide ie which CSide the light is on
} tLightmapInfo;

typedef struct tLightmap {
	CFloatVector3	*bmP;
} tLightmap;

typedef struct tLightmapBuffer {
	GLuint		handle;
	CRGBColor	bmP [LIGHTMAP_BUFWIDTH][LIGHTMAP_BUFWIDTH];
} tLightmapBuffer;

typedef struct tLightmapList {
	CArray<tLightmapInfo>	info;
	CArray<tLightmapBuffer>	buffers;
	int							nBuffers;
	int							nLights; 
	ushort						nLightmaps;
} tLightmapList;

typedef CSegFace* tSegFacePtr;


typedef struct tLightmapData {
	int						nType;
	int						nColor;
	CFixVector				vNormal;
	ushort					sideVerts [4]; 
	CVertColorData			vcd;
	CRGBColor				texColor [MAX_LIGHTMAP_WIDTH * MAX_LIGHTMAP_WIDTH];
	CFixVector				pixelPos [MAX_LIGHTMAP_WIDTH * MAX_LIGHTMAP_WIDTH]; 
	int						nOffset [MAX_LIGHTMAP_WIDTH];
	CArray<tSegFacePtr>	faceList;
	CSegFace*				faceP;
	} tLightmapData;

class CLightmapManager {
	private:
		tLightmapData	m_data;
		tLightmapList	m_list;

	public:
		CLightmapManager () { Init (); } 
		~CLightmapManager () { Destroy (); }
		void Init (void);
		void Setup (int nLevel);
		void Destroy (void);
		void RestoreLights (int bVariable);
		int Bind (int nLightmap);
		int BindAll (void);
		void Release (void);
		int Create (int nLevel);
		void Build (int nThread);
		void BuildAll (int nFace);
		inline tLightmapBuffer* Buffer (uint i = 0) { return &m_list.buffers [i]; }
		inline int HaveLightmaps (void) { return !gameStates.app.bNostalgia && (m_list.buffers.Buffer () != NULL); }

	private:
		int Init (int bVariable);
		inline void ComputePixelOffset (CFixVector& vPos, CFixVector& v1, CFixVector& v2, int nOffset);
		double SideRad (int nSegment, int nSide);
		int CountLights (int bVariable);
		void Copy (CRGBColor *texColorP, ushort nLightmap);
		void CreateSpecial (CRGBColor *texColorP, ushort nLightmap, ubyte nColor);
		void Realloc (int nBuffers);
		int Save (int nLevel);
		int Load (int nLevel);
		char* Filename (char *pszFilename, int nLevel);

		static int CompareFaces (const tSegFacePtr* pf, const tSegFacePtr* pm);
	};

extern CLightmapManager lightmapManager;

//------------------------------------------------------------------------------


#define	USE_LIGHTMAPS \
			(gameStates.render.bLightmapsOk && \
			 gameOpts->render.bUseLightmaps && \
			 !IsMultiGame && \
			 (gameOpts->render.nLightingMethod == 0))

//------------------------------------------------------------------------------

//extern CTexture	*lightmaps;
extern tLightmapData		lightmapData;
extern int					lightmapWidth [5];
extern GLhandleARB		lmShaderProgs [3];

//------------------------------------------------------------------------------

int SetupLightmap (CSegFace* faceP);

//------------------------------------------------------------------------------

#endif //__lightmap_h
