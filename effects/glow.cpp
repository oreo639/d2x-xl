
#include "descent.h"
#include "error.h"
#include "ogl_lib.h"
#include "ogl_shader.h"
#include "automap.h"
#include "glow.h"
#include "cockpit.h"

CGlowRenderer glowRenderer;

#define USE_VIEWPORT 1
#define BLUR 2
#define START_RAD (m_bViewport ? 2.0f : 0.0f)
#define RAD_INCR (m_bViewport ? 2.0f : 2.0f)

//------------------------------------------------------------------------------

#if 1

int hBlurShader = -1;

#	if 1

// linear sampling
const char *blurFS = 
	"uniform sampler2D glowSource;\r\n" \
	"uniform float direction;\r\n" \
	"uniform float scale; // render target width/height\r\n" \
	"uniform float brightness; // render target width/height\r\n" \
	"vec2 offset = vec2 (1.3846153846, 3.2307692308);\r\n" \
	"vec3 weight = vec3 (0.2270270270, 0.3162162162, 0.0702702703);\r\n" \
	"void main() {\r\n" \
	"float xScale = (1.0 - direction) * scale, yScale = direction * scale;\r\n" \
	"vec2 uv = gl_TexCoord [0].xy;\r\n" \
	"vec3 tc = texture2D (glowSource, uv).rgb * weight [0];\r\n" \
	"vec2 v = vec2 (offset [0] * xScale, offset [0] * yScale);\r\n" \
	"tc += texture2D (glowSource, uv + v).rgb * weight [1];\r\n" \
	"tc += texture2D (glowSource, uv - v).rgb * weight [1];\r\n" \
	"v = vec2 (offset [1] * xScale, offset [1] * yScale);\r\n" \
	"tc += texture2D (glowSource, uv + v).rgb * weight [2];\r\n" \
	"tc += texture2D (glowSource, uv - v).rgb * weight [2];\r\n" \
	"gl_FragColor = vec4 (tc, 1.0) * brightness;\r\n" \
	"}\r\n";

#	else

// discrete sampling
const char *blurFS = 
	"uniform sampler2D glowSource;\r\n" \
	"uniform float direction;\r\n" \
	"uniform float scale; // render target width/height\r\n" \
	"uniform float brightness; // render target width/height\r\n" \
	"void main() {\r\n" \
	"float xScale = (1.0 - direction) * scale, yScale = direction * scale;\r\n" \
	"vec2 uv = gl_TexCoord [0].xy;\r\n" \
	"vec3 tc = texture2D (glowSource, uv).rgb * 0.2270270270;\r\n" \
	"vec2 v = vec2 (xScale, yScale);\r\n" \
	"tc += texture2D (glowSource, uv + v).rgb * 0.1945945946;\r\n" \
	"tc += texture2D (glowSource, uv - v).rgb * 0.1945945946;\r\n" \
	"v = vec2 (2.0 * xScale, 2.0 * yScale);\r\n" \
	"tc += texture2D (glowSource, uv + v).rgb * 0.1216216216;\r\n" \
	"tc += texture2D (glowSource, uv - v).rgb * 0.1216216216;\r\n" \
	"v = vec2 (3.0 * xScale, 3.0 * yScale);\r\n" \
	"tc += texture2D (glowSource, uv + v).rgb * 0.0540540541;\r\n" \
	"tc += texture2D (glowSource, uv - v).rgb * 0.0540540541;\r\n" \
	"v = vec2 (4.0 * xScale, 4.0 * yScale);\r\n" \
	"tc += texture2D (glowSource, uv + v).rgb * 0.0162162162;\r\n" \
	"tc += texture2D (glowSource, uv - v).rgb * 0.0162162162;\r\n" \
	"gl_FragColor = vec4 (tc, 1.0) * brightness;\r\n" \
	"}\r\n";

#	endif
	
#else

int hBlurShader [2] = {-1, -1};

