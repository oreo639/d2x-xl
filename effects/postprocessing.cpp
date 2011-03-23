/* Image post processing (fullscreen effects)
	Dietfrid Mali
	22.3.2011
*/

#include "descent.h"
#include "ogl_defs.h"
#include "ogl_lib.h"
#include "ogl_tmu.h"
#include "ogl_color.h"
#include "ogl_shader.h"
#include "ogl_render.h"
#include "transformation.h"
#include "postprocessing.h"

CPostProcessManager postProcessManager;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// shockwave effect

int hShockwaveShader = -1;

const char* shockwaveVS = 
	"void main(void) {\r\n" \
	"gl_Position = ftransform();\r\n" \
	"gl_TexCoord[0] = gl_MultiTexCoord0;\r\n" \
	"}";

const char* shockwaveFS = 
	"uniform sampler2D sceneTex;\r\n" \
	"uniform sampler2D depthTex;\r\n" \
	"uniform int nBias;\r\n" \
	"uniform int nShockwaves;\r\n" \
	"uniform vec2 screenSize;\r\n" \
	"uniform vec3 effectStrength;\r\n" \
	"#define LinearDepth(_z) 10000.0 / (5001.0 - (_z) * 4999.0)\r\n" \
	"void main() {\r\n" \
	"vec2 tcSrc = gl_TexCoord [0].xy * screenSize;\r\n" \
	"vec2 tcDest = tcSrc; //vec2 (0.0, 0.0);\r\n" \
	"int i, h = 0;\r\n" \
	"for (i = 0; i < 8; i++) if (i < nShockwaves) {\r\n" \
	"  vec2 v = tcSrc - gl_LightSource [i].position.xy;\r\n" \
	"  float d = length (v);\r\n" \
	"  float r = gl_LightSource [i].constantAttenuation;\r\n" \
	"  float offset = d - r;\r\n" \
	"  if (abs (offset) <= effectStrength.z) {\r\n" \
	"    if (LinearDepth (texture2D (depthTex, gl_TexCoord [0].xy).r) <= 50.0) h = 1;\r\n" \
	"    if (gl_LightSource [i].position.z <= LinearDepth (texture2D (depthTex, gl_TexCoord [0].xy).r)) {\r\n" \
	"      offset /= screenSize.x;\r\n" \
	"      offset *= (1.0 - pow (abs (offset) * effectStrength.x, effectStrength.y)) * pow (gl_LightSource [i].quadraticAttenuation, 0.25);\r\n" \
	"      tcDest -= v * (nBias * offset / d) * screenSize;\r\n" \
	"      }\r\n" \
	"    }\r\n" \
	"  }\r\n" \
	"gl_FragColor = (h == 1) ? vec4 (1.0, 0.5, 0.0, 1.0) : texture2D (sceneTex, tcDest / screenSize);\r\n" \
	"/*float r = 1.0 - LinearDepth (texture2D (depthTex, gl_TexCoord [0].xy).r) / 5000.0;\r\n" \
	"float r = (1.0 + texture2D (depthTex, gl_TexCoord [0].xy).r) / 2.0;\r\n" \
	"gl_FragColor = vec4 (r, r, r, 1.0);*/\r\n" \
	"}\r\n";

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CPostProcessManager::Destroy (void) 
{
while (m_effects) {
	CPostEffect* e = m_effects;
	m_effects = m_effects->Next ();
	delete e;
	}
m_nEffects = 0;
}

//------------------------------------------------------------------------------

void CPostProcessManager::Add (CPostEffect* e) 
{
if (!(ogl.m_states.bRender2TextureOk && e->Enabled ()))
	delete e;
else {
	e->Link (NULL, m_effects);
	if (m_effects)
		m_effects->Link (e, m_effects->Next ());
	m_effects = e;
	}
}

//------------------------------------------------------------------------------

void CPostProcessManager::Render (void)
{
for (CPostEffect* e = m_effects; e; e = e->Next ()) 
	e->Render ();
}

//------------------------------------------------------------------------------

