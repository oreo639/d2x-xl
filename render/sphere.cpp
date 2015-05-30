// -----------------------------------------------------------------------------

#include <math.h>

#ifdef HAVE_CONFIG_H
#	include <conf.h>
#endif

#include "descent.h"
#include "error.h"
#include "network.h"
#include "sphere.h"
#include "rendermine.h"
#include "maths.h"
#include "u_mem.h"
#include "glare.h"
#include "../effects/glow.h"
#include "ogl_lib.h"
#include "ogl_shader.h"
#include "objeffects.h"
#include "objrender.h"
#include "ogl_render.h"
#include "transprender.h"
#include "oof.h"
#include "addon_bitmaps.h"

#define SPHERE_ADDITIVE_BLENDING 1
#define SPHERE_MAX_RINGS			256
#if DBG
#	define SPHERE_DEFAULT_QUALITY 3
#	define SPHERE_DRAW_OUTLINE		1
#	define SPHERE_SW_TRANSFORM		0
#	define SPHERE_WIREFRAME			0
#	define SPHERE_DRAW_NORMALS		0
#else
#	define SPHERE_DEFAULT_QUALITY	3
#	define SPHERE_DRAW_OUTLINE		1
#	define SPHERE_SW_TRANSFORM		0
#	define SPHERE_WIREFRAME			0
#	define SPHERE_DRAW_NORMALS		0
#endif

// TODO: Create a c-tor for the two tables

float CSphereVertex::m_fNormRadScale = 1.0f; //(float) sqrt (2.0f) / 2.0f;

// -----------------------------------------------------------------------------

const char *pszSphereEffectFS [2] =
	{
	// hit effect
	"uniform sampler2D sphereTex;\r\n" \
	"uniform vec4 vHit [3];\r\n" \
	"uniform vec3 fRad;\r\n" \
	"varying vec3 vertex;\r\n" \
	"void main() {\r\n" \
	"vec3 scale;\r\n" \
	"scale.x = 1.0 - clamp (length (vertex - vec3 (vHit [0])) * fRad.x, 0.0, 1.0);\r\n" \
	"scale.y = 1.0 - clamp (length (vertex - vec3 (vHit [1])) * fRad.y, 0.0, 1.0);\r\n" \
	"scale.z = 1.0 - clamp (length (vertex - vec3 (vHit [2])) * fRad.z, 0.0, 1.0);\r\n" \
	"gl_FragColor = texture2D (sphereTex, gl_TexCoord [0].xy) * gl_Color * max (scale.x, max (scale.y, scale.z));\r\n" \
	"}"
	,
	// color effect
	"uniform sampler2D sphereTex;\r\n" \
	"uniform float refY;\r\n" \
	"uniform float colorScale;\r\n" \
	"uniform bool bMovingRing;\r\n" \
	"varying vec3 vertex;\r\n" \
	"varying vec3 center;\r\n" \
	"void main() {\r\n" \
	"//vec3 center = vec3 (gl_ModelViewMatrix * vec4 (0.0, 0.0, 0.0, 1.0));\r\n" \
	"vec3 s = normalize (vertex - center);\r\n" \
	"vec3 color = gl_Color.rgb * (1.0 - abs (dot (normalize (center), s)));\r\n" \
	"if (bMovingRing) {\r\n" \
	"	vec3 r = vec3 (s.x, refY, s.z);\r\n" \
	"	float fScale = min (1.0, length (s - r) * 4.0);\r\n" \
	"	if (fScale < 1.0) {\r\n" \
	"		float fBump = 1.0 / max (color.r, max (color.g, color.b));\r\n" \
	"		color *= fBump - (fBump - 1.0) * fScale;\r\n" \
	"		}\r\n" \
	"	}\r\n" \
	"vec4 texColor = texture2D (sphereTex, gl_TexCoord [0].xy);\r\n" \
	"gl_FragColor = vec4 (texColor.rgb * color * colorScale, texColor.a * gl_Color.a);\r\n" \
	"}"
	};

// -----------------------------------------------------------------------------

const char *pszSphereEffectVS [2] =
	{
	"varying vec3 vertex;\r\n" \
	"void main() {\r\n" \
	"	gl_TexCoord [0] = gl_MultiTexCoord0;\r\n" \
	"	gl_Position = ftransform();\r\n" \
   "	gl_FrontColor = gl_Color;\r\n" \
	"	vertex = vec3 (gl_Vertex);\r\n" \
	"	}"
	,
	"varying vec3 vertex;\r\n" \
	"varying vec3 center;\r\n" \
	"void main() {\r\n" \
	"	gl_TexCoord [0] = gl_MultiTexCoord0;\r\n" \
	"	gl_Position = ftransform();\r\n" \
   "	gl_FrontColor = gl_Color;\r\n" \
	"	vertex = vec3 (gl_ModelViewMatrix * gl_Vertex);\r\n" \
	"	center = vec3 (gl_ModelViewMatrix * vec4 (0.0, 0.0, 0.0, 1.0));\r\n" \
	"	}"
	}
	;

// -----------------------------------------------------------------------------

int32_t sphereEffectShaderProgs [2] = {-1, -1};

// -----------------------------------------------------------------------------

int32_t CreateSphereShader (int32_t nShader)
{
if (!(ogl.m_features.bShaders && ogl.m_features.bPerPixelLighting.Available ())) {
	gameStates.render.bPerPixelLighting = 0;
	return 0;
	}
if (sphereEffectShaderProgs [nShader] < 0) {
	PrintLog (1, "building sphere shader program for %s effect\n", nShader ? "color" : "hit");
	if (!shaderManager.Build (sphereEffectShaderProgs [nShader], pszSphereEffectFS [nShader], pszSphereEffectVS [nShader])) {
		ogl.m_features.bPerPixelLighting.Available (0);
		gameStates.render.bPerPixelLighting = 0;
		PrintLog (-1);
		return -1;
		}
	PrintLog (-1);
	}
return 1;
}

// -----------------------------------------------------------------------------

void ResetSphereShaders (void)
{
//sphereEffectShaderProg [0] = -1;
}

// -----------------------------------------------------------------------------

void UnloadSphereShader (void)
{
shaderManager.Deploy (-1);
}

// -----------------------------------------------------------------------------

int32_t WantEffect (CObject* pObj)
{
return (pObj->Type () == OBJ_PLAYER) || (pObj->Type () == OBJ_ROBOT);
}

// -----------------------------------------------------------------------------

int32_t NeedEffect (CObject* pObj)
{
	CObjHitInfo	hitInfo = pObj->HitInfo ();
	float fSize = 1.0f + 2.0f / X2F (pObj->Size ());

for (int32_t i = 0; i < 3; i++) {
	int32_t dt = gameStates.app.nSDLTicks [0] - int32_t (hitInfo.t [i]);
	if (dt < SHIELD_EFFECT_TIME) {
		float h = (fSize * float (cos (sqrt (float (dt) / float (SHIELD_EFFECT_TIME)) * PI / 2)));
		if (h > 1.0f / 1e6f)
			return 1;
		}
	}
return 0;
}

// -----------------------------------------------------------------------------