const char *blurFS [2] = { 
	"uniform sampler2D glowSource;\r\n" \
	"uniform float scale; // render target width/height\r\n" \
	"uniform float brightness; // render target width/height\r\n" \
	"float offset[5] = float[](0.0, 1.0, 2.0, 3.0, 4.0);\r\n" \
	"float weight[5] = float[](0.18, 0.15, 0.12, 0.09, 0.05);\r\n" \
	"void main() {\r\n" \
	"vec2 uv = gl_TexCoord[0].xy;\r\n" \
	"vec3 tc = texture2D(glowSource, uv).rgb * weight[0];\r\n" \
	"vec2 v = vec2 (0.0, offset[1]*scale);\r\n" \
	"tc += texture2D(glowSource, uv + v).rgb * weight[1];\r\n" \
	"tc += texture2D(glowSource, uv - v).rgb * weight[1];\r\n" \
	"v = vec2 (0.0, offset[2]*scale);\r\n" \
	"tc += texture2D(glowSource, uv + v).rgb * weight[2];\r\n" \
	"tc += texture2D(glowSource, uv - v).rgb * weight[2];\r\n" \
	"v = vec2 (0.0, offset[3]*scale);\r\n" \
	"tc += texture2D(glowSource, uv + v).rgb * weight[3];\r\n" \
	"tc += texture2D(glowSource, uv - v).rgb * weight[3];\r\n" \
	"v = vec2 (0.0, offset[4]*scale);\r\n" \
	"tc += texture2D(glowSource, uv + v).rgb * weight[4];\r\n" \
	"tc += texture2D(glowSource, uv - v).rgb * weight[4];\r\n" \
	"gl_FragColor = vec4(tc, 1.0) * brightness;\r\n" \
	"}\r\n"
	,
	"uniform sampler2D glowSource;\r\n" \
	"uniform float scale; // render target width/height\r\n" \
	"uniform float brightness; // render target width/height\r\n" \
	"float offset[5] = float[](0.0, 1.0, 2.0, 3.0, 4.0);\r\n" \
	"float weight[5] = float[](0.18, 0.15, 0.12, 0.09, 0.05);\r\n" \
	"void main() {\r\n" \
	"vec2 uv = gl_TexCoord[0].xy;\r\n" \
	"vec3 tc = texture2D(glowSource, uv).rgb * weight[0];\r\n" \
	"vec2 v = vec2 (offset[1]*scale, 0.0);\r\n" \
	"tc += texture2D(glowSource, uv + v).rgb * weight[1];\r\n" \
	"tc += texture2D(glowSource, uv - v).rgb * weight[1];\r\n" \
	"v = vec2 (offset[2]*scale, 0.0);\r\n" \
	"tc += texture2D(glowSource, uv + v).rgb * weight[2];\r\n" \
	"tc += texture2D(glowSource, uv - v).rgb * weight[2];\r\n" \
	"v = vec2 (offset[3]*scale, 0.0);\r\n" \
	"tc += texture2D(glowSource, uv + v).rgb * weight[3];\r\n" \
	"tc += texture2D(glowSource, uv - v).rgb * weight[3];\r\n" \
	"v = vec2 (offset[4]*scale, 0.0);\r\n" \
	"tc += texture2D(glowSource, uv + v).rgb * weight[4];\r\n" \
	"tc += texture2D(glowSource, uv - v).rgb * weight[4];\r\n" \
	"gl_FragColor = vec4(tc, 1.0) * brightness;\r\n" \
	"}\r\n"
	};

#endif

const char *blurVS =
	"void main (void){\r\n" \
	"gl_TexCoord [0] = gl_MultiTexCoord0;\r\n" \
	"gl_Position = ftransform (); //gl_ModelViewProjectionMatrix * gl_Vertex;\r\n" \
	"gl_FrontColor = gl_Color;}\r\n"
	;

//------------------------------------------------------------------------------

bool CGlowRenderer::LoadShader (int const direction, float const radius)
{
	//float fScale [2] = {ogl.m_data.windowScale.dim.y * (radius * 0.5f + 1.0f), ogl.m_data.windowScale.dim.x * (radius * 0.5f + 1.0f)};
	float fScale [2] = {ogl.m_data.windowScale.dim.y * 2.0f, ogl.m_data.windowScale.dim.x * 2.0f};
	//float fScale [2] = {1.0f / float (gameData.render.scene.Height ()) * radius, 1.0f / float (gameData.render.scene.Width ()) * radius};

m_shaderProg = GLhandleARB (shaderManager.Deploy (hBlurShader /*[direction]*/));
if (!m_shaderProg)
	return false;
if (shaderManager.Rebuild (m_shaderProg))
	/*nothing*/;
shaderManager.Set ("glowSource", 0);
shaderManager.Set ("direction", float (direction));
shaderManager.Set ("scale", fScale [direction]);
shaderManager.Set ("brightness", m_brightness);
return true;
}

