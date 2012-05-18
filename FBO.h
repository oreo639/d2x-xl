#ifndef __FBO_H
#define __FBO_H

#include <windows.h>
#include <stddef.h>

#define MAX_COLOR_BUFFERS 16

typedef struct tFrameBuffer {
	GLuint	hFBO;
	GLuint	hColorBuffers [MAX_COLOR_BUFFERS];
	GLuint	bufferIds [MAX_COLOR_BUFFERS];
	GLuint	hDepthBuffer;
	GLuint	hStencilBuffer;
	int		nColorBuffers;
	int		nFirstBuffer;
	int		nBufferCount;
	int		nBuffer;
	int		nType;
	int		nWidth;
	int		nHeight;
	int		bActive;
	GLenum	nStatus;
} tFrameBuffer;

class CFBO {
	private:
		tFrameBuffer	m_info;

	public:
		CFBO () { Init (); }

		~CFBO () { Destroy (); }

		static void Setup (void);

		void Init (void);

		int Create (int nWidth, int nHeight, int nType, int nColorBuffers = 1);

		void Destroy (void);

		int Available (void);

		int Enable (int nColorBuffers = 0);

		int Disable (void);

		inline int GetType (void) { return m_info.nType; }

		inline void SetType (int nType) { m_info.nType = nType; }

		inline int GetWidth (void) { return m_info.nWidth; }

		inline void SetWidth (int nWidth) { m_info.nWidth = nWidth; }

		inline int GetHeight (void) { return m_info.nHeight; }

		inline void SetHeight (int nHeight) { m_info.nHeight = nHeight; }

		inline GLenum GetStatus (void) { return m_info.nStatus; }

		inline void SetStatus (GLenum nStatus) { m_info.nStatus = nStatus; }

		inline int Active (void) { return m_info.bActive; }

		inline GLuint* BufferIds (void) { return m_info.bufferIds + m_info.nFirstBuffer; }

		inline GLuint BufferCount (void) { return m_info.nBufferCount; }

		void SelectColorBuffers (int nBuffer = 0);

		GLuint Handle (void) { return m_info.hFBO; }

		GLuint& ColorBuffer (int i = 0) { return m_info.hColorBuffers [(i < m_info.nColorBuffers) ? i : m_info.nColorBuffers - 1]; }

		GLuint& DepthBuffer (void) { return m_info.hDepthBuffer; }

		GLuint& StencilBuffer (void) { return m_info.hStencilBuffer; }

		void CFBO::Draw (CRect viewport);
		
	private:
		int CreateColorBuffers (int nBuffers);

		int CreateDepthBuffer (void);

		void AttachBuffers (void);
	};

#endif //__FBO_H