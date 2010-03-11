#ifdef HAVE_CONFIG_H
#	include <conf.h>
#endif

#include <math.h>
#include <stdio.h>
#include <string.h>	// for memset ()

#include "descent.h"
#include "error.h"
#include "u_mem.h"
#include "fix.h"
#include "vecmat.h"
#include "network.h"
#include "ogl_defs.h"
#include "ogl_lib.h"
#include "ogl_tmu.h"
#include "ogl_color.h"
#include "ogl_shader.h"
#include "ogl_render.h"
#include "gameseg.h"
#include "endlevel.h"
#include "renderthreads.h"
#include "light.h"
#include "lightmap.h"
#include "headlight.h"
#include "dynlight.h"

#define ONLY_LIGHTMAPS 0

#define PPL_AMBIENT_LIGHT	0.3f
#define PPL_DIFFUSE_LIGHT	0.7f

#if 1
#define GEO_LIN_ATT	(/*0.0*/ gameData.render.fAttScale [0])
#define GEO_QUAD_ATT	(/*0.003333f*/ gameData.render.fAttScale [1])
#define OBJ_LIN_ATT	(/*0.0*/ gameData.render.fAttScale [0])
#define OBJ_QUAD_ATT	(/*0.003333f*/ gameData.render.fAttScale [1])
#else
#define GEO_LIN_ATT	0.05f
#define GEO_QUAD_ATT	0.005f
#define OBJ_LIN_ATT	0.05f
#define OBJ_QUAD_ATT	0.005f
#endif

//-------------------------------------------------------------------------
// per pixel lighting
// 2 - 8 light sources

