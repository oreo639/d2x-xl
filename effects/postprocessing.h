#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#define PP_EFFECT_EXPLOSION	1

//------------------------------------------------------------------------------

class CPostEffect {
	private:
		class CPostEffect*	m_prev;
		class CPostEffect*	m_next;
		int						m_nType;

	public:
		CPostEffect (int nType = 0) : 
			m_nType (nType), m_prev (NULL), m_next (NULL) 
			{}

		inline CPostEffect* Prev (void) { return m_prev; }

		inline CPostEffect* Next (void) { return m_next; }

		inline void Setup (int nType) { m_nType = nType; }

		inline void Link (CPostEffect* prev, CPostEffect* next) {
			m_prev = prev, m_next = next;
			}

		inline void Insert (CPostEffect* next) { 
			if (next)
				next->Link (this, m_next);
			if (m_next)
				m_next->Link (next, m_next->Next ());
			m_next = next;
			}

		inline void Unlink (void) {
			if (m_prev)
				m_prev->Link (m_prev->Prev (), m_next);
			if (m_next)
				m_next->Link (m_prev, m_next->Next ());
			m_prev = m_next = NULL;
			}

		virtual bool Terminate (void) = 0;

		virtual void Update (void) = 0;
};

//------------------------------------------------------------------------------

bool CPostEffectExplosion::bRendered;
int CPostEffectExplosion::nExplosions;

class CPostEffectExplosion : public CPostEffect {
	private:
		int	m_nStart;
		int	m_nLife;
		int	m_nSize;
		int	m_x;
		int	m_y;


	public:
		static bool bRendered;
		static int nExplosions;

	public:
		CPostEffectExplosion (int nStart = 0, int nLife = 0, int nSize = 0, int x = 0, int y = 0) :
			CPostEffect (PP_EFFECT_EXPLOSION, next), 
			m_nStart (nStart), m_nLife (nLife), m_nSize (nSize), m_x (x), m_y (y)
			{}

		void Setup (int nStart, int nLife, int nSize, int x, int y) {
			CPostEffect::Setup (PP_EFFECT_EXPLOSION);
			m_nStart = nStart, m_nLife = nLife, m_nSize = nSize, m_x = x, m_y = y;
			}

		virtual bool Terminate (void) { return SDL_GetTicks () - m_nStart >= m_nLife; }

		virtual void Update (void);

		virtual void Render (void);
	};

//------------------------------------------------------------------------------

class CPostProcessManager {
	private:
		int				m_nEffects;
		CPostEffect*	m_effects;

	public:
		CPostProcessManager () { Init (); }

		~CPostProcessManager () { Destroy (); }

		inline void Init (void) { 
			m_nEffects = 0;
			m_effects = NULL;
			}

		void Destroy (void);

		void Add (CPostEffect* e);

		void Update (void);

		inline void CPostEffect* Effects (void) { return m_effects; }
	};


#endif //POSTPROCESS_H
