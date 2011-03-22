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
	"uniform int nShockwaves; // explosion center\r\n" \
	"uniform vec3 effectStrength; // 10.0, 0.8, 0.1\r\n" \
	"void main() {\r\n" \
	"vec2 texCoord = gl_TexCoord [0].xy;\r\n" \
	"int i;\r\n" \
	"for (i = 0; i < 8; i++) if (i < nShockwaves) {\r\n" \
	"  vec2 v = texCoord - gl_LightSource [i].position.xy;\r\n" \
	"  float r = length (v);\r\n" \
	"  float d = r - gl_LightSource [i].position.z;\r\n" \
	"  if (abs (d) <= effectStrength.z) {\r\n" \
	"    d *= 1.0 - pow (abs (d) * effectStrength.x, effectStrength.y) * sqrt (gl_LightSource [i].quadraticAttenuation);\r\n" \
	"    texCoord += v / (r * d);\r\n" \
	"    }\r\n" \
	"  }\r\n" \
	"gl_FragColor = texture2D (sceneTex, texCoord);\r\n" \
	"}\r\n";

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


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
e->Link (NULL, m_effects);
if (m_effects)
	m_effects->Link (e, m_effects->Next ());
m_effects = e;
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

bool CPostEffectShockwave::LoadShader (const CFixVector pos, const int size, const float ttl)
{
	static CFloatVector3 effectStrength = {10.0f, 0.8f, 0.1f};

if (m_nShockwaves >= 8)
	return true;

CFixVector p [5];
if (transformation.TransformAndEncode (p [0], pos) & CC_BEHIND)
	return true;
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

int i;

for (i = 1; i < 5; i++) {
	if (!(transformation.Codes (p [i]) & CC_BEHIND))
		break;
	}	
if (i == 5)
	return true;

tScreenPos s [5];
for (int i = 1; i < 5; i++)
	ProjectPoint (p [i], s [i], 0, 0);

int d = 0;
int n = 0;
for (int i = 1; i < 5; i++) {
	if ((s [i].x >= 0) && (s [i].x < screen.Width ()) && (s [i].y >= 0) && (s [i].y < screen.Height ())) {
		d += labs (s [0].x - s [i].x) + labs (s [0].y - s [i].y);
		n += 2;
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
	m_shaderProg = GLhandleARB (shaderManager.Deploy (hShockwaveShader /*[direction]*/));
	if (!m_shaderProg)
		return false;
	if (shaderManager.Rebuild (m_shaderProg))
		;
	glUniform1i (glGetUniformLocation (m_shaderProg, "sceneTex"), 0);
	glUniform3fv (glGetUniformLocation (m_shaderProg, "effectStrength"), 1, reinterpret_cast<GLfloat*> (&effectStrength));
	ogl.SetLighting (true);
	}

CFloatVector3 f;
f.Assign (p [0]);
f.v.coord.z = X2F (d / n);
glEnable (GL_LIGHT0 + m_nShockwaves);
glLightfv (GL_LIGHT0 + m_nShockwaves, GL_POSITION, reinterpret_cast<GLfloat*> (&f));
glLightf (GL_LIGHT0 + m_nShockwaves, GL_QUADRATIC_ATTENUATION, ttl);
glUniform1i (glGetUniformLocation (m_shaderProg, "nShockWaves"), ++m_nShockwaves);
return true;
}

//------------------------------------------------------------------------------

void CPostEffectShockwave::Update (void)
{
LoadShader (m_pos, m_nSize, float (m_nLife) / float (SDL_GetTicks () - m_nStart));
}

//------------------------------------------------------------------------------

void CPostEffectShockwave::Render (void)
{
// render current render target
OglDrawArrays (GL_QUADS, 0, 4);
if (m_nShockwaves > 0) {
	for (int i = 0; i < m_nShockwaves; i++)
		glDisable (GL_LIGHT0 + i);
	ogl.EnableLighting (false);
	m_nShockwaves = 0;
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

