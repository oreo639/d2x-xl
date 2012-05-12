#include "stdafx.h"
#include "dle-xp.h"
#include "glew.h"
#include "cstack.h"
#include "shadermanager.h"

//------------------------------------------------------------------------------

int tmShaderProgs [3] = {-1, -1, -1};

const char *texMergeFS [3] = {
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
	"uniform vec3 superTransp;\r\n" \
	"vec4 decalColor, texColor, arrowColor;\r\n" \
	"void main(void)" \
	"{decalColor=texture2D(decalTex,gl_TexCoord [1].xy);\r\n" \
	"if((abs(decalColor.r-superTransp.r)<2.0/255.0)&&(abs(decalColor.g-superTransp.g)<2.0/255.0)&&(abs(decalColor.b-superTransp.b)<2.0/255.0))discard;\r\n" \
	"texColor=texture2D(baseTex,gl_TexCoord [0].xy);\r\n" \
	"arrowColor=texture2D(arrowTex,gl_TexCoord [2].xy);\r\n" \
	"gl_FragColor=mix(vec4(vec3(mix(texColor,decalColor,decalColor.a)),texColor.a+decalColor.a),arrowColor,arrowColor.a*0.8)*gl_Color;\r\n" \
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

for (int i = 0; i < 3; i++) {
	if (shaderManager.Build (tmShaderProgs [i], texMergeFS [i], texMergeVS)) 
		nShaders |= 1 << i;
	else
		tmShaderProgs [i] = -1;
	}
return nShaders;
}

//------------------------------------------------------------------------------

int CTextureManager::DeployShader (int nType)
{
if (tmShaderProgs [nType] < 0)
	return -1;
GLhandleARB shaderProg = GLhandleARB (shaderManager.Deploy (tmShaderProgs [nType]));
if (!shaderProg)
	return -1;
shaderManager.Rebuild (shaderProg);
#if 1
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
#else
glUniform1i (glGetUniformLocation (shaderProg, "baseTex"), 0);
glUniform1i (glGetUniformLocation (shaderProg, "decalTex"), 1);
#endif
return tmShaderProgs [nType];
}

//------------------------------------------------------------------------------