const char *pszPPXLightingFS [] = {
	"#define LIGHTS 8\r\n" \
	"uniform int nLights;\r\n" \
	"varying vec3 normal, vertPos;\r\n" \
	"void main() {\r\n" \
	"	vec4 colorSum = vec4 (0.0, 0.0, 0.0, 1.0);\r\n" \
	"	vec3 vertNorm = normalize (normal);\r\n" \
	"	int i;\r\n" \
	"	for (i = 0; i < LIGHTS; i++) if (i < nLights) {\r\n" \
	"		vec4 color;\r\n" \
	"		vec3 lightVec = vec3 (gl_LightSource [i].position) - vertPos;\r\n" \
	"		float lightDist = length (lightVec);\r\n" \
	"		vec3 lightNorm = lightVec / lightDist;\r\n" \
	"     /*float bDirected = length (gl_LightSource [i].spotDirection);*/\r\n" \
	"     float NdotL = (/*bDirected **/ lightDist > 0.1) ? dot (lightNorm, vertNorm) : 1.0;\r\n" \
	"		float lightAngle = /*bDirected **/ ((lightDist > 0.01) ? -dot (lightNorm, gl_LightSource [i].spotDirection) + 0.01 : 1.0);\r\n" \
	"		float lightRad = gl_LightSource [i].specular.a * (1.0 - abs (lightAngle));\r\n" \
	"	   lightDist -= 100.0 * /*bDirected **/ min (0.0, min (NdotL, lightAngle)); //bDirected\r\n" \
	"		float att = 1.0, dist = max (lightDist - lightRad, 0.0);\r\n" \
	"	   if (dist == 0.0)\r\n" \
	"			color = gl_LightSource [i].diffuse + gl_LightSource [i].ambient;\r\n" \
	"		else {\r\n" \
	"			att += gl_LightSource [i].linearAttenuation * dist + gl_LightSource [i].quadraticAttenuation * dist * dist;\r\n" \
	"			/*specular highlight >>>>>*/\r\n" \
	"			if (NdotL >= 0.0) {\r\n" \
	"				vec3 halfV = normalize (gl_LightSource [i].halfVector.xyz);\r\n" \
	"				float NdotHV = max (dot (vertNorm, halfV), 0.0);\r\n" \
	"				color += (gl_LightSource [i].specular * pow (NdotHV, 16.0)) / att;\r\n" \
	"				}/*<<<<< specular highlight*/\r\n" \
	"			NdotL = max (NdotL, 0.0);\r\n" \
	"			if (lightRad > 0.0)\r\n" \
	"				NdotL += (1.0 - NdotL) / att;\r\n" \
	"			color = (gl_LightSource [i].diffuse * NdotL + gl_LightSource [i].ambient) / att;\r\n" \
	"			}\r\n" \
	"		colorSum += color * gl_LightSource [i].constantAttenuation;\r\n" \
	"		}\r\n" \
	"	gl_FragColor = colorSum;\r\n" \
	"	}"
	,
	"#define LIGHTS 8\r\n" \
	"uniform sampler2D maskTex;\r\n" \
	"uniform int nLights;\r\n" \
	"varying vec3 normal, vertPos;\r\n" \
	"void main() {\r\n" \
	"if (texture2D (maskTex, gl_TexCoord [2].xy).r < 0.5)\r\n" \
	"  discard;\r\n" \
	"else {\r\n" \
   "	vec4 colorSum = vec4 (0.0, 0.0, 0.0, 1.0);\r\n" \
	"	vec3 vertNorm = normalize (normal);\r\n" \
	"	int i;\r\n" \
	"	for (i = 0; i < LIGHTS; i++) if (i < nLights) {\r\n" \
	"		vec4 color;\r\n" \
	"		vec3 lightVec = vec3 (gl_LightSource [i].position) - vertPos;\r\n" \
	"		float lightDist = length (lightVec);\r\n" \
	"		vec3 lightNorm = lightVec / lightDist;\r\n" \
	"     /*float bDirected = length (gl_LightSource [i].spotDirection);*/\r\n" \
	"     float NdotL = (/*bDirected **/ lightDist > 0.1) ? dot (lightNorm, vertNorm) : 1.0;\r\n" \
	"		float lightAngle = /*bDirected **/ ((lightDist > 0.01) ? -dot (lightNorm, gl_LightSource [i].spotDirection) + 0.01 : 1.0);\r\n" \
	"		float lightRad = gl_LightSource [i].specular.a * (1.0 - abs (lightAngle));\r\n" \
	"	   lightDist -= 100.0 * /*bDirected **/ min (0.0, min (NdotL, lightAngle)); //bDirected\r\n" \
	"		float att = 1.0, dist = max (lightDist - lightRad, 0.0);\r\n" \
	"	   if (dist == 0.0)\r\n" \
	"			color = gl_LightSource [i].diffuse + gl_LightSource [i].ambient;\r\n" \
	"		else {\r\n" \
	"			att += gl_LightSource [i].linearAttenuation * dist + gl_LightSource [i].quadraticAttenuation * dist * dist;\r\n" \
	"			/*specular highlight >>>>>*/\r\n" \
	"			if (NdotL >= 0.0) {\r\n" \
	"				vec3 halfV = normalize (gl_LightSource [i].halfVector.xyz);\r\n" \
	"				float NdotHV = max (dot (vertNorm, halfV), 0.0);\r\n" \
	"				color += (gl_LightSource [i].specular * pow (NdotHV, 16.0)) / att;\r\n" \
	"				}/*<<<<< specular highlight*/\r\n" \
	"			NdotL = max (NdotL, 0.0);\r\n" \
	"			if (lightRad > 0.0)\r\n" \
	"				NdotL += (1.0 - NdotL) / att;\r\n" \
	"			color = (gl_LightSource [i].diffuse * NdotL + gl_LightSource [i].ambient) / att;\r\n" \
	"			}\r\n" \
	"		colorSum += color * gl_LightSource [i].constantAttenuation;\r\n" \
	"		}\r\n" \
	"	gl_FragColor = colorSum;\r\n" \
	"	}\r\n" \
	"}"
	};

//-------------------------------------------------------------------------
// one light source