//-------------------------------------------------------------------------

void CGlowRenderer::InitShader (void)
{
ogl.m_states.bGlowRendering = 0;
//DeleteShaderProg (NULL);
if (ogl.m_features.bRenderToTexture && ogl.m_features.bShaders) {
	PrintLog (0, "building glow shader program\n");
	ogl.m_states.bGlowRendering = 1;
	m_shaderProg = 0;
#if 1
	if (!shaderManager.Build (hBlurShader, blurFS, blurVS)) {
		ogl.ClearError (0);
		ogl.m_states.bGlowRendering = 0;
		}
#else
	for (int i = 0; i < 2; i++) {
		if (!shaderManager.Build (hBlurShader [i], blurFS [i], blurVS)) {
			ogl.ClearError (0);
			ogl.m_states.bGlowRendering = 0;
			}
		}
#endif
	}
}

//-------------------------------------------------------------------------

bool CGlowRenderer::ShaderActive (void)
{
#if 1
if ((hBlurShader >= 0) && (shaderManager.Current () == hBlurShader))
	return true;
#else
for (int i = 0; i < 2; i++)
	if ((hBlurShader [i] >= 0) && (shaderManager.Current () == hBlurShader [i]))
		return true;
#endif
return false;
}

//------------------------------------------------------------------------------

static void ClearDrawBuffer (int nType)
{
#if 0 //DBG
if (gameStates.render.cameras.bActive) {
	glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT);
	float vertices [4][4][2] = {
		{{0.0, 0.0}, {0.0, 0.5}, {0.5, 0.5}, {0.5, 0.0}},
		{{0.5, 0.0}, {0.5, 0.5}, {1.0, 0.5}, {1.0, 0.0}},
		{{0.0, 0.5}, {0.0, 1.0}, {0.5, 1.0}, {0.5, 0.5}},
		{{0.5, 0.5}, {0.5, 1.0}, {1.0, 1.0}, {1.0, 0.5}},
		};
	float colors [4][4] = {
		{1.0, 0.5, 0.0, 0.5},
		{0.0, 0.5, 1.0, 0.5},
		{1.0, 0.0, 0.5, 0.5},
		{0.0, 1.0, 0.5, 0.5}
	};

	GLenum nBlendModes [2], nDepthMode = ogl.GetDepthMode ();
	ogl.GetBlendMode (nBlendModes [0], nBlendModes [1]);

	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();//clear matrix
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();//clear matrix
	glOrtho (0.0, 1.0, 1.0, 0.0, -1.0, 1.0);

	ogl.EnableClientStates (1, 0, 0, GL_TEXTURE0);
	ogl.SetTexturing (false);
	ogl.SetBlendMode (OGL_BLEND_REPLACE);
	ogl.SetDepthMode (GL_ALWAYS);
	for (int i = 0; i < 4; i++) {
		ogl.EnableClientStates (1, 0, 0, GL_TEXTURE0);
		ogl.SetTexturing (false);
		glColor3fv (colors [i]);
		OglVertexPointer (2, GL_FLOAT, 0, vertices [i]);
		OglDrawArrays (GL_QUADS, 0, 4);
		}
	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
	glPopMatrix ();
	ogl.SetBlendMode (nBlendModes [0], nBlendModes [1]);
	ogl.SetDepthMode (nDepthMode);
	return;
	}
#endif
if (nType == BLUR_SHADOW) 
	glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
else 
#if 0 && DBG
	glClearColor (1.0f, 0.5f, 0.0f, 0.25f);
#else
	glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
#endif
glClear (GL_COLOR_BUFFER_BIT);
}

//------------------------------------------------------------------------------

int CGlowRenderer::Activate (void)
{
if (!ogl.SelectGlowBuffer ()) {
#if DBG
	ogl.SelectGlowBuffer ();
#endif
	return 0;
	}
CCanvas::Current ()->SetViewport ();
ClearDrawBuffer (m_nType);
return 1;
}

//------------------------------------------------------------------------------

