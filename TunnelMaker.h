#ifndef __tunnelmaker_h
#define __tunnelmaker_h

#define MAX_TUNNEL_SEGMENTS	64
#define MAX_TUNNEL_LENGTH		2000
#define MIN_TUNNEL_LENGTH		10
#define TUNNEL_INTERVAL			10

extern char szTunnelMakerError [];

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

class CCubicBezier {
	CDoubleVector	m_points [4];
	double			m_length [2];

	private:
		long Faculty (int n); 

		double Coeff (int n, int i); 

		double Blend (int i, int n, double u); 

	public:
		CCubicBezier () : m_length ({1.0, 1.0}) {}

		CDoubleVector Compute (double u, int nPoints, CDoubleVector* p); 

		inline void SetPoint (CDoubleVector p, int i) { m_points [i] = p; }

		inline CDoubleVector& GetPoint (int i) { return m_points [i]; }

		inline double SetLength (double length, int i) { m_length [i] = length; }

		inline double GetLength (int i) { return fabs (m_length [i]); }

		inline double Length (void) { return GetLength (0) + GetLength (1); }
	};	

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

class CTunnelBase : public CSideKey {
	public:
		CDoubleVector	m_point;
		CDoubleVector	m_normal;

		CTunnelBase (CSideKey key = CSideKey (-1, -1)) : CSideKey (key) { Compute (); }

		CTunnelBase (CDoubleVector point, CDoubleVector normal) : m_point (point), m_normal (normal), CSideKey (-1, -1) {}

		void Compute (void);

		inline CDoubleVector GetPoint (void) { return m_point; }

		inline void SetPoint (CDoubleVector point) { m_point = point; }

		inline CDoubleVector GetNormal (void) { return m_normal; }

		inline void SetNormal (CDoubleVector normal) { m_normal = normal; }
	};

//------------------------------------------------------------------------
// A single segment of a tunnel

class CTunnelElement {
	public:
		CDoubleVector	m_node; // path point (segment center)
		ushort			m_nVertices [4];
		short				m_nSegment;
	};

//------------------------------------------------------------------------

class CTunnelPath {
	private:
		CCubicBezier						m_bezier;

	public:
		short									m_nLength [2]; // current length, previous length [segments]
		CDynamicArray<CTunnelElement>	m_elements;

		void Compute (CTunnelBase base [2]);

		void Remove (int i);

		void Destroy (void);

		inline short MaxSegments (void) {
			short h = SEGMENT_LIMIT - segmentManager.Count ();
			return (h > MAX_TUNNEL_SEGMENTS) ? MAX_TUNNEL_SEGMENTS : h;
			}
	};

//------------------------------------------------------------------------

class CTunnelInfo : public CSideKey {
public:
	double	m_length;

	CSegment _const_ * Segment (void) _const_ { return segmentManager.Segment (m_nSegment); }
	};

//------------------------------------------------------------------------
// A string of connected tunnel segments from a start to an end point 

class CTunnelSegment {
	public:
		CTunnelBase							m_start, m_end;
		CDynamicArray<CTunnelElement>	m_elements;
	};

//------------------------------------------------------------------------

class CTunnelMaker {
	private:
		bool									m_bActive;
		int									m_nMaxSegments;
		CTunnelInfo							m_info [2];
		CDynamicArray<CTunnelSegment>	m_tunnel;

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

		inline int Length (void) { return m_nLength [0]; }

		void ComputeTunnel (void); 

		void Draw (CRenderer& renderer, CPen* redPen, CPen* bluePen, CViewMatrix* view);

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

		void SetupVertices (void);

		void Remove (int l);
	};

extern CTunnelMaker tunnelMaker;

//------------------------------------------------------------------------------

#endif //__tunnelmaker_h