const char *pszPP1LightingFS [] = {
	"uniform int nLights;\r\n" \
	"varying vec3 normal, vertPos;\r\n" \
	"void main() {\r\n" \
	"	vec4 colorSum;\r\n" \
	"	vec3 vertNorm = normalize (normal);\r\n" \
	"	vec3 lightVec = vec3 (gl_LightSource [0].position) - vertPos;\r\n" \
	"	float lightDist = length (lightVec);\r\n" \
	"	vec3 lightNorm = lightVec / lightDist;\r\n" \
	"  /*float bDirected = length (gl_LightSource [i].spotDirection);*/\r\n" \
	"  float NdotL = (/*bDirected **/ lightDist > 0.1) ? dot (lightNorm, vertNorm) : 1.0;\r\n" \
	"	float lightAngle = /*bDirected **/ min (NdotL, -dot (lightNorm, gl_LightSource [0].spotDirection));\r\n" \
	"	float lightRad = gl_LightSource [i].specular.a * (1.0 - abs (lightAngle));\r\n" \
	"	lightDist -= 100.0 * /*bDirected **/ min (0.0, min (NdotL, lightAngle)); //bDirected\r\n" \
	"	float att = 1.0, dist = max (lightDist - lightRad, 0.0);\r\n" \
	"	if (dist == 0.0)\r\n" \
	"		colorSum = gl_LightSource [0].diffuse + gl_LightSource [0].ambient;\r\n" \
	"	else {\r\n" \
	"		att += gl_LightSource [0].linearAttenuation * dist + gl_LightSource [0].quadraticAttenuation * dist * dist;\r\n" \
	"		/*specular highlight >>>>>*/\r\n" \
	"		if (NdotL >= 0.0) {\r\n" \
	"			vec3 halfV = normalize (gl_LightSource [i].halfVector.xyz);\r\n" \
	"			float NdotHV = max (dot (vertNorm, halfV), 0.0);\r\n" \
	"			colorSum += (gl_LightSource [i].specular * pow (NdotHV, 16.0)) / att;\r\n" \
	"			}/*<<<<< specular highlight*/\r\n" \
	"		NdotL = max (NdotL, 0.0);\r\n" \
	"		if (lightRad > 0.0)\r\n" \
	"			NdotL += (1.0 - NdotL) / att;\r\n" \
	"		colorSum = (gl_LightSource [0].diffuse * NdotL + gl_LightSource [0].ambient) / att;\r\n" \
	"		}\r\n" \
	"	gl_FragColor = colorSum;\r\n" \
	"	}"
	,
	"uniform int nLights;\r\n" \
	"varying vec3 normal, vertPos;\r\n" \
	"void main() {\r\n" \
	"if (texture2D (maskTex, gl_TexCoord [2].xy).r < 0.5)\r\n" \
	"  discard;\r\n" \
	"else {\r\n" \
	"	vec4 colorSum;\r\n" \
	"	vec3 vertNorm = normalize (normal);\r\n" \
	"	vec3 lightVec = vec3 (gl_LightSource [0].position) - vertPos;\r\n" \
	"	float lightDist = length (lightVec);\r\n" \
	"	vec3 lightNorm = lightVec / lightDist;\r\n" \
	"  /*float bDirected = length (gl_LightSource [i].spotDirection);*/\r\n" \
	"  float NdotL = (/*bDirected **/ lightDist > 0.1) ? dot (lightNorm, vertNorm) : 1.0;\r\n" \
	"	float lightAngle = /*bDirected **/ min (NdotL, -dot (lightNorm, gl_LightSource [0].spotDirection));\r\n" \
	"	float lightRad = gl_LightSource [i].specular.a * (1.0 - abs (lightAngle));\r\n" \
	"	lightDist -= 100.0 * /*bDirected **/ min (0.0, min (NdotL, lightAngle)); //bDirected\r\n" \
	"	float att = 1.0, dist = max (lightDist - lightRad, 0.0);\r\n" \
	"	if (dist == 0.0)\r\n" \
	"		colorSum = gl_LightSource [0].diffuse + gl_LightSource [0].ambient;\r\n" \
	"	else {\r\n" \
	"		att += gl_LightSource [0].linearAttenuation * dist + gl_LightSource [0].quadraticAttenuation * dist * dist;\r\n" \
	"		/*specular highlight >>>>>*/\r\n" \
	"		if (NdotL >= 0.0) {\r\n" \
	"			vec3 halfV = normalize (gl_LightSource [i].halfVector.xyz);\r\n" \
	"			float NdotHV = max (dot (vertNorm, halfV), 0.0);\r\n" \
	"			colorSum += (gl_LightSource [i].specular * pow (NdotHV, 16.0)) / att;\r\n" \
	"			}/*<<<<< specular highlight*/\r\n" \
	"		NdotL = max (NdotL, 0.0);\r\n" \
	"		if (lightRad > 0.0)\r\n" \
	"			NdotL += (1.0 - NdotL) / att;\r\n" \
	"		colorSum = (gl_LightSource [0].diffuse * NdotL + gl_LightSource [0].ambient) / att;\r\n" \
	"		}\r\n" \
	"	gl_FragColor = colorSum;\r\n" \
	"	}\r\n" \
	"}"

	};

//-------------------------------------------------------------------------