bool CGlowRenderer::Reset (int bGlow, int bOgl)
{
m_nType = -1;
m_nStrength = -1;
m_bViewport = 0;
if (bOgl) {
	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
	glPopMatrix ();
	}
CCanvas::Current ()->SetViewport ();
return 0 != (gameOpts->render.effects.bGlow = bGlow);
}

//------------------------------------------------------------------------------

inline int ScreenScale (void)
{
return (!gameStates.render.cameras.bActive || gameOpts->render.cameras.bHires) ? 1 : 2;
}

#if 0

inline int ScreenWidth (void)
{
return gameData.render.screen.Width () /*/ ScreenScale ()*/;
}

inline int ScreenHeight (void)
{
return gameData.render.screen.Height () /*/ ScreenScale ()*/;
}

#else

inline int ScreenWidth (void)
{
return CCanvas::Current ()->Width (); 
}

inline int ScreenHeight (void)
{
return CCanvas::Current ()->Height ();
}

#endif

//------------------------------------------------------------------------------

void CGlowRenderer::SetItemExtent (CFloatVector3 v, bool bTransformed)
{
#if USE_VIEWPORT
if (gameOpts->render.effects.bGlow != 1)
	return;
if (!bTransformed)
	transformation.Transform (v, v);
tScreenPos s;
#	if 0
ProjectPoint (v, s);
#	else
v = transformation.m_info.projection * v;
float z = fabs (v.v.coord.z);
float w = (float) gameData.render.scene.Width () * 0.5f;
float h = (float) gameData.render.scene.Height () * 0.5f;
s.x = fix (w * (1.0f + v.v.coord.x / z));
s.y = fix (h * (1.0f + v.v.coord.y / z));
#	endif
#	if 0
if (gameStates.render.cockpit.nType == CM_LETTERBOX)
	s.y += (gameData.render.screen.Height () - CCanvas::Current ()->Height ()) / 2;
else if (gameStates.render.cockpit.nType != CM_FULL_SCREEN)
	s.y += gameData.render.screen.Height () - CCanvas::Current ()->Height ();
#	endif
#pragma omp critical
{
if (m_itemMin.x > s.x)
	m_itemMin.x = s.x;
if (m_itemMin.y > s.y)
	m_itemMin.y = s.y;
if (m_itemMax.x < s.x)
	m_itemMax.x = s.x;
if (m_itemMax.y < s.y)
	m_itemMax.y = s.y;
}
m_bViewport = 1;
#else
m_bViewport = 0;
#endif
}

//------------------------------------------------------------------------------

bool CGlowRenderer::UseViewport (void)
{
#if USE_VIEWPORT
return !ogl.IsSideBySideDevice () && (gameOpts->render.effects.bGlow == 1);
#else
return 0;
#endif
}

//------------------------------------------------------------------------------

bool CGlowRenderer::Visible (void)
{
#if USE_VIEWPORT
if (!UseViewport ())
	return true;
if (m_bViewport == 0)
	return false;
if (m_bViewport == -1) // no extent set
	return true;

if ((m_itemMax.x < 0) || (m_itemMin.x >= ScreenWidth ()) ||
	 (m_itemMax.y < 0) || (m_itemMin.y >= ScreenHeight ()))
	 return false;
if (m_renderMin.x > m_itemMin.x)
	m_renderMin.x = m_itemMin.x;
if (m_renderMax.x < m_itemMax.x)
	m_renderMax.x = m_itemMax.x;
if (m_renderMin.y > m_itemMin.y)
	m_renderMin.y = m_itemMin.y;
if (m_renderMax.y < m_itemMax.y)
	m_renderMax.y = m_itemMax.y;
#endif
return true;
}

//------------------------------------------------------------------------------

void CGlowRenderer::InitViewport (void)
{
if (!UseViewport ()) {
	m_renderMin.x = CCanvas::Current ()->Left ();
	m_renderMin.y = CCanvas::Current ()->Top ();
	m_renderMax.x = CCanvas::Current ()->Right ();
	m_renderMax.y = CCanvas::Current ()->Bottom ();
	}	
else if (!m_bViewport) {
	m_renderMin.x = CCanvas::Current ()->Right ();
	m_renderMin.y = CCanvas::Current ()->Bottom ();
	m_renderMax.x = CCanvas::Current ()->Left ();
	m_renderMax.y = CCanvas::Current ()->Top ();
	m_bViewport = -1;
	}
}

