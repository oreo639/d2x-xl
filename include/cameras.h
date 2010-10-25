#ifndef _cameras_h
#define _cameras_h

#include "ogl_defs.h"
#include "ogl_texture.h"

#define MAX_CAMERAS	100

typedef struct tCamera {
	CObject			obj;
	CObject			*objP;
	short				nId;
	short				nSegment;
	short				nSide;
	ubyte				*screenBuf;
	GLuint			glTexId;
	time_t			nTimeout;
	char				nType;
	char				bValid;
	char				bVisible;
	char				bTimedOut;
	char				bAligned;
	char				bShadowMap;
	char				bTeleport;
	char				bMirror;
	int				nWaitFrames;
	tUVL				uvlList [4];
	tTexCoord2f		texCoord [6];
#if RENDER2TEXTURE == 1
	tPixelBuffer	pb;
	CTexture		glTex;
#elif RENDER2TEXTURE == 2
	CFBO				fbo;
	CTexture			glTex;
#endif
	CFixMatrix		orient;
	fixang			curAngle;
	fixang			curDelta;
	time_t			t0;
} tCamera;

class CCamera : public CCanvas {
	private:
		tCamera	m_data;
	public:
		CCamera () { Init (); };
		~CCamera () { Destroy (); };
		void Init (void);
		int Render (void);
		int Ready (time_t t);
		void Reset (void);
		void Rotate (void);
		void Align (CSegFace *faceP, tUVL *uvlP, tTexCoord2f *texCoordP, CFloatVector3 *vertexP);
		int Create (short nId, short srcSeg, short srcSide, short tgtSeg, short tgtSide, 
						CObject *objP, int bShadowMap, int bTeleport);
		void Destroy (void);
		int HaveBuffer (int bCheckTexture);
		int HaveTexture (void);
		inline void ReAlign (void) { m_data.bAligned = 0; }
		inline void SetVisible (char bVisible) { m_data.bVisible = bVisible; }
		inline char GetVisible (void) { return m_data.bVisible; }
		inline char GetTeleport (void) { return m_data.bTeleport; }
		inline char Valid (void) { return m_data.bValid; }
		inline CFixMatrix& Orient (void) { return m_data.orient; }
		//inline CBitmap& Buffer (void) { return *this; }
		inline tTexCoord2f* TexCoord (void) { return m_data.texCoord; }
		inline CObject* GetObject (void) { return m_data.objP; }
		inline CFBO& FrameBuffer (void) { return m_data.fbo; } 
		int EnableBuffer (void);
		int DisableBuffer (bool bPrepare = true);

	private:
		int CreateBuffer (void);
		int BindBuffer (void);
		int ReleaseBuffer (void);
		int DestroyBuffer (void);
	};

//------------------------------------------------------------------------------

class CCameraManager {
	private:
		CCamera			m_cameras [MAX_CAMERAS];
		CCamera*			m_current;
		short				m_nCameras;
		CArray<char>	m_faceCameras;
		CArray<ushort>	m_objectCameras;

	public:
		CCameraManager () : m_current (NULL), m_nCameras (0), m_objectCameras (0) {}
		~CCameraManager ();
		int Create ();
		void Destroy ();
		int Render ();
		void Rotate (CObject *objP);
		inline CCamera* Cameras (void) { return m_cameras; }
		inline CCamera* Camera (short i = 0) { return Cameras () + i; }
		inline int Current (void) { return int (m_current - m_cameras); }
		CCamera* Camera (CObject *objP);
		inline int GetObjectCamera (int nObject);
		inline void SetObjectCamera (int nObject, int i);
		int GetFaceCamera (int nFace);
		void ReAlign (void);
		inline void SetFaceCamera (int nFace, int i);
		inline int Index (CCamera* cameraP) { return int (cameraP - m_cameras); }
	};

extern CCameraManager cameraManager;

//------------------------------------------------------------------------------

#define USE_CAMERAS (extraGameInfo [0].bUseCameras && (!IsMultiGame || (gameStates.app.bHaveExtraGameInfo [1] && extraGameInfo [1].bUseCameras)))

//------------------------------------------------------------------------------

#endif // _cameras_h