const char *pszPPLightingVS [] = {
	"#define LIGHTS 8\r\n" \
	"varying vec3 normal, vertPos;\r\n" \
	"void main() {\r\n" \
	"	normal = normalize (gl_NormalMatrix * gl_Normal);\r\n" \
	"	vertPos = vec3 (gl_ModelViewMatrix * gl_Vertex);\r\n" \
	"	gl_Position = ftransform();\r\n" \
   "	gl_FrontColor = gl_Color;\r\n" \
	"	}"
	,
	"#define LIGHTS 8\r\n" \
	"varying vec3 normal, vertPos;\r\n" \
	"void main() {\r\n" \
	"	normal = normalize (gl_NormalMatrix * gl_Normal);\r\n" \
	"	vertPos = vec3 (gl_ModelViewMatrix * gl_Vertex);\r\n" \
	"	gl_Position = ftransform();\r\n" \
	"	gl_TexCoord [0] = gl_MultiTexCoord0;\r\n" \
   "	gl_FrontColor = gl_Color;\r\n" \
	"	}"
	};

//-------------------------------------------------------------------------

char *BuildLightingShader (const char *pszTemplate, int nLights)
{
	int	l = (int) strlen (pszTemplate) + 1;
	char	*pszFS, szLights [2];

if (!(pszFS = new char [l]))
	return NULL;
if (nLights > MAX_LIGHTS_PER_PIXEL)
	nLights = MAX_LIGHTS_PER_PIXEL;
memcpy (pszFS, pszTemplate, l);
if (strstr (pszFS, "#define LIGHTS ") == pszFS) {
	sprintf (szLights, "%d", nLights);
	pszFS [15] = *szLights;
	}
#if DBG
PrintLog (" \n\nShader program:\n");
PrintLog (pszFS);
PrintLog (" \n");
#endif
return pszFS;
}

//-------------------------------------------------------------------------

int perPixelLightingShaderProgs [9][2] = {
	{-1,-1},
	{-1,-1},
	{-1,-1},
	{-1,-1},
	{-1,-1},
	{-1,-1},
	{-1,-1},
	{-1,-1},
	{-1,-1}
};

int CreatePerPixelLightingShader (int nType, int nLights)
{
	int	h, i, j, bOk;
	char	*pszFS, *pszVS;
	const char	**fsP, **vsP;

if (!(ogl.m_states.bShadersOk && gameStates.render.bUsePerPixelLighting && (ogl.m_states.bPerPixelLightingOk == 2)))
	ogl.m_states.bPerPixelLightingOk =
	gameStates.render.bPerPixelLighting = 0;
if (!gameStates.render.bPerPixelLighting)
	return -1;
nLights = gameStates.render.nMaxLightsPerPass;
if (perPixelLightingShaderProgs [nLights][nType] >= 0)
	return nLights;
for (h = 0; h <= 1; h++) {
	i = gameStates.render.nMaxLightsPerPass;
	if (perPixelLightingShaderProgs [i][h] >= 0)
		continue;
	fsP = (i == 1) ? pszPP1LightingFS : pszPPXLightingFS;
	vsP = pszPPLightingVS;
	PrintLog ("building lighting shader programs\n");
	pszFS = BuildLightingShader (fsP [h], i);
	pszVS = BuildLightingShader (vsP [h], i);
	bOk = (pszFS != NULL) && (pszVS != NULL) && shaderManager.Build (perPixelLightingShaderProgs [i][h], pszFS, pszVS);
	delete[] pszFS;
	delete[] pszVS;
	if (!bOk) {
		ogl.m_states.bPerPixelLightingOk = 1;
		gameStates.render.bPerPixelLighting = 1;
		for (i = 0; i <= MAX_LIGHTS_PER_PIXEL; i++)
			for (j = 0; j < 4; j++)
				shaderManager.Delete (perPixelLightingShaderProgs [i][j]);
		nLights = 0;
		return -1;
		}
	}
return ogl.m_data.nPerPixelLights [nType] = nLights;
}

// ----------------------------------------------------------------------------------------------

void ResetPerPixelLightingShaders (void)
{
//memset (perPixelLightingShaderProgs, 0xFF, sizeof (perPixelLightingShaderProgs));
}

// ----------------------------------------------------------------------------------------------

void InitPerPixelLightingShaders (void)
{
for (int nType = 0; nType < 4; nType++)
	CreatePerPixelLightingShader (nType, gameStates.render.nMaxLightsPerPass);
}