int32_t SetupHitEffectShader (CObject* pObj, float alpha)
{
	int32_t	nHits = 0;

PROF_START
if (CreateSphereShader (0) < 1) {
	PROF_END(ptShaderStates)
	return 0;
	}

	CObjHitInfo	hitInfo = pObj->HitInfo ();
	float fSize = 1.0f + 2.0f / X2F (pObj->Size ());
	float fScale [3];
	CFloatVector vHitf [3];

	tObjTransformation *pPos = OBJPOS (pObj);
	CFixMatrix m;
	CFixVector m_v;

if (!ogl.UseTransform ()) {
	fSize *= X2F (pObj->Size ());
	ogl.SetupTransform (0);
	m = CFixMatrix::IDENTITY;
	transformation.Begin (*PolyObjPos (pObj, &m_v), m, __FILE__, __LINE__); 
	}
else {
	m = pPos->mOrient;
	m.Transpose (m);
	m = m.Inverse ();
	}

for (int32_t i = 0; i < 3; i++) {
	int32_t dt = gameStates.app.nSDLTicks [0] - int32_t (hitInfo.t [i]);
	if (dt < SHIELD_EFFECT_TIME) {
		float h = (fSize * float (cos (sqrt (float (dt) / float (SHIELD_EFFECT_TIME)) * PI / 2)));
		if (h > 1.0f / 1e6f) {
			fScale [i] = 1.0f / h;
			vHitf [i].v.coord.w = 0.0f;
			if (ogl.UseTransform ()) {
				vHitf [i].Assign (m * hitInfo.v [i]);
				CFloatVector::Normalize (vHitf [i]);
				}
			else {
				vHitf [i].Assign (hitInfo.v [i]);
				transformation.Transform (vHitf [i], vHitf [i]);
				}
			nHits++;
			}
		else {
			fScale [i] = 1e6f;
			vHitf [i].SetZero ();
			}
		}
	else {
		fScale [i] = 1e6f;
		vHitf [i].SetZero ();
		}
	}

if (!ogl.UseTransform ()) {
	transformation.End (__FILE__, __LINE__);
	ogl.ResetTransform (1);
	}

if (!nHits)
	return 0;

GLhandleARB shaderProg = GLhandleARB (shaderManager.Deploy (sphereEffectShaderProgs [0]));
if (shaderProg) {
	if (shaderManager.Rebuild (shaderProg))
		/*nothing*/;
	glUniform1i (glGetUniformLocation (shaderProg, "sphereTex"), 0);
	//if (shaderProg) 
		{
		glUniform4fv (glGetUniformLocation (shaderProg, "vHit"), 3, reinterpret_cast<GLfloat*> (vHitf));
		glUniform3fv (glGetUniformLocation (shaderProg, "fRad"), 1, reinterpret_cast<GLfloat*> (fScale));
		}
	}
ogl.ClearError (0);
PROF_END(ptShaderStates)
return shaderManager.Current ();
}

// -----------------------------------------------------------------------------

