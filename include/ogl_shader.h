#ifndef _OGL_SHADER_H
#define _OGL_SHADER_H

#ifdef _WIN32
#include <windows.h>
#include <stddef.h>
#endif

extern GLhandleARB	genShaderProg;

#if 0
char *LoadShader (char* fileName);
int CreateShaderProg (GLhandleARB *shaderProgP);
int CreateShaderFunc (GLhandleARB *shaderProgP, GLhandleARB *fsP, GLhandleARB *vsP, 
		const char *fsName, const char *vsName, int bFromFile);
int LinkShaderProg (GLhandleARB *shaderProgP);
void DeleteShaderProg (GLhandleARB *shaderProgP);
void InitShaders (void);
#endif

void OglInitShaders (void);

typedef struct tShaderData {
	GLhandleARB		shaders [2];
	GLhandleARB		program;
	int*				refP;
} tShaderData;

class CShaderManager {
	private:
		CStack<tShaderData>		m_shaders;
		int							m_nCurrent;

	public:
		CShaderManager ();
		~CShaderManager ();
		void Init (void);
		void Destroy (bool bAll = true);
		void Setup (void);
		int64_t Deploy (int nShader);
		int Alloc (int& nShader);
		char* Load (const char* filename);
		int Create (int nShader);
		int Compile (int nShader, const char* pszFragShader, const char* pszVertShader, bool bFromFile = 0);
		int Link (int nShader);
		int Build (int& nShader, const char* pszFragShader, const char* pszVertShader, bool bFromFile = 0);
		void Reset (int nShader);
		void Delete (int nShader);
		inline int Current (void) { return m_nCurrent; }
		inline bool IsCurrent (int nShader) { return m_nCurrent == nShader; }
		inline bool Rebuild (GLhandleARB& nShaderProg) {
			if (sizeof (GLhandleARB) == 4) {
				if (0 < int32_t (nShaderProg))
					return true;
				nShaderProg = GLhandleARB (-(int32_t (nShaderProg)));
				return false;
				}
			else {
				if (0 < int64_t (nShaderProg))
					return true;
				nShaderProg = GLhandleARB (-(int64_t (nShaderProg)));
				return false;
				}
			}

	private:
		void Dispose (GLhandleARB& shaderProg);
		void PrintLog (GLhandleARB handle, int bProgram);
};

extern CShaderManager shaderManager;

#endif
