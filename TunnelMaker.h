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
		CVertex			m_point;
		CVertex			m_normal;
		CVertex			m_vertices [4];
		CDoubleMatrix	m_orientation; // orientation of tunnel end side

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

class CTunnelPath {
	public:
		CCubicBezier				m_bezier;
		CTunnelBase					m_base [2];
		short							m_nSteps;
		CDynamicArray<CVertex>	m_vertices;
		CDoubleMatrix				m_unRotate; // for unrotating base vertices
		CDoubleVector				m_rotAxis; // rotation angle and axis for transforming the start point orientation to the end point orientation
		double						m_rotAngle;

		CTunnelPath () : m_nSteps (0) {}

		bool Setup (CTunnelBase base [2]);

		void Destroy (void);

		void Release (void);

		bool Create (short nPathLength);

		void Draw (CRenderer& renderer, CPen* redPen, CPen* bluePen, CViewMatrix* viewMatrix);

		inline short Steps (void) { return m_nSteps; }

		double Length (int nSteps = -1);

		inline double Scale (int nStep) { return Length (nStep) / Length (); }

		inline CCubicBezier& Bezier () { return m_bezier; }

		inline CVertex& operator[] (uint i) { return m_vertices [i]; }
	};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
// A string of connected tunnel segments from a start to an end point 

class CTunnelSegment {
	public:
		short									m_nSteps; // current path length
		CTunnelBase							m_base [2];
		CDynamicArray<CTunnelElement>	m_elements;

		void Setup (CTunnelBase base [2]);

		bool Create (CTunnelPath& path);

		void Realize (void);

		void Release (void);

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

class CTunnelMaker {
	private:
		bool									m_bActive;
		short									m_nSteps;
		short									m_nGranularity;
		CTunnelBase							m_base [2];
		CDynamicArray<CTunnelSegment>	m_segments;
		CTunnelPath							m_path;

	public:
		CTunnelMaker () : m_nSteps (0) {}

		void Run (void); 

		void Destroy (void);

		bool Setup (void);
		
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

		void Reset (void);

		short PathLength (void);

		bool Create (void); 

		void Draw (CRenderer& renderer, CPen* redPen, CPen* bluePen, CViewMatrix* viewMatrix);

		inline short MaxSegments (void) {
			short h = SEGMENT_LIMIT - segmentManager.Count ();
			return (h > MAX_TUNNEL_SEGMENTS) ? MAX_TUNNEL_SEGMENTS : h;
			}

	private:
		CDoubleVector RectPoints (double angle, double radius, CVertex* origin, CVertex* normal); 

		bool Update (void);
	};

extern CTunnelMaker tunnelMaker;

//------------------------------------------------------------------------------

#endif //__tunnelmaker_h