void CPostProcessManager::Update (void)
{
for (CPostEffect* e = m_effects; e; ) {
	CPostEffect* h = e;
	e = e->Next ();
	if (h->Terminate ()) {
		if (m_effects == h)
			m_effects = m_effects->Next ();
		h->Unlink ();
		delete h;
		}
	else
		h->Update ();
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

int CPostEffectShockwave::m_nShockwaves = 0;
GLhandleARB CPostEffectShockwave::m_shaderProg;

void CPostEffectShockwave::InitShader (void)
{
PrintLog ("building shockwave shader program\n");
if (ogl.m_states.bRender2TextureOk && ogl.m_states.bShadersOk) {
	m_shaderProg = 0;
	if (!shaderManager.Build (hShockwaveShader, shockwaveFS, shockwaveVS)) {
		ogl.ClearError (0);
		}
	}
}

//------------------------------------------------------------------------------

bool CPostEffectShockwave::LoadShader (const CFixVector pos, int size, const float ttl, const int nBias)
{
	static CFloatVector3 effectStrength = {10.0f, 0.8f, screen.Width () * 0.1f};

if (m_nShockwaves >= 8)
	return true;

int i;
CFixVector p [5];

for (i = 0; i < 2; i++)
	transformation.TransformAndEncode (p [i], VERTICES [56 + i]);
for (; i < 4; i++)
	transformation.TransformAndEncode (p [i], VERTICES [599 + i]);
	
if (transformation.TransformAndEncode (p [0], pos) & CC_BEHIND)
	return true;

size = int (float (size) * ttl);
p [1].v.coord.x = 
p [4].v.coord.x = p [0].v.coord.x - size;
p [1].v.coord.y = 
p [2].v.coord.y = p [0].v.coord.y - size;
p [2].v.coord.x = 
p [3].v.coord.x = p [0].v.coord.x + size;
p [3].v.coord.y = 
p [4].v.coord.y = p [0].v.coord.y + size;
p [1].v.coord.z =
p [2].v.coord.z =
p [3].v.coord.z =
p [4].v.coord.z = p [0].v.coord.z;

for (i = 1; i < 5; i++) {
	if (!(transformation.Codes (p [i]) & CC_BEHIND))
		break;
	}	
if (i == 5)
	return true;

tScreenPos s [5];
for (int i = 0; i < 5; i++)
	ProjectPoint (p [i], s [i], 0, 0);

int d = 0;
int n = 0;
for (int i = 1; i < 5; i++) {
	if ((s [i].x >= 0) && (s [i].x < screen.Width ()) && (s [i].y >= 0) && (s [i].y < screen.Height ())) {
		d += labs (s [0].x - s [i].x) + labs (s [0].y - s [i].y);
		n += 4;
		}
	}
if (n == 0)
	return true;

if (m_nShockwaves == 0) {
	if (hShockwaveShader < 0) {
		InitShader ();
		if (hShockwaveShader < 0)
			return false;
		}
	if (!ogl.CopyDepthTexture (1))
		return false;
	m_shaderProg = GLhandleARB (shaderManager.Deploy (hShockwaveShader /*[direction]*/));
	if (!m_shaderProg)
		return false;
	if (shaderManager.Rebuild (m_shaderProg))
		;
	glUniform1i (glGetUniformLocation (m_shaderProg, "sceneTex"), 0);
	glUniform1i (glGetUniformLocation (m_shaderProg, "depthTex"), 1);
	glUniform1i (glGetUniformLocation (m_shaderProg, "nBias"), nBias);
	glUniform3fv (glGetUniformLocation (m_shaderProg, "effectStrength"), 1, reinterpret_cast<GLfloat*> (&effectStrength));
	float screenSize [2] = {screen.Width (), screen.Height () };
	glUniform2fv (glGetUniformLocation (m_shaderProg, "screenSize"), 1, reinterpret_cast<GLfloat*> (screenSize));
	ogl.SetLighting (true);
	}

CFloatVector3 f;
f.v.coord.x = float (s [0].x)/* / float (screen.Width ())*/;
f.v.coord.y = float (s [0].y)/*/ float (screen.Height ())*/;
f.v.coord.z = X2F (p [0].v.coord.z); /* / float (screen.Width ())*/;
glEnable (GL_LIGHT0 + m_nShockwaves);
glLightfv (GL_LIGHT0 + m_nShockwaves, GL_POSITION, reinterpret_cast<GLfloat*> (&f));
glLightf (GL_LIGHT0 + m_nShockwaves, GL_CONSTANT_ATTENUATION, float (d) / float (n));
glLightf (GL_LIGHT0 + m_nShockwaves, GL_QUADRATIC_ATTENUATION, 1.0f - ttl);
glUniform1i (glGetUniformLocation (m_shaderProg, "nShockwaves"), ++m_nShockwaves);
return true;
}

//------------------------------------------------------------------------------

float CPostEffectShockwave::Life (void)
{
return float (m_nLife) * gameStates.gameplay.slowmo [0].fSpeed;
}
	 
//------------------------------------------------------------------------------

void CPostEffectShockwave::Update (void)
{
LoadShader (m_pos, m_nSize, float (SDL_GetTicks () - m_nStart) / Life (), m_nBias);
}

//------------------------------------------------------------------------------

void CPostEffectShockwave::Render (void)
{
// render current render target
OglDrawArrays (GL_QUADS, 0, 4);
if (m_nShockwaves > 0) {
	for (int i = 0; i < m_nShockwaves; i++)
		glDisable (GL_LIGHT0 + i);
	ogl.SetLighting (false);
	m_nShockwaves = 0;
	}
}

//------------------------------------------------------------------------------

bool CPostEffectShockwave::Enabled (void)
{
return gameOpts->render.effects.bEnabled && (gameOpts->render.effects.nShockwaves > 1);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