//------------------------------------------------------------------------------

#if DBG
int CheckUsedLights2 (void);
#endif

int SetupHardwareLighting (CSegFace *faceP, int nType)
{
PROF_START
	int					nLightRange, nLights;
	float					fBrightness;
	tRgbaColorf			ambient, diffuse;
#if 0
	tRgbaColorf			black = {0,0,0,0};
#endif
	tRgbaColorf			specular = {0.5f,0.5f,0.5f,0.5f};
	//CFloatVector			vPos = CFloatVector::Create(0,0,0,1);
	GLenum				hLight;
	CActiveDynLight*	activeLightsP;
	CDynLight*			psl;
	CDynLightIndex*	sliP = &lightManager.Index (0)[0];

#if DBG
if (faceP && (faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
if (faceP - FACES.faces == nDbgFace)
	nDbgFace = nDbgFace;
#endif
if (!ogl.m_states.iLight) {
#if DBG
	if (faceP && (faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
		nDbgSeg = nDbgSeg;
	if (faceP - FACES.faces == nDbgFace)
		nDbgFace = nDbgFace;
#endif
#if ONLY_LIGHTMAPS == 2
	ogl.m_states.nLights = 0;
#else
	ogl.m_states.nLights = lightManager.SetNearestToFace (faceP, nType != 0);
#endif
	if (ogl.m_states.nLights > gameStates.render.nMaxLightsPerFace)
		ogl.m_states.nLights = gameStates.render.nMaxLightsPerFace;
	ogl.m_states.nFirstLight = sliP->nFirst;
	gameData.render.nTotalLights += ogl.m_states.nLights;
	if (gameData.render.nMaxLights < ogl.m_states.nLights)
		gameData.render.nMaxLights = ogl.m_states.nLights;
#if DBG
	if (faceP && (faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
		HUDMessage (0, "%d lights", ogl.m_states.nLights);
#endif
	}
activeLightsP = lightManager.Active (0) + ogl.m_states.nFirstLight;
nLightRange = sliP->nLast - ogl.m_states.nFirstLight + 1;
for (nLights = 0;
	  (ogl.m_states.iLight < ogl.m_states.nLights) & (nLightRange > 0) && (nLights < gameStates.render.nMaxLightsPerPass);
	  activeLightsP++, nLightRange--) {
	if (!(psl = lightManager.GetActive (activeLightsP, 0)))
		continue;
#if 0 //DBG
	if (psl->info.nObject < 1)
		continue;
	if (OBJECTS [psl->info.nObject].info.nType != OBJ_LIGHT)
		continue;
#endif
#if 0//DBG
	if (faceP && (faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
		nDbgSeg = nDbgSeg;
	if (faceP - FACES == nDbgFace)
		nDbgFace = nDbgFace;
	if ((psl->nTarget < 0) && (-psl->nTarget - 1 != faceP->m_info.nSegment))
		faceP = faceP;
	else if ((psl->nTarget > 0) && (psl->nTarget != faceP - FACES + 1))
		faceP = faceP;
	if (!psl->nTarget)
		psl = psl;
#endif
	if (psl->render.bUsed [0] == 2) {//nearest vertex light
		lightManager.ResetUsed (psl, 0);
		sliP->nActive--;
		}
	hLight = GL_LIGHT0 + nLights++;
	//glEnable (hLight);
	specular.alpha = (psl->info.nSegment >= 0) ? psl->info.fRad : psl->info.fRad * psl->info.fBoost; //krasser Missbrauch!
	fBrightness = psl->info.fBrightness;
#if 0//DBG
	if ((psl->info.nObject >= 0) && (OBJECTS [psl->info.nObject].nType == nDbgObjType) &&
		 ((nDbgObjId < 0) || (OBJECTS [psl->info.nObject].id == nDbgObjId)))
		nDbgObjType = nDbgObjType;
#endif
	if (psl->info.nType == 2) {
		glLightf (hLight, GL_CONSTANT_ATTENUATION, 1.0f);
		glLightf (hLight, GL_LINEAR_ATTENUATION, OBJ_LIN_ATT / fBrightness);
		glLightf (hLight, GL_QUADRATIC_ATTENUATION, OBJ_QUAD_ATT / fBrightness);
#if 1
		glLightfv (hLight, GL_SPOT_DIRECTION, (GLfloat*) (&CFloatVector3::ZERO));
#else
		if (!faceP)
			glLightfv (hLight, GL_SPOT_DIRECTION, (GLfloat*) (&CFloatVector3::ZERO));
		else {
			CFloatVector vNormal = -(faceP->Normal ());
			glLightfv (hLight, GL_SPOT_DIRECTION, reinterpret_cast<GLfloat*> (&vNormal));
			}
#endif
		ambient.red = psl->info.color.red * PPL_AMBIENT_LIGHT;
		ambient.green = psl->info.color.green * PPL_AMBIENT_LIGHT;
		ambient.blue = psl->info.color.blue * PPL_AMBIENT_LIGHT;
		ambient.alpha = 1.0f;
		diffuse.red = psl->info.color.red * PPL_DIFFUSE_LIGHT;
		diffuse.green = psl->info.color.green * PPL_DIFFUSE_LIGHT;
		diffuse.blue = psl->info.color.blue * PPL_DIFFUSE_LIGHT;
		diffuse.alpha = 1.0f;
		}
	else {
		glLightf (hLight, GL_CONSTANT_ATTENUATION, 1.0f);
		glLightf (hLight, GL_LINEAR_ATTENUATION, GEO_LIN_ATT / fBrightness);
		glLightf (hLight, GL_QUADRATIC_ATTENUATION, GEO_QUAD_ATT / fBrightness);
		glLightfv (hLight, GL_SPOT_DIRECTION, reinterpret_cast<GLfloat*> (&psl->info.vDirf));
		ambient.red = psl->info.color.red * PPL_AMBIENT_LIGHT;
		ambient.green = psl->info.color.green * PPL_AMBIENT_LIGHT;
		ambient.blue = psl->info.color.blue * PPL_AMBIENT_LIGHT;
		ambient.alpha = 1.0f;
		fBrightness = min (fBrightness, 1.0f) * PPL_DIFFUSE_LIGHT;
		diffuse.red = psl->info.color.red * fBrightness;
		diffuse.green = psl->info.color.green * fBrightness;
		diffuse.blue = psl->info.color.blue * fBrightness;
		diffuse.alpha = 1.0f;
		}
	glLightfv (hLight, GL_DIFFUSE, reinterpret_cast<GLfloat*> (&diffuse));
	glLightfv (hLight, GL_SPECULAR, reinterpret_cast<GLfloat*> (&specular));
	glLightfv (hLight, GL_AMBIENT, reinterpret_cast<GLfloat*> (&ambient));
	glLightfv (hLight, GL_POSITION, reinterpret_cast<GLfloat*> (psl->render.vPosf));
	ogl.m_states.iLight++;
	}
if (nLightRange <= 0) {
	ogl.m_states.iLight = ogl.m_states.nLights;
	}
ogl.m_states.nFirstLight = int (activeLightsP - lightManager.Active (0));
#if DBG
if ((ogl.m_states.iLight < ogl.m_states.nLights) && !nLightRange)
	nDbgSeg = nDbgSeg;
#endif
#if 0
if (CreatePerPixelLightingShader (nType, nLights) >= 0)
#endif
	{
	PROF_END(ptPerPixelLighting)
	return nLights;
	}
ogl.DisableLighting ();
ogl.ClearError (0);
PROF_END(ptPerPixelLighting)
return -1;
}

//------------------------------------------------------------------------------

int SetupPerPixelLightingShader (CSegFace *faceP, int nType)
{
PROF_START
#if DBG
if (faceP && (faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
#endif

int nLights = SetupHardwareLighting (faceP, nType);
if (0 > nLights) {
	PROF_END(ptShaderStates)
	return 0;
	}
GLhandleARB shaderProg = GLhandleARB (shaderManager.Deploy (perPixelLightingShaderProgs [gameStates.render.nMaxLightsPerPass][nType]));
if (!shaderProg) {
	PROF_END(ptShaderStates);
	return -1;
	}
shaderManager.Rebuild (shaderProg);
if (nType)
	glUniform1i (glGetUniformLocation (shaderProg, "maskTex"), 0);
glUniform1i (glGetUniformLocation (shaderProg, "nLights"), GLint (nLights));
ogl.ClearError (0);
PROF_END(ptShaderStates)
return perPixelLightingShaderProgs [gameStates.render.nMaxLightsPerPass][nType];
}

// ----------------------------------------------------------------------------------------------
//eof
