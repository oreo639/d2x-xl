#include "stdafx.h"
#include "dle-xp.h"
#include "glew.h"
#include "cstack.h"
#include "shadermanager.h"

//------------------------------------------------------------------------------

#define SHADER_COUNT 7

int tmShaderProgs [SHADER_COUNT] = {-1, -1, -1,-1, -1, -1, -1};

const char *texMergeFS [SHADER_COUNT] = {
	"uniform sampler2D baseTex, decalTex;\r\n" \
	"uniform vec3 superTransp;\r\n" \
	"vec4 decalColor, texColor;\r\n" \
	"void main(void)" \
	"{decalColor=texture2D(decalTex,gl_TexCoord [1].xy);\r\n" \
	"if((abs(decalColor.r-superTransp.r)<2.0/255.0)&&(abs(decalColor.g-superTransp.g)<2.0/255.0)&&(abs(decalColor.b-superTransp.b)<2.0/255.0))discard;\r\n" \
	"texColor=texture2D(baseTex,gl_TexCoord [0].xy);\r\n" \
	"gl_FragColor=vec4(vec3(mix(texColor,decalColor,decalColor.a)),min(1.0,texColor.a+decalColor.a))*gl_Color;\r\n" \
   "}"
	,
	"uniform sampler2D baseTex, arrowTex;\r\n" \
	"vec4 texColor, arrowColor;\r\n" \
	"void main(void)" \
	"{texColor=texture2D(baseTex,gl_TexCoord [0].xy);\r\n" \
	"arrowColor=texture2D(arrowTex,gl_TexCoord [1].xy);\r\n" \
	"gl_FragColor=mix(texColor,arrowColor,arrowColor.a*0.8)*gl_Color;\r\n" \
   "}"
	,
	"uniform sampler2D baseTex, decalTex, arrowTex;\r\n" \
	"uniform vec3 sideKey;\r\n" \
	"uniform vec3 superTransp;\r\n" \
	"vec4 decalColor, texColor, arrowColor;\r\n" \
	"void main(void)" \
	"{decalColor=texture2D(decalTex,gl_TexCoord [1].xy);\r\n" \
	"if((abs(decalColor.r-superTransp.r)<2.0/255.0)&&(abs(decalColor.g-superTransp.g)<2.0/255.0)&&(abs(decalColor.b-superTransp.b)<2.0/255.0))discard;\r\n" \
	"texColor=texture2D(baseTex,gl_TexCoord [0].xy);\r\n" \
	"arrowColor=texture2D(arrowTex,gl_TexCoord [2].xy);\r\n" \
	"gl_FragColor=mix(vec4(vec3(mix(texColor,decalColor,decalColor.a)),texColor.a+decalColor.a),arrowColor,arrowColor.a*0.8)*gl_Color;\r\n" \
   "}"
	,
	"uniform sampler2D baseTex;\r\n" \
	"uniform vec3 sideKey;\r\n" \
	"void main(void){" \
	"gl_FragData[0]=texture2D(baseTex,gl_TexCoord [0].xy)*glColor;\r\n" \
	"gl_FragData[1]=vec4 (sideKey, 1.0);\r\n" \
   "}"
	,
	"uniform sampler2D baseTex, decalTex;\r\n" \
	"uniform vec3 sideKey;\r\n" \
	"uniform vec3 superTransp;\r\n" \
	"vec4 decalColor, texColor;\r\n" \
	"void main(void)" \
	"{decalColor=texture2D(decalTex,gl_TexCoord [1].xy);\r\n" \
	"if((abs(decalColor.r-superTransp.r)<2.0/255.0)&&(abs(decalColor.g-superTransp.g)<2.0/255.0)&&(abs(decalColor.b-superTransp.b)<2.0/255.0))discard;\r\n" \
	"texColor=texture2D(baseTex,gl_TexCoord [0].xy);\r\n" \
	"gl_FragData[0]=vec4(vec3(mix(texColor,decalColor,decalColor.a)),min(1.0,texColor.a+decalColor.a))*gl_Color;\r\n" \
	"gl_FragData[1]=vec4 (sideKey, 1.0);\r\n" \
   "}"
	,
	"uniform sampler2D baseTex, arrowTex;\r\n" \
	"uniform vec3 sideKey;\r\n" \
	"vec4 texColor, arrowColor;\r\n" \
	"void main(void)" \
	"{texColor=texture2D(baseTex,gl_TexCoord [0].xy);\r\n" \
	"arrowColor=texture2D(arrowTex,gl_TexCoord [1].xy);\r\n" \
	"gl_FragData[0]=mix(texColor,arrowColor,arrowColor.a*0.8)*gl_Color;\r\n" \
	"gl_FragData[1]=vec4 (sideKey, 1.0);\r\n" \
   "}"
	,
	"uniform sampler2D baseTex, decalTex, arrowTex;\r\n" \
	"uniform vec3 sideKey;\r\n" \
	"uniform vec3 superTransp;\r\n" \
	"vec4 decalColor, texColor, arrowColor;\r\n" \
	"void main(void)" \
	"{decalColor=texture2D(decalTex,gl_TexCoord [1].xy);\r\n" \
	"if((abs(decalColor.r-superTransp.r)<2.0/255.0)&&(abs(decalColor.g-superTransp.g)<2.0/255.0)&&(abs(decalColor.b-superTransp.b)<2.0/255.0))discard;\r\n" \
	"texColor=texture2D(baseTex,gl_TexCoord [0].xy);\r\n" \
	"arrowColor=texture2D(arrowTex,gl_TexCoord [2].xy);\r\n" \
	"gl_FragData[0]=mix(vec4(vec3(mix(texColor,decalColor,decalColor.a)),texColor.a+decalColor.a),arrowColor,arrowColor.a*0.8)*gl_Color;\r\n" \
	"gl_FragData[1]=vec4 (sideKey, 1.0);\r\n" \
   "}"
	};