int32_t SetupColorEffectShader (float fRefY, float fColorScale, int32_t bMovingRing)
{
PROF_START
if (CreateSphereShader (1) < 1) {
	PROF_END(ptShaderStates)
	return 0;
	}


GLhandleARB shaderProg = GLhandleARB (shaderManager.Deploy (sphereEffectShaderProgs [1]));
if (shaderProg) {
	if (shaderManager.Rebuild (shaderProg))
		/*nothing*/;
	glUniform1i (glGetUniformLocation (shaderProg, "sphereTex"), 0);
	//if (shaderProg) 
		{
#if 0
		COGLMatrix m;
		m.Getf (GL_MODELVIEW_MATRIX);
		glUniformMatrix4fv (glGetUniformLocation (shaderProg, "modelViewMatrix"), 1, GL_FALSE, m.Dataf ());
		glUniform3fv (glGetUniformLocation (shaderProg, "center"), 3, reinterpret_cast<GLfloat*> (vCenter));
		glUniform3fv (glGetUniformLocation (shaderProg, "viewDir"), 3, reinterpret_cast<GLfloat*> (vViewDir));
#endif
		glUniform1f (glGetUniformLocation (shaderProg, "refY"), GLfloat (fRefY));
		glUniform1f (glGetUniformLocation (shaderProg, "colorScale"), GLfloat (fColorScale));
		glUniform1i (glGetUniformLocation (shaderProg, "bMovingRing"), GLboolean (bMovingRing));
		}
	}
ogl.ClearError (0);
PROF_END(ptShaderStates)
return shaderManager.Current ();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CSphereData::Init (void)
{
m_pPulse = NULL;
m_pBitmap = NULL;
m_nFrame = 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CSphereVertex::Normalize (void)
{
m_v /= m_v.Mag () * m_fNormRadScale;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CSphereFace::Normalize (CFloatVector& v)
{
v /= v.Mag () * CSphereVertex::m_fNormRadScale;
}

// -----------------------------------------------------------------------------

void CSphereFace::ComputeNormal (void)
{
m_vNormal.m_v = -CFloatVector::Normal (Vertex (0), Vertex (1), Vertex (2));
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

CSphereVertex *CSphereTriangle::ComputeCenter (int32_t bNormalize)
{
m_vCenter = m_v [0];
m_vCenter += m_v [1];
m_vCenter += m_v [2];
m_vCenter *= 1.0f / 3.0f;
if (bNormalize)
	m_vCenter.Normalize ();
return &m_vCenter;
}

// -----------------------------------------------------------------------------

CSphereTriangle *CSphereTriangle::Split (CSphereTriangle *pDest)
{
	static int32_t o [4][3] = {{0, 3, 5}, {3, 4, 5}, {3, 1, 4}, {4, 2, 5}};

	int32_t			i, j;
	CSphereVertex	h [6];

for (i = 0; i < 3; i++)
	h [i] = m_v [i];
for (i = 0; i < 3; i++) 
	h [i + 3] = (h [i] + h [(i + 1) % 3]) * 0.5f;
for (i = 0; i < 6; i++)
	h [i].Normalize ();

for (i = 0; i < 4; i++, pDest++) {
	for (j = 0; j < 3; j++)
		pDest->m_v [j] = h [o [i][j]];
	}
return pDest;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

CSphereVertex *CSphereQuad::ComputeCenter (int32_t bNormalize)
{
m_vCenter = m_v [0];
m_vCenter += m_v [1];
m_vCenter += m_v [2];
m_vCenter += m_v [3];
m_vCenter *= 0.25f;
if (bNormalize)
	m_vCenter.Normalize ();
return &m_vCenter;
}

// -----------------------------------------------------------------------------

inline int32_t Wrap (int32_t i, int32_t l)
{
return (i < 0) ? i + l : i % l;
}

// -----------------------------------------------------------------------------

CSphereQuad *CSphereQuad::Split (CSphereQuad *pDest)
{
	static int32_t o [4][4] = {{0, 4, 8, 7}, {4, 1, 5, 8}, {8, 5, 2, 6}, {7, 8, 6, 3}};

	int32_t			i, j;
	CSphereVertex	h [9];

ComputeCenter ();
for (i = 0; i < 4; i++)
	h [i] = m_v [i];
for (i = 0; i < 4; i++)
	h [i + 4] = (m_v [i] + m_v [(i + 1) % 4]) * 0.5f;;
h [8] = m_vCenter;
for (i = 0; i < 9; i++)
	h [i].Normalize ();

for (i = 0; i < 4; i++, pDest++) {
	for (j = 0; j < 4; j++)
		pDest->m_v [j] = h [o [i][j]];
	}
return pDest;
}

// -----------------------------------------------------------------------------

CSphereTriangle *CSphereQuad::Split (CSphereTriangle *pDest)
{
	static int32_t o [4][3] = {{0, 1, 4}, {1, 2, 4}, {2, 3, 4}, {3, 0, 4}};

	int32_t			i, j;
	CSphereVertex	h [5];

for (i = 0; i < 4; i++) {
	h [i] = m_v [i];
	h [i].Normalize ();
	}
ComputeCenter ();
h [4] = m_vCenter;

for (i = 0; i < 4; i++, pDest++) {
	for (j = 0; j < 3; j++)
		pDest->m_v [j] = h [o [i][j]];
	}
return pDest;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

int16_t CSphere::FindVertex (CSphereVertex& v)
{
for (int16_t i = 0; i < m_nVertices; i++)
#if 1
	if (m_worldVerts [i] == v)
#else
	if (CFloatVector::Dist (m_worldVerts [i].m_v, v.m_v) < 1e-6f)
#endif
		return i;
return -1;
}

// -----------------------------------------------------------------------------

int16_t CSphere::AddVertex (CSphereVertex& v)
{
v.Normalize ();
int16_t i = FindVertex (v);
if (i < 0) {
	i = m_nVertices++;
	m_worldVerts [i] = v;
	}
return i;
}

// -----------------------------------------------------------------------------

CPulseData *CSphere::SetPulse (CPulseData *pPulse)
{
CPulseData *pOldPulse = GetPulse ();
m_pPulse = pPulse ? pPulse : &m_pulse;
return pOldPulse;
}

// -----------------------------------------------------------------------------

CPulseData *CSphere::SetupSurface (CPulseData *pPulse, CBitmap *pBitmap)
{
SetBitmap (pBitmap);
return SetPulse (pPulse);
}

// -----------------------------------------------------------------------------

void CSphere::SetupPulse (float fSpeed, float fMin)
{
m_pulse.Setup (fSpeed, fMin);
}

// -----------------------------------------------------------------------------

void CSphere::Pulsate (void)
{
if (m_pPulse)
	m_pPulse->Update ();
}

// -----------------------------------------------------------------------------

void CSphere::Animate (CBitmap* pBm)
{
#if 1
for (int32_t i = 0; i < 2; i++)
	if (shield [i].IsMe (pBm))
		shield [i].Animate (10);
#endif
}

// -----------------------------------------------------------------------------

int32_t CSphere::InitSurface (float red, float green, float blue, float alpha, float fScale)
{
	int32_t	bTextured = m_pBitmap != NULL;

fScale = /*m_pPulse ? m_pPulse->fScale :*/ 1.0f;
ogl.ResetClientStates (0);
if (m_pBitmap) {
	Animate (m_pBitmap);
	ogl.EnableClientStates (bTextured, 0, 0, GL_TEXTURE0);
	if (m_pBitmap->CurFrame ())
		m_pBitmap = m_pBitmap->CurFrame ();
	if (m_pBitmap->Bind (1))
		m_pBitmap = NULL;
	else {
		if (!bTextured)
			bTextured = -1;
		}
	}
if (!m_pBitmap) {
	ogl.SetTexturing (false);
	bTextured = 0;
	alpha /= 2;
	ogl.EnableClientStates (0, 0, 0, GL_TEXTURE0);
	}
if (alpha < 0)
	alpha = (float) (1.0f - gameStates.render.grAlpha / (float) FADE_LEVELS);
if (alpha < 1.0f) {
#if SPHERE_ADDITIVE_BLENDING
	fScale *= coronaIntensities [gameOpts->render.coronas.nObjIntensity];
#endif
#if 0
	if (m_pPulse && m_pPulse->fScale) 
#endif
		{
		red *= fScale;
		green *= fScale;
		blue *= fScale;
		}
	}
ogl.SetDepthWrite (false);
m_color.Set (red, green, blue, alpha);
return bTextured;
}

// -----------------------------------------------------------------------------

void CSphere::DrawFaces (int32_t nOffset, int32_t nFaces, int32_t nClientArrays, int32_t nPrimitive, int32_t nState)
{
int32_t nVertices = nFaces * FaceNodes ();
if (nState & 1) {
	ogl.EnableClientStates ((nClientArrays & 1) != 0, (nClientArrays & 2) != 0, 0, GL_TEXTURE0);
	if (((nClientArrays & 1) != 0) && !m_pBitmap->Bind (1))
		OglTexCoordPointer (2, GL_FLOAT, sizeof (CSphereVertex), reinterpret_cast<GLfloat*> (&m_worldVerts [nOffset * nVertices].m_tc));
	if ((nClientArrays & 2) != 0)
		OglColorPointer (4, GL_FLOAT, sizeof (CSphereVertex), reinterpret_cast<GLfloat*> (&m_worldVerts [nOffset * nVertices].m_c));
	else {
		if (m_pPulse)
			m_color *= m_pPulse->Scale ();
		glColor4fv ((GLfloat*) m_color.v.vec);
		}
	OglVertexPointer (3, GL_FLOAT, sizeof (CSphereVertex), reinterpret_cast<GLfloat*> (&m_worldVerts [nOffset * nVertices].m_v));
	}
OglDrawArrays (nPrimitive, 0, nVertices);
if (nState & 2) {
	ogl.DisableClientStates ((nClientArrays & 1) != 0, (nClientArrays & 2) != 0, 0, GL_TEXTURE0);
	OglCullFace (0);
	}
}

// -----------------------------------------------------------------------------

void CSphere::DrawFaces (CFloatVector *pVertex, tTexCoord2f *pTexCoord, int32_t nFaces, int32_t bTextured, int32_t nPrimitive, int32_t nState)
{
if (nState & 1) {
	ogl.EnableClientStates (bTextured, 0, 0, GL_TEXTURE0);
	if (bTextured && !m_pBitmap->Bind (1))
		OglTexCoordPointer (2, GL_FLOAT, 0, pTexCoord);
	OglVertexPointer (3, GL_FLOAT, sizeof (CFloatVector), pVertex);
	}
glColor4fv ((GLfloat*) m_color.v.vec);
OglDrawArrays (nPrimitive, 0, nFaces * FaceNodes ());
if (nState & 2) {
	ogl.DisableClientStates (bTextured, 0, 0, GL_TEXTURE0);
	OglCullFace (0);
	}
}

// -----------------------------------------------------------------------------

int32_t CSphere::Render (CObject *pObj, CFloatVector *pvPos, float xScale, float yScale, float zScale,
								 float red, float green, float blue, float alpha, int32_t nFaces, char bAdditive)
{
	float	fScale = 1.0f;
	int32_t	bTextured = 0;
	int32_t	bAppearing = pObj->Appearing ();
	int32_t	bEffect = 0;
	int32_t	bCartoonStyle = gameStates.render.EnableCartoonStyle ();

if (bAppearing) {
	float scale = pObj->AppearanceScale ();
	scale = Min (1.0f, (float) pow (1.0f - scale, 0.25f));
#if 1
	xScale *= scale;
	yScale *= scale;
	zScale *= scale;
#endif
	red *= scale;
	green *= scale;
	blue *= scale;
	}
else if (WantEffect (pObj)) {
	if (!NeedEffect (pObj))
		return 0;
	bEffect = 1;
	}

	int32_t	bGlow = /*(bAppearing || bEffect) &&*/ (bAdditive != 0) && glowRenderer.Available (GLOW_SHIELDS);

CFixVector vPos;
PolyObjPos (pObj, &vPos);
tObjTransformation *pPos = OBJPOS (pObj);

ogl.SetTransform (1);
transformation.Begin (vPos, pPos->mOrient, __FILE__, __LINE__);
glScalef (xScale, xScale, xScale);

#if SPHERE_DRAW_OUTLINE && (SPHERE_WIREFRAME < 2)
if (!bEffect/* && (gameOpts->render.textures.nQuality > 1)*/) {
	if (gameStates.render.CartoonStyle ()) {
#	if SPHERE_SW_TRANSFORM
#		if 0 //DBG
		transformation.End (__FILE__, __LINE__);
		ogl.ResetTransform (1);
		ogl.SetTransform (0);
		CFixMatrix m = CFixMatrix::IDENTITY;
		transformation.Begin (vPos, m, __FILE__, __LINE__);
#		endif
#	endif
		//gameStates.render.SetOutlineColor (0, 128, 255);
		RenderOutline (pObj, xScale);
		//gameStates.render.ResetOutlineColor ();
#	if SPHERE_SW_TRANSFORM
		ogl.SetTransform (1);
		transformation.Begin (vPos, pPos->mOrient, __FILE__, __LINE__);
#	endif
		}
#	if 0
	else if (alpha < 0.0f) {
		float h = 1.0f / Max (red, Max (green, blue)) * 255.0f;
		gameStates.render.SetOutlineColor (uint8_t (red * h), uint8_t (green * h), uint8_t (blue * h), 127);
#if 1
		RenderOutline (pObj, xScale);
#else
		if (gameOpts->render.effects.bGlow) {
			glowRenderer.End ();
			if (gameOpts->render.effects.bGlow <= 2) 
				glowRenderer.Begin (BLUR_OUTLINE);
			}
		RenderOutline (pObj, xScale);
		if (gameOpts->render.effects.bGlow)
			glowRenderer.End ();
#endif
		gameStates.render.ResetOutlineColor ();
		}
#	endif
	}
#endif

Pulsate ();
if (bGlow) {
	glowRenderer.Begin (GLOW_SHIELDS, 3, /*pObj->Type () == OBJ_POWERUP*/true, 1.0f);
	if (glowRenderer.SetViewport (GLOW_SHIELDS, CFixVector::ZERO, 4 * xScale / 3) < 0) { // not on screen
#if DBG
		tScreenPos s;
		ProjectPoint (vPos, s);
		transformation.Transform (vPos, vPos);
		ProjectPoint (vPos, s);
		ProjectPoint (pPos->vPos, s);
		transformation.Transform (vPos, pPos->vPos);
		ProjectPoint (vPos, s);
#endif
		glowRenderer.Done (GLOW_SHIELDS);
		ogl.SetDepthMode (GL_LEQUAL);
		transformation.End (__FILE__, __LINE__);
		return 0;
		}
#if 1
	red *= 1.0f / 3.0f;
	green *= 1.0f / 3.0f;
	blue *= 1.0f / 3.0f;
#endif
	ogl.SetBlendMode (OGL_BLEND_REPLACE);
	}
else {
	ogl.SetBlendMode (bAdditive > 0);
	}
ogl.SetDepthMode (GL_LEQUAL);

if (!bEffect)
	UnloadSphereShader ();
else if (gameOpts->render.bUseShaders && ogl.m_features.bShaders.Available ()) {
	if (!SetupHitEffectShader (pObj, fabs (alpha))) {
		transformation.End (__FILE__, __LINE__);
		return 0;
		}
	}

bTextured = InitSurface (red, green, blue, bEffect ? 1.0f : fabs (alpha), fScale);

#if 1
if (alpha < 0.0f)
	glBlendEquation (GL_MAX);
RenderFaces (xScale, nFaces, bTextured, bEffect, bGlow);
if (alpha < 0.0f)
	glBlendEquation (GL_FUNC_ADD);
if (bGlow) {
	if (bAdditive)
		glowRenderer.Done (GLOW_SHIELDS);
	else
		glowRenderer.End (fabs (alpha));
	}
#endif

transformation.End (__FILE__, __LINE__);
ogl.ResetTransform (0);
ogl.ResetClientStates (0);
gameStates.render.SetCartoonStyle (bCartoonStyle);
ogl.SetDepthWrite (true);
//ogl.SetDepthMode (GL_LEQUAL);
return 1;
}

// -----------------------------------------------------------------------------

void CSphere::Destroy (void)
{
m_worldVerts.Destroy ();
Init ();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CSphereEdge::Transform (float fScale)
{
for (int32_t i = 0; i < 2; i++) {
	transformation.Rotate (m_faces [i].m_vNormal [1], m_faces [i].m_vNormal [0]);
	CFloatVector v = m_faces [i].m_vCenter [0] * fScale;
	transformation.Transform (m_faces [i].m_vCenter [1], v);
#if SPHERE_SW_TRANSFORM
	v = m_vertices [i][0] * fScale;
	transformation.Transform (m_vertices [i][1], v);
#endif
	}
}

//------------------------------------------------------------------------------

int32_t CSphereEdge::Prepare (CFloatVector vViewer, int32_t nFilter, float fScale)
{
Transform (fScale);
#if 0
CFloatVector v = (m_vertices [0][1] + m_vertices [1][1]) * 0.5f;
if ((CFloatVector::Dot (v, m_faces [0].m_vNormal [1]) > 0.0f) == (CFloatVector::Dot (v, m_faces [1].m_vNormal [1]) > 0.0f))
	return 0;
#elif 1
if ((CFloatVector::Dot (m_faces [0].m_vCenter [1], m_faces [0].m_vNormal [1]) > 0.0f) == (CFloatVector::Dot (m_faces [1].m_vCenter [1], m_faces [1].m_vNormal [1]) > 0.0f))
	return 0;
#endif
gameData.segData.edgeVertexData [0].Add (m_vertices [0][SPHERE_SW_TRANSFORM]);
gameData.segData.edgeVertexData [0].Add (m_vertices [1][SPHERE_SW_TRANSFORM]);
return 1;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CTesselatedSphere::Transform (float fScale)
{
	CSphereVertex	*w = m_worldVerts.Buffer (),
						*v = m_viewVerts.Buffer ();

#if USE_OPENMP
if (gameStates.app.bMultiThreaded) {
#	pragma omp parallel
#	pragma omp for
	for (int32_t i = 0; i < m_nVertices; i++) {
		CFloatVector h = w [i].m_v * fScale;
		transformation.Transform (v [i].m_v, h);
		}
	}
else 
#endif
	{
	for (int32_t i = 0; i < m_nVertices; i++) {
		CFloatVector h = w [i].m_v * fScale;
		transformation.Transform (v [i].m_v, h);
		}
	}
}

// -----------------------------------------------------------------------------

int32_t CTesselatedSphere::Quality (void)
{
#if 0/*DBG && SPHERE_DEFAULT_QUALITY > -1*/
return SPHERE_DEFAULT_QUALITY;
#else
return m_nQuality ? m_nQuality : Max (SPHERE_MIN_QUALITY, gameOpts->render.textures.nQuality + 1);
#endif
}

// -----------------------------------------------------------------------------

void CTesselatedSphere::Destroy (void)
{
m_edges.Destroy ();
m_viewVerts.Destroy ();
CSphere::Destroy ();
}

// -----------------------------------------------------------------------------

int32_t CTesselatedSphere::FindEdge (CFloatVector& v1, CFloatVector& v2)
{
CSphereEdge e;
e.m_vertices [0][0] = v1;
e.m_vertices [1][0] = v2;
for (int32_t i = 0; i < m_nEdges; i++)
	if (m_edges [i] == e)
		return i;
return -1;
}

// -----------------------------------------------------------------------------

int32_t CTesselatedSphere::AddEdge (CFloatVector& v1, CFloatVector& v2, CFloatVector& v3)
{
#if USE_OPENMP
#pragma omp critical
{
#endif
#if SPHERE_DRAW_NORMALS
CSphereEdge *pEdge = m_edges + m_nEdges++;
pEdge->m_faces [0].m_vNormal [0] = -CFloatVector::Normal (v1, v2, v3);
#if DBG
if (pEdge->m_faces [0].m_vNormal [0].IsZero ())
	BRP;
#endif
pEdge->m_faces [0].m_vCenter [0] = (v1 + v2 + v3) / 3.0f;
pEdge->m_vertices [1][0] = pEdge->m_faces [0].m_vCenter [0];
pEdge->m_vertices [0][0] = pEdge->m_faces [0].m_vCenter [0] + pEdge->m_faces [0].m_vNormal [0];
pEdge->m_nFaces = 1;
#else
int32_t nFace, i = FindEdge (v1, v2);
if (i < 0) {
	i = m_nEdges++;
	nFace = 0;
	}
else
	nFace = 1;
CSphereEdge *pEdge = m_edges + i;
pEdge->m_nFaces = nFace + 1;
if (!nFace) {
	pEdge->m_vertices [0][0] = v1;
	pEdge->m_vertices [1][0] = v2;
	}
pEdge->m_faces [nFace].m_vNormal [0] = CFloatVector::Normal (v1, v2, v3);
pEdge->m_faces [nFace].m_vCenter [0] = (v1 + v2 + v3) / 3.0f;
if (CFloatVector::Dot (pEdge->m_faces [nFace].m_vNormal [0], v1) < 0.0f)
	pEdge->m_faces [nFace].m_vNormal [0].Neg ();
#endif
#if USE_OPENMP
}
#endif
return 1;
}

// -----------------------------------------------------------------------------

void CTesselatedSphere::RenderOutline (CObject *pObj, float fScale)
{
if (m_edges.Buffer ()) {
	gameData.segData.edgeVertexData [0].m_nVertices = 0;
	gameData.segData.edgeVertexData [1].m_nVertices = 0;
	for (int32_t i = 0; i < m_nEdges; i++)
		m_edges [i].Prepare (CFloatVector::ZERO, 2, fScale);
	if ((pObj->Type () == OBJ_POWERUP) && (pObj->Id () == POW_SHIELD_BOOST)) // draw thinner lines
		Swap (gameData.segData.edgeVertexData [0], gameData.segData.edgeVertexData [1]);
#if SPHERE_SW_TRANSFORM
	transformation.End (__FILE__, __LINE__);
#endif
	UnloadSphereShader ();
	int32_t bGlow = gameOpts->render.effects.bGlow;
	//gameOpts->render.effects.bGlow  = Min (gameOpts->render.effects.bGlow, 1);
	RenderMeshOutline (CMeshEdge::DistToScale (X2F (Max (0, CFixVector::Dist (pObj->Position (), gameData.objData.pViewer->Position ()) - pObj->Size ()))));
	gameOpts->render.effects.bGlow = bGlow;
	}
}

// -----------------------------------------------------------------------------
// Make the sphere glow stronger at its borders and create a glowing ring moving
// vertically over it.
// This could, and eventually will, be done more effectively and nicer in a shader program.

static inline float Sqr (float f) { return f * f; }

static inline float Sign (float v) { return (v < 0.0f) ? -1.0f : 1.0f; }

static inline float Wrap (float v, float l) { return (v < 0.0f) ? v + l : (v > l) ? v - l : v; }

#if 1
static inline float ColorBump (float f) { return f; }
#else
static inline float ColorBump (float f) { return pow (f * f, 1.0f / 3.0f); }
#endif

// -----------------------------------------------------------------------------

int32_t CTesselatedSphere::SetupColor (float fRadius, int32_t bGlow)
{
float fRefY = 1.0f - float (SDL_GetTicks () % 3001) / 750.0f;

CSphereVertex	*w = m_worldVerts.Buffer (),
					*v = m_viewVerts.Buffer ();

m_color *= 1.0f / Max (m_color.Red (), Max (m_color.Green (), m_color.Blue ()));

int32_t bMovingRing = gameOpts->render.textures.nQuality > 2;
float fColorScale = bGlow ? 1.0f / 4.0f : 1.0f;

#if DBG
static int32_t bUseColorEffectShader = 1;
if (bUseColorEffectShader)
#endif
if (SetupColorEffectShader (fRefY, fColorScale, bMovingRing))
	return 0;

Transform (fRadius);

CFloatVector vCenter, vViewDir;
vViewDir.SetZero ();
transformation.Transform (vCenter, vViewDir);
vViewDir = vCenter;
CFloatVector::Normalize (vViewDir);

if (m_pPulse && m_pPulse->Valid ())
	m_color *= m_pPulse->Scale ();

#if USE_OPENMP
if (gameStates.app.bMultiThreaded) {
#	pragma omp parallel
#	pragma omp for
	for (int32_t i = 0; i < m_nVertices; i++) {
		CFloatVector s = v [i].m_v - vCenter;
		CFloatVector::Normalize (s);
		CFloatVector color = m_color;
		color *= ColorBump (1.0f - fabs (CFloatVector::Dot (vViewDir, s))); 
		if (bMovingRing) {
		// create a ring moving down the sphere
			CFloatVector r;
			r.Set (s.X (), fRefY, s.Z (), 1.0f);
			float fScale = Min (1.0f, CFloatVector::Dist (s, r) * 4.0f);
			if (fScale < 1.0f) {
				float fBump = 1.0f / Max (color.Red (), Max (color.Green (), color.Blue ()));
				color *= fBump - (fBump - 1.0f) * fScale; 
				}
			}
		color *= fColorScale;
		w [i].m_c = color;
		}
	}
else 
#endif
	{
	for (int32_t i = 0; i < m_nVertices; i++) {
		CFloatVector s = v [i].m_v - vCenter;
		CFloatVector::Normalize (s);
		CFloatVector color = m_color;
		color *= ColorBump (1.0f - fabs (CFloatVector::Dot (vViewDir, s))); 
		if (bMovingRing) {
			// create a ring moving down the sphere
			CFloatVector r;
			r.Set (s.X (), fRefY, s.Z (), 1.0f);
			float fScale = Min (1.0f, CFloatVector::Dist (s, r) * 4.0f);
			if (fScale < 1.0f) {
				float fBump = 1.0f / Max (color.Red (), Max (color.Green (), color.Blue ()));
				color *= fBump - (fBump - 1.0f) * fScale; 
				}
			}
		color *= fColorScale;
		w [i].m_c = color;
		}
	}
return 1;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define UV_SCALE	3.0f

int32_t CRingedSphere::CreateVertices (int32_t nRings, int32_t nFaces)
{
	int32_t			h, i, j;
	float				t1, t2, t3, a, sint1, cost1, sint2, cost2, sint3, cost3;
	CSphereVertex	*pVertex;

if (nRings > SPHERE_MAX_RINGS)
	nRings = SPHERE_MAX_RINGS;
if ((m_nRings == nRings) && (m_nFaces == nFaces) && (m_worldVerts.Buffer () != NULL))
	return 1;

m_nRings =
m_nFaces = 
m_nVertices = 0;
m_worldVerts.Destroy ();
h = nRings * (nRings + 1);
if (!m_worldVerts.Create (h))
	return 0;
m_nRings = nRings;
m_nFaces = nFaces;
m_nVertices = h;
h = nRings / 2;
a = float (2 * PI / nRings);
pVertex = m_worldVerts.Buffer ();
for (j = 0; j < h; j++) {
	t1 = float (j * a - PI / 2);
	t2 = t1 + a;
	sint1 = float (sin (t1));
	cost1 = float (cos (t1));
	sint2 = float (sin (t2));
	cost2 = float (cos (t2));
	for (i = 0; i <= nRings; i++) {
		t3 = i * a;
		sint3 = float (sin (t3));
		cost3 = float (cos (t3));
		pVertex->m_v.v.coord.x = cost2 * cost3;
		pVertex->m_v.v.coord.y = sint2;
		pVertex->m_v.v.coord.z = cost2 * sint3;
		pVertex->m_tc.v.u =(1.0f - float (i) / nRings) * nFaces * UV_SCALE;
		pVertex->m_tc.v.v = (float (2 * j + 2) / nRings) * nFaces * UV_SCALE;
		pVertex++;
		pVertex->m_v.v.coord.x = cost1 * cost3;
		pVertex->m_v.v.coord.y = sint1;
		pVertex->m_v.v.coord.z = cost1 * sint3;
		pVertex->m_tc.v.u = (1.0f - float (i) / nRings) * nFaces * UV_SCALE;
		pVertex->m_tc.v.v = (float (2 * j) / nRings) * nFaces * UV_SCALE;
		pVertex++;
		}
	}
return 1;
}

// -----------------------------------------------------------------------------

int32_t CRingedSphere::Create (void)
{
return CreateVertices ();
}

// -----------------------------------------------------------------------------

void CRingedSphere::RenderFaces (float fRadius, int32_t nFaces, int32_t bTextured, int32_t bEffect, int32_t bGlow)
{
	int32_t			nCull, h, i, j, nQuads, nRings = Quality ();
	CFloatVector	p [2 * SPHERE_MAX_RINGS + 2];
	tTexCoord2f		tc [2 * SPHERE_MAX_RINGS + 2];
	CSphereVertex*	svP [2];

if (nRings > SPHERE_MAX_RINGS)
	nRings = SPHERE_MAX_RINGS;
if (!CreateVertices (nRings, nFaces))
	return;
h = nRings / 2;
nQuads = 2 * nRings + 2;

ogl.EnableClientStates (bTextured, 0, 0, GL_TEXTURE0);
if (ogl.UseTransform ()) {
	glScalef (fRadius, fRadius, fRadius);
	for (nCull = 0; nCull < 2; nCull++) {
		svP [0] = svP [1] = m_worldVerts.Buffer ();
		ogl.SetCullMode (nCull ? GL_FRONT : GL_BACK);
		for (i = 0; i < h; i++) {
			DrawFaces (i, nQuads, bTextured, GL_QUAD_STRIP, nCull ? 2 : 1);
#if 0
			if (!bTextured) {
				for (i = 0; i < nQuads; i++, svP [1]++) {
					p [i] = svP [1]->m_v;
					if (bTextured) {
						tc [i].dir.u = svP [1]->m_tc.dir.u * nFaces;
						tc [i].dir.dir = svP [1]->m_tc.dir.dir * nFaces;
						}
					}
				glLineWidth (2);
				RenderSphereRing (p, tc, nQuads, 0, GL_LINE_STRIP);
				glLineWidth (1);
				}
#endif
			}
		}
	}
else {
	for (nCull = 0; nCull < 2; nCull++) {
		ogl.SetCullMode (nCull ? GL_FRONT : GL_BACK);
		svP [0] = svP [1] = m_worldVerts.Buffer ();
		for (j = 0; j < h; j++) {
			for (i = 0; i < nQuads; i++, svP [0]++) {
				p [i] = svP [0]->m_v * fRadius;
				transformation.Transform (p [i], p [i], 0);
				if (bTextured)
					tc [i] = svP [0]->m_tc;
				}
			DrawFaces (p, tc, nQuads, bTextured, GL_QUAD_STRIP, 3);
#if 0
			if (!bTextured) {
				for (i = 0; i < nQuads; i++, svP [1]++) {
					p [i] = svP [1]->m_v;
					VmVecScale (p + i, p + i, fRadius);
					transformation.Transform (p + i, p + i, 0);
					if (bTextured) {
						tc [i].dir.u = svP [1]->m_tc.dir.u * nFaces;
						tc [i].dir.dir = svP [1]->m_tc.dir.dir * nFaces;
						}
					}
				glLineWidth (2);
				RenderSphereRing (p, tc, nQuads, 0, GL_LINE_STRIP);
				glLineWidth (1);
				}
#endif
			}
		}
	}
ogl.DisableClientStates (bTextured, 0, 0, GL_TEXTURE0);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

int32_t CTriangleSphere::Tesselate (CSphereTriangle *pSrc, CSphereTriangle *pDest, int32_t nFaces)
{
for (int32_t i = 0; i < nFaces; i++)
		pSrc [i].Split (pDest + 3 * i);
return 1;
}

// -----------------------------------------------------------------------------

void CTriangleSphere::SetupFaces (void)
{
static float baseOctagon [8][3][3] = {
	{{-1,0,1},{1,0,1},{0,1,0}},
	{{1,0,1},{1,0,-1},{0,1,0}},
	{{1,0,-1},{-1,0,-1},{0,1,0}},
	{{-1,0,-1},{-1,0,1},{0,1,0}},
	{{1,0,1},{-1,0,1},{0,-1,0}},
	{{1,0,-1},{1,0,1},{0,-1,0}},
	{{-1,0,-1},{1,0,-1},{0,-1,0}},
	{{-1,0,1},{-1,0,-1},{0,-1,0}}
	};

static tTexCoord2f baseTC [2][3] = {{{0,0}, {1,0}, {0.5f,0.5f}}, {{0.5f,0.5f}, {1,1}, {0,1}}};

for (int32_t i = 0; i < 8; i++) {
	for (int32_t j = 0; j < 3; j++) {
		CSphereVertex v;
		for (int32_t k = 0; k < 3; k++) 
			v.m_v.v.vec [k] = baseOctagon [i][j][k];
		v.m_tc = baseTC [i & 1][j];
		m_faces [0][i].m_v [j] = v;
		}
	}
}

// -----------------------------------------------------------------------------

int32_t CTriangleSphere::CreateFaces (void)
{
SetupFaces ();
int32_t i, j, nFaces = 8, q = Quality ();
for (i = 0, j = 1; i < q; i++, nFaces *= 4, j = !j) {
	Tesselate (m_faces [j].Buffer (), m_faces [!j].Buffer (), nFaces);
	}
m_nFaces = nFaces;
return !j;
}

// -----------------------------------------------------------------------------

int32_t CTriangleSphere::CreateBuffers (void)
{
m_nFaces = 8 * int32_t (pow (4.0f, float (Quality ())));
m_nVertices = m_nFaces * 3;

if (m_faces [0].Create (m_nFaces) && m_faces [1].Create (m_nFaces) && m_worldVerts.Create (m_nVertices) && m_viewVerts.Create (m_nVertices)) 
	return 1;
m_viewVerts.Destroy ();
m_worldVerts.Destroy ();
m_faces [1].Destroy ();
m_faces [0].Destroy ();
PrintLog (-1);
return 0;
}

// -----------------------------------------------------------------------------

int32_t CTriangleSphere::CreateEdgeList (void)
{
m_nEdges = m_nFaces * 2;
if (!gameData.segData.CreateEdgeBuffers (m_nEdges))
	return -1;
if (!m_edges.Create (m_nEdges))
	return -1;

m_nEdges = 0;

for (int32_t i = 0; i < m_nFaces; i++) {
	CSphereFace *pFace = Face (i);
	CSphereVertex *pVertex = pFace->Vertices ();
#if SPHERE_DRAW_NORMALS
	pFace->ComputeCenter (0);
	AddEdge (pVertex [0].m_v, pVertex [1].m_v, pFace->Center ());
#else
	for (int32_t j = 0; j < 3; j++)
		AddEdge (pVertex [j].m_v, pVertex [(j + 1) % 3].m_v, pFace->Center ());
#endif
	}
return m_nEdges;
}

// -----------------------------------------------------------------------------

int32_t CTriangleSphere::Create (void)
{
if (!CreateBuffers ())
	return 0;

SetQuality (Quality ());

m_nFaceBuffer = CreateFaces ();
CreateEdgeList ();

CSphereTriangle *pFace = m_faces [m_nFaceBuffer].Buffer ();
CSphereVertex *pVertex = m_worldVerts.Buffer ();

for (int32_t i = 0; i < m_nFaces; i++, pFace++) {
	for (int32_t j = 0; j < 3; j++)
		*(pVertex++) = pFace->m_v [j];
	}
m_faces [0].Destroy ();
m_faces [1].Destroy ();
return m_nFaces;
}

// -----------------------------------------------------------------------------

void CTriangleSphere::RenderFaces (float fRadius, int32_t nFaces, int32_t bTextured, int32_t bEffect, int32_t bGlow)
{
if (!m_worldVerts.Buffer () || !m_viewVerts.Buffer ())
	return;
#if SPHERE_WIREFRAME
glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
glLineWidth (3);
#endif
if (bEffect) {
	for (int32_t nCull = 0; nCull < 2; nCull++) {
		ogl.SetCullMode (nCull ? GL_FRONT : GL_BACK);
		DrawFaces (0, m_nFaces, bTextured, GL_TRIANGLES, nCull ? 2 : 1);
		}
	}
else {
	ogl.SetCullMode (GL_FRONT);
	int32_t bColored = SetupColor (fRadius, bGlow);
	DrawFaces (0, m_nFaces, bTextured | (bColored << 1), GL_TRIANGLES, 3);
	if (bColored)
		UnloadSphereShader ();
	}
#if SPHERE_WIREFRAME
glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
#endif
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#if DBG
int32_t nDestBuffer = 1;
#endif

int32_t CQuadSphere::Tesselate (CSphereQuad *pDest, CSphereQuad *pSrc, int32_t nFaces)
{
for (int32_t i = 0; i < nFaces; i++)
		pSrc [i].Split (pDest + i * 4);
return 1;
}

// -----------------------------------------------------------------------------

int32_t CQuadSphere::Tesselate (CSphereTriangle *pDest, CSphereQuad *pSrc, int32_t nFaces)
{
for (int32_t i = 0; i < nFaces; i++)
		pSrc [i].Split (pDest + i * 4);
return 1;
}

// -----------------------------------------------------------------------------

void CQuadSphere::SetupFaces (void)
{
static float baseCube [6][4][3] = {
	{{-1,-1,-1},{+1,-1,-1},{+1,+1,-1},{-1,+1,-1}},
	{{+1,-1,-1},{+1,-1,+1},{+1,+1,+1},{+1,+1,-1}},
	{{+1,-1,+1},{-1,-1,+1},{-1,+1,+1},{+1,+1,+1}},
	{{-1,-1,+1},{-1,-1,-1},{-1,+1,-1},{-1,+1,+1}},
	{{-1,+1,-1},{+1,+1,-1},{+1,+1,+1},{-1,+1,+1}},
	{{+1,-1,-1},{-1,-1,-1},{-1,-1,+1},{+1,-1,+1}}
	};

static tTexCoord2f baseTC [4] = {{0,0}, {1,0}, {1,1}, {0,1}};

for (int32_t i = 0; i < 6; i++) {
	for (int32_t j = 0; j < 4; j++) {
		CSphereVertex v;
		for (int32_t k = 0; k < 3; k++)
			v.m_v.v.vec [k] = baseCube [i][j][k];
		v.m_tc = baseTC [j];
		m_quads [0][i].m_v [j] = v;
		}
	}
}

// -----------------------------------------------------------------------------

int32_t CQuadSphere::CreateFaces (void)
{
SetupFaces ();
int32_t i, j, nFaces = 6, q = Quality ();
for (i = 0, j = 1; i < q - 1; i++, nFaces *= 4, j = !j) {
#if DBG
	nDestBuffer = j;
#endif
	Tesselate (m_quads [j].Buffer (), m_quads [!j].Buffer (), nFaces);
	}
Tesselate (m_faces.Buffer (), m_quads [!j].Buffer (), nFaces);
return j;
}

// -----------------------------------------------------------------------------

int32_t CQuadSphere::CreateBuffers (void)
{
m_nFaces = 6 * int32_t (pow (4.0f, float (Quality ())));
m_nVertices = m_nFaces * 3;

if (m_faces.Create (m_nFaces) && m_quads [0].Create (m_nFaces) && m_quads [1].Create (m_nFaces) && m_worldVerts.Create (m_nVertices) && m_viewVerts.Create (m_nVertices)) 
	return 1;
m_viewVerts.Destroy ();
m_worldVerts.Destroy ();
m_quads [1].Destroy ();
m_quads [0].Destroy ();
m_faces.Destroy ();
PrintLog (-1);
return 0;
}

// -----------------------------------------------------------------------------

#define SPLIT_TRIANGLES	1

#if SPLIT_TRIANGLES

int32_t CQuadSphere::CreateEdgeList (void)
{
m_nEdges = m_nFaces * 2;
if (!gameData.segData.CreateEdgeBuffers (m_nEdges))
	return -1;
if (!m_edges.Create (m_nEdges))
	return -1;

m_nEdges = 0;

for (int32_t i = 0; i < m_nFaces; i++) {
	CSphereFace *pFace = Face (i);
	CSphereVertex *pVertex = pFace->Vertices ();
	pFace->ComputeCenter (0);
	for (int32_t j = 0; j < 3; j++)
		AddEdge (pVertex [j].m_v, pVertex [(j + 1) % 3].m_v, pFace->Center ());
	}
return m_nEdges;
}

#else

int32_t CQuadSphere::CreateEdgeList (void)
{
m_nEdges = m_nFaces * 6;
if (!gameData.segData.CreateEdgeBuffers (m_nEdges))
	return -1;
if (!m_edges.Create (m_nEdges))
	return -1;

m_nEdges = 0;

	static int32_t o [4][5] = {{0,1,4,0,1},{1,2,4,1,2},{2,3,4,2,3},{3,0,4,3,0}};
	CFloatVector	v [5];

for (int32_t i = 0; i < m_nFaces; i++) {
	CSphereFace *pFace = Face (i);
	CSphereVertex *pVertex = pFace->Vertices ();
	for (int32_t j = 0; j < 4; j++)
		v [j] = pVertex [j].m_v;
	pFace->ComputeCenter (0);
	v [4] = pFace->m_vCenter.m_v;
	for (int32_t h = 0; h < 4; h++) {
#if SPHERE_DRAW_NORMALS
		AddEdge (h [o [h][0]].m_v, h [o [h][1]].m_v, h [o [h][2]].m_v);
#else
		for (int32_t j = 0; j < 3; j++)
			AddEdge (v [o [h][j]], v [o [h][j + 1]], v [o [h][j + 2]]);
#endif
		}
	}
return m_nEdges;
}

#endif

// -----------------------------------------------------------------------------

int32_t CQuadSphere::Create (void)
{
if (!CreateBuffers ())
	return 0;

SetQuality (Quality ());

m_nFaceBuffer = CreateFaces ();
CreateEdgeList ();

CSphereTriangle *pFace = m_faces.Buffer ();
CSphereVertex *pVertex = m_worldVerts.Buffer ();

for (int32_t i = 0; i < m_nFaces; i++, pFace++) {
	for (int32_t j = 0; j < 3; j++)
		*(pVertex++) = pFace->m_v [j];
	}

m_quads [0].Destroy ();
m_quads [1].Destroy ();
return m_nFaces;
}

// -----------------------------------------------------------------------------

void CQuadSphere::RenderFaces (float fRadius, int32_t nFaces, int32_t bTextured, int32_t bEffect, int32_t bGlow)
{
if (!m_worldVerts.Buffer () || !m_viewVerts.Buffer ())
	return;

#if SPHERE_WIREFRAME
glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
glLineWidth (3);
#endif
if (bEffect) {
	for (int32_t nCull = 0; nCull < 2; nCull++) {
		ogl.SetCullMode (nCull ? GL_FRONT : GL_BACK);
		DrawFaces (0, m_nFaces, bTextured, GL_TRIANGLES, nCull ? 2 : 1);
		}
	}
else {
	ogl.SetCullMode (GL_FRONT);
	int32_t bColored = SetupColor (fRadius, bGlow);
	DrawFaces (0, m_nFaces, bTextured | (bColored << 1), GL_TRIANGLES, 3);
	if (bColored)
		UnloadSphereShader ();
	}
#if SPHERE_WIREFRAME
glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
#endif
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CPulseData::Setup (float fSpeed, float fMin)
{
m_fScale =
m_fMin = fMin;
m_fSpeed =
m_fDir = fSpeed;
m_bValid = 1;
}

// -----------------------------------------------------------------------------

void CPulseData::Update (void)
{
static CTimeout to (25);

if (m_bValid && to.Expired ()) {
	m_fScale += m_fDir;
	if (m_fScale > 1.0f) {
		m_fScale = 1.0f;
		m_fDir = -m_fSpeed;
		}
	else if (m_fScale < m_fMin) {
		m_fScale = m_fMin;
		m_fDir = m_fSpeed;
		}
	}
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

CSphere *CreateSphere (void)
{
#if SPHERE_TYPE == SPHERE_TYPE_RINGS
return new CRingedSphere ();
#elif SPHERE_TYPE == SPHERE_TYPE_TRIANGLES
return new CTriangleSphere ();
#elif SPHERE_TYPE == SPHERE_TYPE_QUADS
return new CQuadSphere ();
#endif
}

// -----------------------------------------------------------------------------

int32_t CreateShieldSphere (void)
{
if (!shield [0].Load () || !shield [1].Load () || !shield [2].Load ())
	/*nothing*/; //return 0;
if (gameData.render.shield) {
	if (gameData.render.shield->HasQuality (gameOpts->render.textures.nQuality + 1))
		return 1;
	delete gameData.render.shield;
	}
if (!(gameData.render.shield = CreateSphere ()))
	return 0;
gameData.render.shield->Create ();
gameData.render.shield->SetupPulse (0.02f, 0.5f);
gameData.render.shield->SetPulse (NULL);
return 1;
}

// -----------------------------------------------------------------------------

int32_t DrawShieldSphere (CObject *pObj, float red, float green, float blue, float alpha, char bAdditive, fix nSize)
{
if (!CreateShieldSphere ())
	return 0;
#if !RINGED_SPHERE
if (gameData.render.shield->m_nFaces > 0)
#endif
 {
	if (!nSize) {
#if 0 //DBG
		nSize = pObj->info.xSize;
#else
		if (pObj->rType.polyObjInfo.nModel < 0) 
			nSize = pObj->info.xSize;
		else {
			CPolyModel* pModel = GetPolyModel (pObj, NULL, pObj->ModelId (), 0);
			nSize = pModel ? pModel->Rad () : pObj->info.xSize;
			}
#endif
		}
	float r = X2F (nSize);
	if (gameStates.render.nType == RENDER_TYPE_TRANSPARENCY)
		gameData.render.shield->Render (pObj, NULL, r, r, r, red, green, blue, alpha, 1, bAdditive);
	else
		transparencyRenderer.AddSphere (riSphereShield, red, green, blue, alpha, pObj, bAdditive, nSize);
	}
return 1;
}

// -----------------------------------------------------------------------------

void DrawMonsterball (CObject *pObj, float red, float green, float blue, float alpha)
{
#if !RINGED_SPHERE
if (!gameData.render.monsterball) {
	gameData.render.monsterball = CreateSphere ();
	if (!gameData.render.monsterball)
		return;
	gameData.render.monsterball->SetQuality (3);
	gameData.render.monsterball->Create ();
	}
if (gameData.render.monsterball->m_nFaces > 0)
#endif
	{
	if (gameStates.render.nType != RENDER_TYPE_TRANSPARENCY)
		transparencyRenderer.AddSphere (riMonsterball, red, green, blue, alpha, pObj, 0);
	else {
		float r = X2F (pObj->info.xSize);
		CFloatVector p;
		p.SetZero ();
		gameData.render.monsterball->Render (pObj, &p, r, r, r, red, green, blue, gameData.hoard.monsterball.bm.Buffer () ? 1.0f : alpha, 4, 0);
		ogl.ResetTransform (1);
		ogl.SetTransform (0);
		}
	}
}

// -----------------------------------------------------------------------------

void DestroyShieldSphere (void)
{
if (gameData.render.shield)
	gameData.render.shield->Destroy ();
}

// -----------------------------------------------------------------------------

void DestroyMonsterball (void)
{
if (gameData.render.monsterball)
	gameData.render.monsterball->Destroy ();
}

// -----------------------------------------------------------------------------

void InitSpheres (void)
{
PrintLog (1, "creating spheres\n");
CreateShieldSphere ();
PrintLog (-1);
}

// -----------------------------------------------------------------------------