//------------------------------------------------------------------------------

bool CGlowRenderer::SetViewport (int const nType, CFloatVector3* vertexP, int nVerts)
{
if (!Available (nType))
	return true;
if ((GLOW_FLAGS & nType) == 0)
	return false;
#if USE_VIEWPORT
if (gameOpts->render.effects.bGlow != 1)
	return true;
//#pragma omp parallel for
m_itemMin.x = m_itemMin.y = 0x7FFF;
m_itemMax.x = m_itemMax.y = -0x7FFF;
for (int i = 0; i < nVerts; i++)
	SetItemExtent (vertexP [i]);
#endif
return Visible ();
}

//------------------------------------------------------------------------------

bool CGlowRenderer::SetViewport (int const nType, CFloatVector* vertexP, int nVerts)
{
if (!Available (nType))
	return true;
if ((GLOW_FLAGS & nType) == 0)
	return false;
#if USE_VIEWPORT
if (!UseViewport ())
	return true;
//#pragma omp parallel for
m_itemMin.x = m_itemMin.y = 0x7FFF;
m_itemMax.x = m_itemMax.y = -0x7FFF;
for (int i = 0; i < nVerts; i++) 
	SetItemExtent (*(vertexP [i].XYZ ()));
#endif
return Visible ();
}

//------------------------------------------------------------------------------

bool CGlowRenderer::SetViewport (int const nType, CFloatVector3 v, float width, float height, bool bTransformed)
{
if (!Available (nType))
	return true;
if ((GLOW_FLAGS & nType) == 0)
	return false;
#if USE_VIEWPORT
if (!UseViewport ())
	return true;
if (!bTransformed)
	transformation.Transform (v, v);
CFloatVector3 r;
r.Set (width, height, 0.0f);
m_itemMin.x = m_itemMin.y = 0x7FFF;
m_itemMax.x = m_itemMax.y = -0x7FFF;
SetItemExtent (v - r, true);
SetItemExtent (v + r, true);
#endif
return Visible ();
}

//------------------------------------------------------------------------------

bool CGlowRenderer::SetViewport (int const nType, CFixVector pos, float radius)
{
if (!Available (nType))
	return true;
if ((GLOW_FLAGS & nType) == 0)
	return false;
#if USE_VIEWPORT
if (!UseViewport ())
	return true;
CFloatVector3 v;
v.Assign (pos);
m_itemMin.x = m_itemMin.y = 0x7FFF;
m_itemMax.x = m_itemMax.y = -0x7FFF;
return SetViewport (nType, v, radius, radius);
#else
return true;
#endif
}

//------------------------------------------------------------------------------

bool CGlowRenderer::Available (int const nType, bool bForce)
{
#if 0 //DBG
if (nType == GLOW_SHIELDS)
	return false;
#endif
if (!ogl.m_features.bShaders)
	return false;
if (ogl.m_features.bDepthBlending < 0)
	return false;
if (!ogl.m_features.bMultipleRenderTargets)
	return false;
if (gameStates.render.nShadowMap > 0)
	return false;
if (!ogl.m_states.bGlowRendering)
	return false;
if (!gameOpts->render.effects.bEnabled)
	return false;
if (gameStates.render.cameras.bActive && !gameOpts->render.cameras.bHires)
	return false;
if ((GLOW_FLAGS & nType) == 0)
	return false;
if (!(bForce || gameOpts->render.effects.bGlow))
	return false;
if (gameOptions [0].render.nQuality < 2)
	return false;
return true;
}

//------------------------------------------------------------------------------

