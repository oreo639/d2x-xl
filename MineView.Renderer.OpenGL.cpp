// render.cpp

#include "stdafx.h"
#include <math.h>

#include "define.h"
#include "types.h"
#include "matrix.h"
#include "global.h"
#include "mine.h"
#include "segment.h"
#include "GameObject.h"
#include "dle-xp.h"
#include "dlcdoc.h"
#include "MainFrame.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "mineview.h"

extern short nDbgSeg, nDbgSide;
extern int nDbgVertex;

//------------------------------------------------------------------------------

static int bInitSinCosTable = 1;

typedef struct tSinCosf {
	float	fSin, fCos;
} tSinCosf;

void ComputeSinCosTable (tSinCosf *sinCosP, int nPoints)
{
for (int i = 0; i < nPoints; i++, sinCosP++) {
	double a = 2.0 * PI * i / nPoints;
	sinCosP->fSin = (float) sin (a);
	sinCosP->fCos = (float) cos (a);
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

CRendererGL::CRendererGL (CRenderData& renderData) : CRenderer (renderData), m_x (0), m_y (0)
{
m_glRC = null;
m_glDC = null;
}

// -----------------------------------------------------------------------------

int CRendererGL::Setup (CWnd* pParent, CDC* m_pDC)	
{
if (!CRenderer::Setup (pParent, m_pDC))
	return 0;
if (!CreateContext ())
	return 0;

InitProjection ();

CRect rc;
m_pParent->GetClientRect (rc);
if (!(rc.Width () && rc.Height ()))
	return 0;
glViewport (rc.left, rc.top, rc.Width (), rc.Height ());
m_glAspectRatio = (GLfloat) rc.Width () / (GLfloat) rc.Height ();
SetupProjection ();
return -1;
}

//------------------------------------------------------------------------------

void CRendererGL::Reset (void)
{
CRenderer::Reset ();
m_vZoom.Clear ();
m_viewMatrix.Origin ().Clear ();
if (Perspective ()) {
	Rotation ().Clear ();
	Translation () = -current->Segment ()->Center ();
	}
ViewMatrix ()->Setup (Translation (), Scale (), Rotation ());
}

//------------------------------------------------------------------------------

void CRendererGL::ClearView (void)
{
glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// -----------------------------------------------------------------------------

GLvoid CRendererGL::DestroyContext (void)
{
if (m_glRC)	{
	if (!wglMakeCurrent (NULL,NULL))
		ErrorMsg ("OpenGL: Release of DC and RC failed.");
	if (!wglDeleteContext(m_glRC))
		ErrorMsg ("OpenGL: Release of rendering context failed.");
		m_glRC = NULL;
	}
if (m_glDC && !::ReleaseDC (m_pParent->m_hWnd, m_glHDC))	 {
	//ErrorMsg ("OpenGL: Release of device context failed.")
		;
	m_glDC = NULL;	
	}
#if 0
if (!UnregisterClass ("OpenGL",AfxGetInstance ())) {
	ErrorMsg ("OpenGL: Could Not Unregister Class.");
	}
#endif
}

// -----------------------------------------------------------------------------

BOOL CRendererGL::CreateContext (void)
{
if (m_glDC)
	return TRUE;

	GLuint PixelFormat;

static PIXELFORMATDESCRIPTOR pfd = {
	sizeof (PIXELFORMATDESCRIPTOR),
	1,	
	PFD_DRAW_TO_WINDOW |	PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,
	32,
	0, 0, 0, 0, 0, 0,
	0,	
	0,
	0,
	0, 0, 0, 0,
	32,
	0,
	0,
	PFD_MAIN_PLANE,
	0,
	0, 0, 0
	};

if (!(m_glDC = m_pParent->GetDC ())) {
	DestroyContext ();
	ErrorMsg ("OpenGL: Can't create device context.");
	return FALSE;
	}
m_glHDC = m_glDC->GetSafeHdc ();
if (!(PixelFormat = ChoosePixelFormat (m_glHDC, &pfd))) {
	DestroyContext ();
	sprintf_s (message, sizeof (message), "OpenGL: Can't find a suitable pixel format. (%d)", GetLastError ());
	ErrorMsg (message);
	return FALSE;
	}
if(!SetPixelFormat (m_glHDC, PixelFormat, &pfd)) {
		DestroyContext();
		sprintf_s (message, sizeof (message), "OpenGL: Can't set the pixel format (%d).", GetLastError ());
		ErrorMsg (message);
		return FALSE;
	}
if (!(m_glRC = wglCreateContext (m_glHDC))) {
	DestroyContext ();
	sprintf_s (message, sizeof (message), "OpenGL: Can't create a rendering context (%d).", GetLastError ());
	ErrorMsg (message);
	return FALSE;
	}

if(!wglMakeCurrent (m_glHDC, m_glRC)) {
	DestroyContext ();
	sprintf_s (message, sizeof (message), "OpenGL: Can't activate the rendering context (%d).", GetLastError ());
	ErrorMsg (message);
	return FALSE;
	}

glewInit (); // must happen after OpenGL context creation!
shaderManager.Setup ();
textureManager.InitShaders ();

return TRUE;
}

//------------------------------------------------------------------------------

int CRendererGL::Project (CRect* pRC, bool bCheckBehind)
{
CHECKMINEV(0);

	CViewMatrix* viewMatrix = ViewMatrix ();
	long	i, j, nProjected;
	bool	bBehind = false;

viewMatrix->Translate (Translation ());
InitViewDimensions ();
viewMatrix->SetViewInfo (ViewWidth (), ViewHeight ());

nProjected = 0;
j = vertexManager.Count ();
#pragma omp parallel for reduction(+: nProjected)
for (i = 0; i < j; i++) {
	CVertex& v = vertexManager [i];
	if (v.Status () == 255) // skybox inclusion/exclusion
		continue;
	v.Transform (viewMatrix);
	if (v.m_view.v.z < 0.0)
		bBehind = true;
	nProjected++;
	}

if (!nProjected)
	return 0;
if (bCheckBehind && bBehind)
	return -1;

for (i = 0; i < j; i++) {
	CVertex& v = vertexManager [i];
	if (v.Status () != 255)
		v.Project (viewMatrix);
	}
ComputeViewLimits (pRC);
return nProjected;
}

//------------------------------------------------------------------------------

int CRendererGL::ZoomIn (int nSteps, bool bSlow)
{
Zoom (nSteps, zoomScales [1]);
return nSteps;
}

//------------------------------------------------------------------------------

int CRendererGL::ZoomOut (int nSteps, bool bSlow)
{
Zoom (nSteps, 1.0 / zoomScales [1]);
return nSteps;
}

//------------------------------------------------------------------------------

void CRendererGL::ComputeZoom (void)
{
if (!Perspective ()) {
	double m = Translation ().Mag ();
	m_vZoom [0] = (m < 0.0) ? double (int ((m - 10.0) / 20.0)) : double (int (sqrt ((m + 10.0) / 20.0)));
	}
}

//------------------------------------------------------------------------------

void CRendererGL::Zoom (int nSteps, double zoom)
{
if (Perspective ()) {
	Pan ('Z', nSteps * ((zoom < 1.0) ? 1 : -1));
	}
else {
	if (nSteps < 0) {
		if (zoom > 1.0)
			Translation ().v.z += double (-nSteps);
		else
			Translation ().v.z -= double (-nSteps);
		}
	else {
		if (zoom > 1.0)
			m_vZoom [0] -= 1.0 * double (nSteps);
		else 
			m_vZoom [0] += 1.0 * double (nSteps);
#if 1
		Translation ().v.z = (m_vZoom [0] > 0.0) ? m_vZoom [0] * m_vZoom [0] * 20.0 : m_vZoom [0] * 20.0;
#else
		if ((m_vZoom [0] < 0) != (zoom < 1.0))
			zoom = 1.0 / zoom;
		double z = ((zoom < 1.0) && (m_vZoom [0] < 0)) 
					  ? -(100.0 + pow (1.0 / zoom, fabs (m_vZoom [0])) * ViewMoveRate ())
					  : (100.0 + pow (zoom, fabs (m_vZoom [0])) * ViewMoveRate ());
		Translation ().v.z = z;
#endif
		}
	}
//CRenderer::Zoom (nSteps, zoom);
}

//------------------------------------------------------------------------------

void CRendererGL::SetCenter (CVertex v, int nType)
{
#if 1
#	if 0
Translation ().v.x = v.v.x;
Translation ().v.y = v.v.y;
#	else
if (Perspective ())
	Translation () = -v;
else {
	m_viewMatrix.Origin () = -v;
	Translation ().v.x = 
	Translation ().v.y = 0.0;
	ComputeZoom ();
	}
#	endif
#else
if (nType == 0) { // mine
	Translation () = v;
	m_vZoom.Clear ();
	}
else { // segment or object
	Translation ().v.x = -v.v.x;
	Translation ().v.y = -v.v.y;
	}
ComputeZoom ();
#endif
}

//------------------------------------------------------------------------------

void CRendererGL::Pan (char axis, double offset)
{
if (offset == 0.0)
	return;

if (Perspective ()) {
	CVertex vMove;
	switch (axis) {
		case 'X':
			vMove = ViewMatrix ()->Right ();
			break;
		case 'Y':
			vMove = ViewMatrix ()->Up ();
			break;
		case 'Z':
			vMove = ViewMatrix ()->Backward ();
			break;
		default:
			return;
		}
	vMove *= offset * ViewMoveRate ();
	Translation () += vMove;
	}
else {
	int i = axis -= 'X';
	if ((i < 0) || (i > 2))
		return;
#if 1
	double d = ViewMoveRate () * ((i == 1) ? offset : -offset);
	if (i == 2)
		Translation () [i] += d;
	else {
		CVertex o = m_viewMatrix.Origin ();
		o.m_view = m_viewMatrix.Transformation () * o;
		o.m_view [i] -= d;
		o = m_viewMatrix.Transformation (1) * o.m_view;
		m_viewMatrix.Origin () = o;
		}
#else
m_viewMatrix.MoveViewer (i, ViewMoveRate () * ((i == 1) ? offset : -offset));
#endif
	}
}

//------------------------------------------------------------------------------

void CRendererGL::DrawFaceTextured (CFaceListEntry& fle) 
{
	CTexture		tex (textureManager.m_bmBuf), * texP [3] = {null, null, null};
	CSegment*	segP = segmentManager.Segment (fle);
	CSide*		sideP = segmentManager.Side (fle);
	CWall*		wallP = segmentManager.Wall (fle);
	CBGRA			color, * colorP = null;
	int			bArrow = 0;

#ifdef _DEBUG
if ((fle.m_nSegment == nDbgSeg) && ((nDbgSide < 0) || (fle.m_nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
#endif

SetAlpha (255);
short nBaseTex = sideP->BaseTex ();
short nOvlTex = sideP->OvlTex (0);

#if 0
texP [0] = &textureManager.Arrow ();
#else
if (textureManager.IsAnimated (sideP->BaseTex ()) && !(bArrow = textureManager.HaveArrow ())) {
	textureManager.BlendTextures (nBaseTex, nOvlTex, texP [0] = &tex, 0, 0);
	tex.DrawAnimDirArrows (nBaseTex);
	}
else {
	texP [0] = textureManager.Texture (nBaseTex);
	texP [0]->m_nTexture = nBaseTex;
	if (nOvlTex) {
		texP [1] = textureManager.Texture (nOvlTex);
		texP [1]->m_nTexture = nOvlTex;
		}
#if 1
	if (bArrow)
		texP [2] = &textureManager.Arrow ();
#endif
	}
if (wallP != null) {
	SetAlpha (wallP->Alpha ());
	if (wallP->IsTransparent () || wallP->IsCloaked ()) {
		CBGRA color;
		if (wallP->IsCloaked ())
			color = CBGRA (0, 0, 0, 255);
		else {
			CColor* texColorP = lightManager.GetTexColor (sideP->BaseTex (), true);
			color = CBGRA (texColorP->Red (), texColorP->Green (), texColorP->Blue (), 255);
			}
		colorP = &color;
		}
	}
#endif

RenderFace (fle, texP, colorP);
if	(texP [0] == &tex)
	texP [0]->GLRelease ();
}

//--------------------------------------------------------------------------

void CRendererGL::BeginRender (bool bOrtho)
{
SetupProjection (bOrtho);
}

//--------------------------------------------------------------------------

void CRendererGL::EndRender (bool bSwapBuffers)
{
for (int i = 0; i < 3; i++) {
	glActiveTexture (GL_TEXTURE0 + i);
	glClientActiveTexture (GL_TEXTURE0 + i);
	glDisableClientState (GL_COLOR_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisable (GL_TEXTURE_2D);
	}
shaderManager.Deploy (-1);
glDisable (GL_DEPTH_TEST);
if (bSwapBuffers)
	SwapBuffers (m_glHDC);
}

//------------------------------------------------------------------------------

static inline void RotateTexCoord2d (tTexCoord2d& dest, tTexCoord2d& src, short nOrient)
{
if (nOrient == 3) {
	dest.u = 1.0f - src.v;
	dest.v = src.u;
	}
else if (nOrient == 2) {
	dest.u = 1.0f - src.u;
	dest.v = 1.0f - src.v;
	}
else if (nOrient == 1) {
	dest.u = src.v;
	dest.v = 1.0f - src.u;
	}
else {
	dest.u = src.u;
	dest.v = src.v;
	}
}

//------------------------------------------------------------------------------

#define GL_TRANSFORM 0

void CRendererGL::RenderFace (CFaceListEntry& fle, CTexture* texP [], CBGRA* colorP)
{
	static float zoom = 10.0f;
	static double scrollAngles [8] = {0.0, Radians (45.0), Radians (90.0), Radians (135.0), Radians (180.0), Radians (-135.0), Radians (-90.0), Radians (-45.0)};

#if 1
	CSegment*	segP = segmentManager.Segment (fle.m_nSegment);
	CSide*		sideP = segP->Side (fle.m_nSide);
	CWall*		wallP = sideP->Wall ();
	float			alpha = float (Alpha ()) / 255.0f * (colorP ? float (colorP->a) / 255.0f : 1.0f);
	int			bIlluminate = RenderIllumination ();
	ushort*		vertexIds = segP->m_info.vertexIds;
	ubyte*		vertexIdIndex = sideP->m_vertexIdIndex;
	int			nVertices = sideP->VertexCount (), 
					nTextures = colorP ? 0 : texP [1] ? 2 : 1;
	int			bArrow = texP [2] != null;
	double		scrollAngle = 0.0;
	short			nOvlAlignment = sideP->OvlAlignment ();
	ushort		brightness [4];

	tDoubleVector	vertices [4];
	tTexCoord2d		texCoords [3][4];
	rgbaColorf		colors [4];

#ifdef _DEBUG
if ((fle.m_nSegment == nDbgSeg) && ((nDbgSide < 0) || (fle.m_nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
#endif

if (bArrow) {
	int nDir = textureManager.ScrollDirection (texP [0]->m_nTexture);
	if (nDir > 0)
		scrollAngle = scrollAngles [nDir];
	}

ComputeBrightness (fle, brightness, RenderVariableLights ());

for (int i = 0, j = nOvlAlignment; i < nVertices; i++, j = (j + 1) % nVertices) {
	int nVertex = vertexIds [vertexIdIndex [i]];
#if GL_TRANSFORM
	vertices [i] = vertexManager [nVertex].v;
#else
	vertices [i] = vertexManager [nVertex].m_view.v;
#endif
	CUVL uvl = sideP->m_info.uvls [i];
	if (!colorP) {
		texCoords [0][i].u = uvl.u;
		texCoords [0][i].v = uvl.v;
		if (nTextures > 1)
			RotateTexCoord2d (texCoords [1][i], texCoords [0][i], nOvlAlignment);
		if (bArrow) {
			if (scrollAngle != 0.0)
				uvl.Rotate (scrollAngle);
			texCoords [nTextures][i].u = uvl.u;
			texCoords [nTextures][i].v = 1.0 - uvl.v;
			}
		if (bIlluminate) {
			float b = 2.0f * X2F (brightness [i]);
			CVertexColor* vertexColor = lightManager.VertexColor (nVertex);
			colors [i].r = float (vertexColor->m_info.color.r) * b;
			colors [i].g = float (vertexColor->m_info.color.g) * b;
			colors [i].b = float (vertexColor->m_info.color.b) * b;
			}
		else
			colors [i].r = 
			colors [i].g = 
			colors [i].b = 1.0f;
		colors [i].a = alpha;
		}
	}

#if 0
if (nTextures == 2)
	textureManager.DeployShader ();
else
	shaderManager.Deploy (-1);
#endif

if (colorP) {
	glDisable (GL_TEXTURE_2D);
	glColor4f (float (colorP->r) / 255.0f, float (colorP->g) / 255.0f, float (colorP->b) / 255.0f, alpha);
	glDisableClientState (GL_COLOR_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnableClientState (GL_VERTEX_ARRAY);
	glVertexPointer (3, GL_DOUBLE, 0, vertices);
	}
else {
	int h = 0;
	for (int i = 0; i < 3; i++) {
		if (!texP [i]) 
			continue;
		glActiveTexture (GL_TEXTURE0 + h);
		glClientActiveTexture (GL_TEXTURE0 + h);
		glEnable (GL_TEXTURE_2D);
		if (!texP [i]->GLBind (GL_TEXTURE0 + h, /*(h == 1) ? GL_DECAL :*/ GL_MODULATE))
			return;
		glEnableClientState (GL_COLOR_ARRAY);
		glEnableClientState (GL_TEXTURE_COORD_ARRAY);
		glEnableClientState (GL_VERTEX_ARRAY);
		glColorPointer (4, GL_FLOAT, 0, colors);
		glTexCoordPointer (2, GL_DOUBLE, 0, texCoords [h]);
		glVertexPointer (3, GL_DOUBLE, 0, vertices);
		h++;
		}
#if 1
	for (; h < 3; h++) {
		glActiveTexture (GL_TEXTURE0 + h);
		glClientActiveTexture (GL_TEXTURE0 + h);
		glBindTexture (GL_TEXTURE_2D, 0);
		glDisable (GL_TEXTURE_2D);
		}
#endif
	}

#if 1
if (nTextures + bArrow > 1)
	textureManager.DeployShader (bArrow ? nTextures : 0);
else
	shaderManager.Deploy (-1);
#endif

#if GL_TRANSFORM
m_viewMatrix.SetupOpenGL ();
glDrawArrays ((nVertices == 3) ? GL_TRIANGLES : GL_TRIANGLE_FAN, 0, GLsizei (nVertices));
m_viewMatrix.ResetOpenGL ();
#else
glDrawArrays ((nVertices == 3) ? GL_TRIANGLES : GL_TRIANGLE_FAN, 0, GLsizei (nVertices));
#endif
#endif
}

//------------------------------------------------------------------------------

void CRendererGL::RenderFaces (CFaceListEntry* faceRenderList, int faceCount)
{
BeginRender ();
for (int nFace = faceCount - 1; nFace >= 0; nFace--)
	if (!faceRenderList [nFace].m_bTransparent)
 		DrawFaceTextured (faceRenderList [nFace]);
for (int nFace = 0; nFace < faceCount; nFace++)
	if (faceRenderList [nFace].m_bTransparent)
 		DrawFaceTextured (faceRenderList [nFace]);
EndRender ();
}

// -----------------------------------------------------------------------------

BOOL CRendererGL::InitProjection (GLvoid)
{
glShadeModel (GL_SMOOTH);
glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
glClearDepth (1.0f);
glEnable (GL_BLEND);
glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glEnable (GL_DEPTH_TEST);
glDepthFunc (GL_LESS);
glEnable (GL_ALPHA_TEST);
glAlphaFunc (GL_GEQUAL, 0.0025f);	
glEnable (GL_CULL_FACE);
glCullFace (GL_FRONT);
glEnable (GL_LINE_SMOOTH);
glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
glDrawBuffer (GL_BACK);
m_glInited = false;
return TRUE;
}

// -----------------------------------------------------------------------------

static double viewAngleTable [4] = {30.0, 40.0, 50.0, 60.0};

void CRendererGL::SetupProjection (bool bOrtho) 
{
if (m_bOrtho = bOrtho) {
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();
	glOrtho (0.0, ViewWidth (), 0.0, ViewHeight (), 0.0, 5000.0);
	glViewport (0, 0, ViewWidth (), ViewHeight ());
	ViewMatrix ()->GetProjection ();
	}
else {
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
#if 0
	gluPerspective (45.0f /** m_glAspectRatio*/, m_glAspectRatio, 1.0f, 5000.0f);
#else
#	if 0
	double h = 1.0f * tan (Radians (180.0 * m_glAspectRatio * 0.5));
	double w = h * m_glAspectRatio;
	glFrustum (w, -w, h, -h, 1.0, 5000.0);
#	else
	double a = Perspective () ? 30.0 : viewAngleTable [m_viewMatrix.DepthPerception ()];
	double h = 1.0f * tan (Radians (a));
	double w = h * m_glAspectRatio;
	glFrustum (-w, w, h, -h, 1.0, 50000.0);
#	endif
#endif
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glScalef (1.0f, -1.0f, -1.0f);
	ViewMatrix ()->GetProjection ();
	glEnable (GL_DEPTH_TEST);
	}
}

//------------------------------------------------------------------------------

void CRendererGL::SelectPen (int nPen)
{
	COLORREF color;

if (nPen > 0)
	color = PenColor (ePenColor (nPen - 1));
else
	return;
glColor3f (float (GetRValue (color)) / 255.0f, float (GetGValue (color)) / 255.0f, float (GetBValue (color)) / 255.0f);
glLineWidth (float (PenWidth (nPen)) * 1.5f);
}

//------------------------------------------------------------------------------

void CRendererGL::MoveTo (int x, int y)
{
m_x = x;
m_y = y;
}

//------------------------------------------------------------------------------

void CRendererGL::MoveTo (CVertex& v)
{
m_v = v;
}

//------------------------------------------------------------------------------

void CRendererGL::LineTo (int x, int y)
{
glBegin (GL_LINES);
glVertex2i (m_x, m_y);
glVertex2i (m_x = x, m_y = y);
glEnd ();
}

//------------------------------------------------------------------------------

void CRendererGL::LineTo (CVertex& v)
{
glBegin (GL_LINES);
glVertex3dv ((GLdouble*) &m_v.m_view.v);
m_v = v;
glVertex3dv ((GLdouble*) &m_v.m_view.v);
glEnd ();
}

//------------------------------------------------------------------------------

void CRendererGL::Rectangle (int left, int top, int right, int bottom)
{
glBegin (GL_LINE_LOOP);
glVertex2i (left, top);
glVertex2i (right, top);
glVertex2i (right, bottom);
glVertex2i (left, bottom);
glEnd ();
}

//------------------------------------------------------------------------------

void CRendererGL::PolyLine (CPoint* points, int nPoints)
{
glBegin (GL_LINE_STRIP);
for (; nPoints; nPoints--, points++)
	glVertex2i (points->x, points->y);
glEnd ();
}

//------------------------------------------------------------------------------

void CRendererGL::PolyLine (CVertex* points, int nPoints)
{
#if 0//def _DEBUG
MoveTo (*points);
for (--nPoints; nPoints; nPoints--)
	LineTo (*(--points));
#else
glBegin (GL_LINE_STRIP);
for (; nPoints; nPoints--, points++)
	glVertex3dv ((GLdouble*) &points->m_view.v);
glEnd ();
#endif
}

//------------------------------------------------------------------------------

void CRendererGL::Polygon (CPoint* points, int nPoints)
{
glBegin (GL_LINE_LOOP);
for (; nPoints; nPoints--, points++)
	glVertex2i (points->x, points->y);
glEnd ();
}

//------------------------------------------------------------------------------

void CRendererGL::Polygon (CVertex* vertices, int nVertices)
{
glBegin (GL_LINE_LOOP);
for (; nVertices; nVertices--, vertices++)
	glVertex3dv ((GLdouble*) &vertices->m_view.v);
glEnd ();
}

//------------------------------------------------------------------------------

void CRendererGL::Polygon (CVertex* vertices, int nVertices, ushort* index)
{
glBegin (GL_LINE_LOOP);
for (int i = 0; i < nVertices; i++)
	glVertex3dv ((GLdouble*) &(vertices [index ? index [i] : i].m_view.v));
glEnd ();
}

//------------------------------------------------------------------------------

void CRendererGL::TexturedPolygon (CTexture* texP, tTexCoord2d* texCoords, rgbColord* color, CVertex* vertices, int nVertices, ushort* index)
{
	static tTexCoord2d defaultTexCoords [4] = {{0.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}, {1.0, 0.0}};

if (!texP) {
	glDisable (GL_TEXTURE_2D);
	glColor3dv ((GLdouble*) color);
	}
else {
	texP->GLBind (GL_TEXTURE0, GL_MODULATE);
	glColor3f (1.0f, 1.0f, 1.0f);
	if (!texCoords)
		texCoords = defaultTexCoords;
	}
glBegin (GL_TRIANGLE_FAN); //(nVertices == 3) ? GL_TRIANGLES : (nVertices == 4) ? GL_QUADS : GL_POLYGON);
for (int i = 0; i < nVertices; i++) {
	if (texP)
		glTexCoord2dv ((GLdouble*) &texCoords [i]);
	glVertex3dv ((GLdouble*) &(vertices [index ? index [i] : i].m_view.v));
	}
glEnd ();
glDisable (GL_TEXTURE_2D);
}

//------------------------------------------------------------------------------

void CRendererGL::TexturedPolygon (short nTexture, tTexCoord2d* texCoords, rgbColord* color, CVertex* vertices, int nVertices, ushort* index)
{
TexturedPolygon (nTexture ? textureManager.Texture (nTexture) : null, texCoords, color, vertices, nVertices, index);
}

//------------------------------------------------------------------------------

void CRendererGL::Sprite (CTexture* texP, CVertex center, double width, double height, bool bAlways)
{
	static rgbColord color = {1.0, 1.0, 1.0};
	static ushort index [4] = {0, 1, 2, 3};

	CVertex vertices [4];
	float				du = float (texP->m_info.xOffset) / float (texP->Width ());
	float				dv = float (texP->m_info.yOffset) / float (texP->Height ());
	tTexCoord2d		texCoords [4] = {
		{du, dv}, 
		{du, 1.0f - dv}, 
		{1.0f - du, 1.0f - dv}, 
		{1.0f - du, dv}
	};

center.Transform (&m_viewMatrix);
vertices [0].m_view.v.x = center.m_view.v.x - width;
vertices [0].m_view.v.y = center.m_view.v.y - height;
vertices [1].m_view.v.x = center.m_view.v.x - width;
vertices [1].m_view.v.y = center.m_view.v.y + height;
vertices [2].m_view.v.x = center.m_view.v.x + width;
vertices [2].m_view.v.y = center.m_view.v.y + height;
vertices [3].m_view.v.x = center.m_view.v.x + width;
vertices [3].m_view.v.y = center.m_view.v.y - height;
vertices [0].m_view.v.z =
vertices [1].m_view.v.z =
vertices [2].m_view.v.z =
vertices [3].m_view.v.z = center.m_view.v.z;
if (bAlways)
	glDepthFunc (GL_ALWAYS);
TexturedPolygon (texP, texCoords, &color, vertices, 4, index);
if (bAlways)
	glDepthFunc (GL_LESS);
}

//------------------------------------------------------------------------------

void CRendererGL::Ellipse (CVertex& center, double xRad, double yRad)
{
	static tSinCosf sinCosTable [32];
	static bool bHaveSinCos = false;

if (!bHaveSinCos) {
	bHaveSinCos = true;
	ComputeSinCosTable (sinCosTable, sizeofa (sinCosTable));
	}

if (!m_bOrtho/* && Perspective ()*/) {
	CVertex points [33];
	double xRadd, yRadd;

	if ((xRad < 0) && (yRad < 0)) {
		xRadd = -xRad;
		yRadd = -yRad;
		}
	else {
		// rad is in screen coordinates, so find out what deltas to use for the view coordinates 
		// to achieve the proper radius when projecting the view coordinate
		CDoubleVector s [2], v [2];
		m_viewMatrix.Project (s [0], center.m_view);
		s [1] = s [0];
		s [0].v.x -= xRad;
		s [0].v.y -= yRad;
		s [1].v.x += xRad;
		s [1].v.y += yRad;
		m_viewMatrix.Unproject (v [0], s [0]);
		m_viewMatrix.Unproject (v [1], s [1]);
		if (xRad > 0)
			xRadd = fabs (v [1].v.x - v [0].v.x);
		if (yRad > 0)
			yRadd = fabs (v [1].v.y - v [0].v.y);
		}

	for (int i = 0; i < 32; i++) {
		points [i].m_view.v.x = center.m_view.v.x + sinCosTable [i].fCos * xRadd;
		points [i].m_view.v.y = center.m_view.v.y + sinCosTable [i].fSin * yRadd;
		points [i].m_view.v.z = center.m_view.v.z;
		}
	points [32] = points [0];
	glLineWidth (3.0);
	PolyLine (points, 33);
	glLineWidth (1.0);
	}
else {
	CPoint points [33];
	xRad = abs (xRad);
	yRad = abs (xRad);
	for (int i = 0; i < 32; i++) {
		points [i].x = center.m_screen.x + (long) Round (sinCosTable [i].fCos * xRad);
		points [i].y = center.m_screen.y + (long) Round (sinCosTable [i].fSin * yRad);
		}
	points [32] = points [0];
	PolyLine (points, 33);
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
