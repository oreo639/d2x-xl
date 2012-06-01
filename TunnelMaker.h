#ifndef __tunnelmaker_h
#define __tunnelmaker_h

#define MAX_TUNNEL_SEGMENTS	64
#define MAX_TUNNEL_LENGTH		2000
#define MIN_TUNNEL_LENGTH		10
#define TUNNEL_INTERVAL			int (10 * DLE.MineView ()->MineMoveRate ())

extern char szTunnelMakerError [];

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

class CCubicBezier {
	CVertex	m_points [4];
	double	m_length [2];

	private:
		long Faculty (int n); 

		double Coeff (int n, int i); 

		double Blend (int i, int n, double u); 

	public:
		CCubicBezier () {m_length [0] = m_length [1] = 1.0; }

		CDoubleVector Compute (double u); 

		inline void SetPoint (CDoubleVector p, int i) { m_points [i] = p; }

		inline CVertex& GetPoint (int i) { return m_points [i]; }

		inline double SetLength (double length, int i) { return m_length [i] = length; }

		inline double GetLength (int i) { return fabs (m_length [i]); }

		inline double Length (void) { return GetLength (0) + GetLength (1); }

		void Transform (CViewMatrix* viewMatrix);

		void Project (CViewMatrix* viewMatrix);
	};	

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

class CTunnelBase : public CSideKey {
	public:
		CVertex	m_point;
		CVertex	m_normal;
		CVertex	m_vertices [4];

		CTunnelBase (CSideKey key = CSideKey (-1, -1)) : CSideKey (key) {}

		CTunnelBase (CDoubleVector point, CDoubleVector normal) : m_point (point), m_normal (normal), CSideKey (-1, -1) {}

		void Setup (void);

		inline CDoubleVector GetPoint (void) { return m_point; }

		inline void SetPoint (CDoubleVector point) { m_point = point; }

		inline CDoubleVector GetNormal (void) { return m_normal; }

		inline void SetNormal (CDoubleVector normal) { m_normal = normal; }

		inline CSegment* Segment (void) { return (m_nSegment < 0) ? null : segmentManager.Segment (m_nSegment); }

		inline CSide* Side (void) { return (m_nSide < 0) ? null : segmentManager.Side (m_nSide); }

	};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
// A single segment of a tunnel

class CTunnelElement {
	public:
		CVertex	m_node; // path point (segment center)
		CVertex	m_relNode; // path point (segment center) relative to start side center
		ushort	m_nVertices [4];
		short		m_nSegment;
	};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
// A string of connected tunnel segments from a start to an end point 

class CTunnelSegment {
	private:
		CCubicBezier*	m_bezier;
		CTunnelBase		m_base [2];

	public:
		short									m_nPathLength; // current path length
		CDynamicArray<CTunnelElement>	m_elements;

		void Setup (CCubicBezier* bezier, CTunnelBase base [2]);

		void Compute (short nPathLength);

		void Realize (void);

		void Release (int i);

		void Destroy (void);

		void Draw (CRenderer& renderer, CPen* redPen, CPen* bluePen, CViewMatrix* viewMatrix);

	private:
		void UntwistSegment (short nSegment, short nSide); 

		void SpinPoint (CVertex* point, double ySpin, double zSpin); 

		void SpinBackPoint (CVertex* point, double ySpin, double zSpin); 

		int MatchingSide (int j); 

		void PolarPoints (double *angle, double *radius, CVertex* vertex, CVertex* origin, CVertex* normal); 

		CDoubleVector RectPoints (double angle, double radius, CVertex* origin, CVertex* normal);

		void SetupVertices (void);

	};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

class CPathBase {
	public:
		CVertex	m_pos;
		CVertex	m_normal;
		ushort	m_nId;
		double	m_length;
	};

//------------------------------------------------------------------------

class CTunnelPath {
	public:
		CCubicBezier				m_bezier;
		CPathBase					m_base [2];
		ushort						m_nId;
		ushort						m_nEnd;
		short							m_nPathLength;
		CDynamicArray<ushort>	m_vertices;

		CTunnelPath () {}

		void CTunnelPath::Setup (CPathBase base [2]);

		void Release (int l);

		bool Create (short nPathLength);
	};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

class CTunnelMaker {
	private:
		bool									m_bActive;
		CCubicBezier						m_bezier;
		short									m_nPathLength;
		short									m_nGranularity;
		CTunnelBase							m_base [2];
		CDynamicArray<CTunnelSegment>	m_segments;
		CDynamicArray<CTunnelPath>		m_paths;

	public:
		CTunnelMaker () : m_nPathLength (0) {}

		void Create (void); 

		void Setup (void);
		
		bool Active (bool bMsg = true) { 
			if (!m_bActive)
				return false; 
			if (bMsg)
				ErrorMsg (szTunnelMakerError); 
			return true;
			}

		void Coarser (void);

		void Finer (void);

		void Stretch (void); 

		void Shrink (void); 

		void Realize (void);

		void Destroy (void);

		short PathLength (void);

		void ComputeTunnel (void); 

		void Draw (CRenderer& renderer, CPen* redPen, CPen* bluePen, CViewMatrix* viewMatrix);

	private:
		CDoubleVector RectPoints (double angle, double radius, CVertex* origin, CVertex* normal); 

		inline short MaxSegments (void) {
			short h = SEGMENT_LIMIT - segmentManager.Count ();
			return (h > MAX_TUNNEL_SEGMENTS) ? MAX_TUNNEL_SEGMENTS : h;
			}

		void Update (void);
	};

extern CTunnelMaker tunnelMaker;

//------------------------------------------------------------------------------

#endif //__tunnelmaker_h