const char *texMergeVS = {
	"void main(void){" \
	"gl_TexCoord [0]=gl_MultiTexCoord0;"\
	"gl_TexCoord [1]=gl_MultiTexCoord1;"\
	"gl_TexCoord [2]=gl_MultiTexCoord2;"\
	"gl_Position=ftransform();"\
	"gl_FrontColor=gl_Color;}"
	};

//-------------------------------------------------------------------------

int CTextureManager::InitShaders (void)
{
	int nShaders = 0;

for (int i = 0; i < SHADER_COUNT; i++) {
	if (shaderManager.Build (tmShaderProgs [i], texMergeFS [i], texMergeVS)) 
		nShaders |= 1 << i;
	else
		tmShaderProgs [i] = -1;
	}
return nShaders;
}

//------------------------------------------------------------------------------

int CTextureManager::DeployShader (int nType, CFaceListEntry* fle)
{
if (fle)
	nType += 3;
if (tmShaderProgs [nType] < 0)
	return -1;
GLhandleARB shaderProg = GLhandleARB (shaderManager.Deploy (tmShaderProgs [nType]));
if (!shaderProg)
	return -1;
shaderManager.Rebuild (shaderProg);
shaderManager.Set ("baseTex", 0);
if (nType == 0)
	shaderManager.Set ("decalTex", 1);
else if (nType == 1)
	shaderManager.Set ("arrowTex", 1);
else if (nType == 2) {
	shaderManager.Set ("decalTex", 1); 
	shaderManager.Set ("arrowTex", 2);
	}
if (nType != 1) 
	shaderManager.Set ("superTransp", *((vec3*) paletteManager.SuperTranspKeyf ()));
if (fle) {
	vec3 sideKey = {float (fle->m_nSegment / 256) / 255.0f, float (fle->m_nSegment % 256) / 255.0f, float (fle->m_nSide + 1)};
	shaderManager.Set ("sideKey", sideKey);
	}
return tmShaderProgs [nType];
}

//------------------------------------------------------------------------------

