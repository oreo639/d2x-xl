#ifndef __tunnelmaker_h
#define __tunnelmaker_h


#define MAX_TUNNEL_SEGMENTS	64
#define MAX_TUNNEL_LENGTH		2000
#define MIN_TUNNEL_LENGTH		10
#define TUNNEL_INTERVAL			10

extern char szTunnelMakerError [];

//------------------------------------------------------------------------

class CTunnelInfo : public CSideKey {
public:
	double	m_length;

	CSegment _const_ * Segment (void) _const_ { return segmentManager.Segment (m_nSegment); }
	};

//------------------------------------------------------------------------

class CTunnelMaker {
	private:
		bool		m_bActive;
		int		m_nMaxSegments;
		CVertex	m_points [MAX_TUNNEL_SEGMENTS];
		short		m_nSegments [MAX_TUNNEL_SEGMENTS];
		ushort	m_nVertices [MAX_TUNNEL_SEGMENTS * 4];
		short		m_nLength;

		CTunnelInfo	m_info [2];

	public:
		void Create (void); 

		bool Active (bool bMsg = true) { 
			if (!m_bActive)
				return false; 
			if (bMsg)
				ErrorMsg (szTunnelMakerError); 
			return true;
			}

		void Stretch (void); 

		void Shrink (void); 

		void Realize (void);

		void Destroy (void);

		inline int Length (void) { return m_nLength; }

		void ComputeTunnel (void); 

		void Draw (CDC* pDC, CPen* redPen, CPen* bluePen, CViewMatrix& view);

	private:
		long Faculty (int n); 

		double Coeff (int n, int i); 

		double Blend (int i, int n, double u); 

		CVertex BezierFcn (double u, int npts, CVertex* p); 

		void UntwistSegment (short nSegment, short nSide); 

		void SpinPoint (CVertex* point, double ySpin, double zSpin); 

		void SpinBackPoint (CVertex* point, double ySpin, double zSpin); 

		int MatchingSide (int j); 

		CDoubleVector RectPoints (double angle, double radius, CVertex* origin, CVertex* normal); 

		void PolarPoints (double *angle, double *radius, CVertex* vertex, CVertex* origin, CVertex* normal); 
	};

extern CTunnelMaker tunnelMaker;

//------------------------------------------------------------------------------

#endif //__tunnelmaker_h