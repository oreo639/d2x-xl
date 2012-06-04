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
		ubyte				m_oppVertexIndex [4];
		CDoubleMatrix	m_rotation; // orientation of tunnel end side
		double			m_sign;

		CTunnelBase (CSideKey key = CSideKey (-1, -1)) : CSideKey (key) {}

		CTunnelBase (CDoubleVector point, CDoubleVector normal) : m_point (point), m_normal (normal), CSideKey (-1, -1) {}

		void Setup (double sign);

		inline CDoubleVector GetPoint (void) { return m_point; }

		inline void SetPoint (CDoubleVector point) { m_point = point; }

		inline CDoubleVector GetNormal (void) { return m_normal * -m_sign; }

		inline void SetNormal (CDoubleVector normal) { m_normal = normal; }

		inline CSegment* Segment (void) { return (m_nSegment < 0) ? null : segmentManager.Segment (m_nSegment); }

		inline CSide* Side (void) { return (m_nSide < 0) ? null : segmentManager.Side (m_nSide); }

	};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

class CTunnelPathNode {
	public:
		CVertex			m_vertex; // absolute and unrotated vertices
		CDoubleMatrix	m_rotation;
		double			m_angle; // rotation angle around z axis

	void CreateOrientation (CVertex fVec, CDoubleMatrix& mOrient, double zAngle);

	void Draw (CRenderer& renderer, CViewMatrix* viewMatrix);
	};

//------------------------------------------------------------------------

class CTunnelStartSide : public CSideKey {
	public:
		CTunnelStartSide () {}

		CTunnelStartSide (CSideKey& other) : CSideKey (other) {}

		CSideKey& operator = (CSideKey other) { 
			m_nSegment = other.m_nSegment, m_nSide = other.m_nSide; 
			return *this;
			}
	
		short m_nVertexIndex [4];
	};

//------------------------------------------------------------------------

class CTunnelPath {
	public:
		CCubicBezier							m_bezier;
		CTunnelBase								m_base [2];
		short										m_nSteps;
		CDynamicArray<CTunnelPathNode>	m_nodes;
		double									m_startAngle;
		double									m_deltaAngle;
		CDynamicArray<CTunnelStartSide>	m_startSides;
		CDynamicArray<ushort>				m_nStartVertices;

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

		inline CTunnelPathNode& operator[] (uint i) { return m_nodes [i]; }
	};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
// A single segment of a tunnel

class CTunnelElement {
	public:
		short		m_nSegment;
		ushort	m_nVertices [4];

	CTunnelElement () : m_nSegment (short (0x7fff)) {}

	};

//------------------------------------------------------------------------

class CTunnelSegment {
	public:
		CDynamicArray<ushort>			m_nVertices;
		CDynamicArray<CTunnelElement>	m_elements;

		CTunnelSegment () {}

		bool CTunnelSegment::Create (CTunnelPath& path, short nSegments, short nVertices);
		
		void CTunnelSegment::Release (void);

		void Draw (void);
	};

//------------------------------------------------------------------------
// A string of connected tunnel segments from a start to an end point 

class CTunnel {
	public:
		short									m_nSteps; // current path length
		CTunnelBase							m_base [2];
		CDynamicArray<CTunnelSegment>	m_segments;

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
		bool					m_bActive;
		short					m_nSteps;
		short					m_nGranularity;
		CTunnelBase			m_base [2];
		CTunnel				m_tunnel;
		CTunnelPath			m_path;

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