bool CGlowRenderer::Begin (int const nType, int const nStrength, bool const bReplace, float const brightness)
{
if (!Available (nType))
	return false;
if ((gameOptions [0].render.nQuality < 3) && automap.Display ())
	return false;
#if 0
if (nType != m_nType) {
#else
if ((m_bReplace == bReplace) && (m_nStrength == nStrength) && (m_brightness == brightness) && ((nType == GLOW_LIGHTNING) == (m_nType == GLOW_LIGHTNING))) {
	gameOpts->render.effects.bGlow = (ogl.SelectGlowBuffer () >= 0);
	CCanvas::Current ()->SetViewport ();
	}
else {
#endif
	End ();
	m_nType = nType;
	m_bReplace = bReplace;
	m_nStrength = nStrength;
	m_brightness = brightness;
	m_bViewport = 0;
	InitViewport ();
	gameOpts->render.effects.bGlow = Activate ();
	}
if (gameOpts->render.effects.bGlow)
	return true;
Reset (0);
return false;
}

//------------------------------------------------------------------------------

inline float ScreenCoord (float v, float m)
{
float c = v / m;
if (c < 0.0f)
	return 0.0f;
if (c > 1.0f)
	return 1.0f;
return c;
}

//------------------------------------------------------------------------------

void RenderTestImage (void);

static int bEnableViewport = 1;

void CGlowRenderer::Render (int const source, int const direction, float const radius, float const scale)
{
#if USE_VIEWPORT == 2 //DBG

	bool bUseRadius = UseViewport () && !ogl.IsSideBySideDevice ();

float r = bUseRadius ? 0.0f : radius * 4.0f; // scale with a bit more than the max. offset from the blur shader
// define the destination area to be rendered to
float w = (float) gameData.render.frame.Width ();
float h = (float) gameData.render.frame.Height ();
if (ogl.IsSideBySideDevice ())
	w *= 2;
float verts [4][2] = {
	{ScreenCoord ((float) m_renderMin.x - r, (float) w),
	 ScreenCoord ((float) m_renderMin.y - r, (float) h)},
	{ScreenCoord ((float) m_renderMin.x - r, (float) w),
	 ScreenCoord ((float) m_renderMax.y + r, (float) h) * scale},
	{ScreenCoord ((float) m_renderMax.x + r, (float) w) * scale,
	 ScreenCoord ((float) m_renderMax.y + r, (float) h) * scale},
	{ScreenCoord ((float) m_renderMax.x + r, (float) w) * scale,
	 ScreenCoord ((float) m_renderMin.y - r, (float) h)}
	};
if (bUseRadius) 
	r += 4.0f;
// define the source area (part of the glow buffer, which serves as a texture here) to be rendered
float texCoord [4][2] = {
	{ScreenCoord ((float) m_renderMin.x - r, (float) w),
	 ScreenCoord ((float) m_renderMin.y - r, (float) h)},
	{ScreenCoord ((float) m_renderMin.x - r, (float) w),
	 ScreenCoord ((float) m_renderMax.y + r, (float) h)},
	{ScreenCoord ((float) m_renderMax.x + r, (float) w),
	 ScreenCoord ((float) m_renderMax.y + r, (float) h)},
	{ScreenCoord ((float) m_renderMax.x + r, (float) w),
	 ScreenCoord ((float) m_renderMin.y - r, (float) h)}
	};

#else

float verts [2][4][2] = {
	{{0,0},{0,1},{1,1},{1,0}},
	{{0,0},{0,1},{1,1},{1,0}},
	};

float w, h, l, r, b, t;

if (bEnableViewport && UseViewport ()) {
	w = (float) gameData.render.scene.Width ();
	h = (float) gameData.render.scene.Height ();

	m_renderMin.x = Clamp (m_renderMin.x, 0, CCanvas::Current ()->Width ());
	m_renderMax.x = Clamp (m_renderMax.x, 0, CCanvas::Current ()->Width ());
	m_renderMin.y = Clamp (m_renderMin.y, 0, CCanvas::Current ()->Height ());
	m_renderMax.y = Clamp (m_renderMax.y, 0, CCanvas::Current ()->Height ());

	l = (float) m_renderMin.x - radius;
	r = (float) m_renderMax.x + radius;
	b = (float) m_renderMin.y - radius;
	b = (float) CCanvas::Current ()->Height () - b;
	t = (float) m_renderMax.y + radius;
	t = (float) CCanvas::Current ()->Height () - t;
#if 0
	b = h - t;
	t = b + (float) (m_renderMax.y + radius - m_renderMin.y + radius);
#elif 0
	b = (float) m_renderMax.y + radius + h - (float) CCanvas::Current ()->Bottom ();
#endif

	verts [1][0][0] = ScreenCoord (l, w);
	verts [1][0][1] = ScreenCoord (t, h);
	verts [1][1][0] = ScreenCoord (l, w);
	verts [1][1][1] = ScreenCoord (b, h);
	verts [1][2][0] = ScreenCoord (r, w);
	verts [1][2][1] = ScreenCoord (b, h);
	verts [1][3][0] = ScreenCoord (r, w);
	verts [1][3][1] = ScreenCoord (t, h);

	w = (float) gameData.render.screen.Width ();
	h = (float) gameData.render.screen.Height ();
#if 1
	if (bEnableViewport < 0) {
		l = (float) CCanvas::Current ()->Left ();
		r = (float) CCanvas::Current ()->Right ();
		b = h - (float) CCanvas::Current ()->Top ();
		t = b - (float) CCanvas::Current ()->Height ();
		}
	else {
#if 0
		l = (float) m_renderMin.x - radius;
		r = (float) m_renderMax.x + radius;
		t = (float) m_renderMin.y - radius;
		b = (float) m_renderMax.y + radius;
#endif
		l += (float) CCanvas::Current ()->Left ();
		r += (float) CCanvas::Current ()->Left ();
		t += (float) CCanvas::Current ()->Top ();
		b += (float) CCanvas::Current ()->Top ();
#	if 1
		t = h - t;
		b = h - b;
		if (b > t)
			Swap (b, t);
#endif
#	if 0
		b += (float) CCanvas::Current ()->Top ();
#	elif 0
		b = h - t;
		t = b - (float) (m_renderMax.y + radius - m_renderMin.y + radius);
#	endif
		}
#	endif
	}
else {
	w = (float) gameData.render.screen.Width ();
	h = (float) gameData.render.screen.Height ();
	l = (float) CCanvas::Current ()->Left ();
	r = (float) CCanvas::Current ()->Right ();
#if 0
	t = (float) CCanvas::Current ()->Top ();
	b = (float) CCanvas::Current ()->Bottom ();
#else
	t = h - (float) CCanvas::Current ()->Top ();
	b = t - (float) CCanvas::Current ()->Height ();
#endif
	}

float texCoord [4][2] = {
	{ScreenCoord (l, w), ScreenCoord (t, h)},
	{ScreenCoord (l, w), ScreenCoord (b, h)},
	{ScreenCoord (r, w), ScreenCoord (b, h)},
	{ScreenCoord (r, w), ScreenCoord (t, h)}
	};

#endif

#if 1 //!DBG
if (direction >= 0) {
#if 0 //DBG
	RenderTestImage ();
	return;
#endif
	LoadShader (direction, radius);
	}
else
#endif 
	{
	shaderManager.Deploy (-1);
	}
#if 0 && DBG
ogl.EnableClientStates (0, 0, 0, GL_TEXTURE0);
ogl.SetTexturing (false);
glColor4f (0.0f, 0.5f, 1.0f, 0.25f);
#else
ogl.EnableClientStates (1, 0, 0, GL_TEXTURE0);
ogl.BindTexture (ogl.BlurBuffer (source)->ColorBuffer (source < 0));
OglTexCoordPointer (2, GL_FLOAT, 0, texCoord);
glColor3f (1,1,1);
#endif
OglVertexPointer (2, GL_FLOAT, 0, verts [bEnableViewport && UseViewport ()]);
OglDrawArrays (GL_QUADS, 0, 4);
ogl.BindTexture (0);
}

//------------------------------------------------------------------------------

void CGlowRenderer::ClearViewport (float const radius)
{
#if USE_VIEWPORT
if ((m_bViewport > 0) && (radius > 0.0f)) {
	ogl.SaveViewport ();
	float r = radius * 4.0f * m_nStrength; // scale with a bit more than the max. offset from the blur shader
	m_renderMin.x = max (m_renderMin.x, 0);
	m_renderMin.y = max (m_renderMin.y, 0);
	glViewport ((GLint) max (m_renderMin.x - r, 0), 
					(GLint) max (m_renderMin.y - r, 0), 
					(GLsizei) min (m_renderMax.x - m_renderMin.x + 1 + 2 * r, ScreenWidth ()), 
					(GLsizei) min (m_renderMax.y - m_renderMin.y + 1 + 2 * r, ScreenHeight ()));
	}

ClearDrawBuffer (m_nType);
if (radius > 0.0f)
	ogl.RestoreViewport ();
#else
glClear (GL_COLOR_BUFFER_BIT);
#endif
}

//------------------------------------------------------------------------------

void CGlowRenderer::ChooseDrawBuffer (void)
{
ogl.ChooseDrawBuffer ();
}

//------------------------------------------------------------------------------

void CGlowRenderer::Done (const int nType)
{
if (Available (nType)) {
#if 1
	ogl.DrawBuffer ()->SelectColorBuffers (0);
#else
	ogl.ChooseDrawBuffer ();
#endif
	CCanvas::Current ()->SetViewport ();
	}	
}

//------------------------------------------------------------------------------

bool CGlowRenderer::End (void)
{
if (m_nStrength < 0)
	return false;

#if USE_VIEWPORT
if (!Visible ())
	ogl.ChooseDrawBuffer ();
else
#endif
	{
	gameData.render.scene.Activate ("CGlowRenderer::End");
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();//clear matrix
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();//clear matrix
	glOrtho (0.0, 1.0, 1.0, 0.0, -1.0, 1.0);

	GLenum nBlendModes [2], nDepthMode = ogl.GetDepthMode ();
	bool bDepthWrite = ogl.GetDepthWrite ();
	ogl.GetBlendMode (nBlendModes [0], nBlendModes [1]);

	ogl.SetDepthWrite (false);
	ogl.SetAlphaTest (false);
	ogl.SetFaceCulling (false);
	ogl.ResetClientStates (1);

	float radius = 0.0f;
	if (m_bViewport < 0) {
		m_renderMin.x = CCanvas::Current ()->Left ();
		m_renderMin.y = CCanvas::Current ()->Top ();
		m_renderMax.x = CCanvas::Current ()->Right ();
		m_renderMax.y = CCanvas::Current ()->Bottom ();
		}

#if BLUR
	ogl.SetDepthMode (GL_ALWAYS);
	ogl.SetBlendMode (OGL_BLEND_REPLACE);

	radius += RAD_INCR;
	if (!ogl.SelectBlurBuffer (0))
		return Reset (0, 1);
	ClearViewport (radius);
	Render (-1, 0, radius); // Glow -> Blur 0
	if (!ogl.SelectBlurBuffer (1))
		return Reset (0, 1);
	ClearViewport (radius);
	Render (0, 1, radius); // Blur 0 -> Blur 1
	if (m_nType != BLUR_SHADOW)
		ogl.SetBlendMode (OGL_BLEND_ADD);
#	if BLUR > 1
	for (int i = 1; i < m_nStrength; i++) {
		radius += RAD_INCR;
		if (!ogl.SelectBlurBuffer (0))
			return Reset (0, 1);
		Render (1, 0, radius); // Blur 1 -> Blur 0
		if (!ogl.SelectBlurBuffer (1))
			return Reset (0, 1);
		Render (0, 1, radius); // Blur 0 -> Blur 1
		}
	//radius += RAD_INCR;
#	endif
#endif

	ogl.ChooseDrawBuffer ();
	ogl.SetDepthMode (GL_ALWAYS);

	float scale = (float) ScreenScale ();
#if BLUR
	ogl.SetBlendMode ((m_nType == BLUR_SHADOW) ? OGL_BLEND_MULTIPLY : OGL_BLEND_ADD);
	Render (1, -1, radius, (scale == 1.0f) ? 1.0f : 8.0f); // Glow -> back buffer
	if (!m_bReplace)
		Render (-1, -1, radius, scale); // render the unblurred stuff on top of the blur
#else
	ogl.SetBlendMode ((m_nType == BLUR_SHADOW) ? OGL_BLEND_MULTIPLY : OGL_BLEND_ADD);
	Render (-1, -1, radius, scale); // render the unblurred stuff on top of the blur
#endif
	ogl.DisableClientStates (1, 0, 0, GL_TEXTURE0);
	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
	glPopMatrix ();
	ogl.SetBlendMode (nBlendModes [0], nBlendModes [1]);
	ogl.SetDepthWrite (bDepthWrite);
	ogl.SetAlphaTest (true);
	ogl.SetDepthMode (nDepthMode);
	ogl.SetStencilTest (false);
	ogl.SetFaceCulling (true);
	CCanvas::Current ()->Deactivate ();
	}
return Reset (gameOpts->render.effects.bGlow);
}

//------------------------------------------------------------------------------


