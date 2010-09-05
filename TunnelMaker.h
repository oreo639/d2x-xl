#ifndef __tunnelmaker_h
#define __tunnelmaker_h

//------------------------------------------------------------------------

class CTunnelMaker {
	private:
		bool		m_bActive;
		int		m_nMaxSplines;
		double	m_splineLength1,
					m_splineLength2;

	public:
		void Create (void); 

		bool Active (void) { return m_bActive; }

		void Disable (void) { m_bActive = false; }

	private:
		long Faculty (int n); 

		double Coeff (int n, int i); 

		double Blend (int i, int n, double u); 

		CVertex BezierFcn (double u, int npts, CVertex* p); 

		void UntwistSegment (short nSegment,short nSide); 

		void SpinPoint (CVertex* point, double ySpin, double zSpin); 

		void SpinBackPoint (CVertex* point, double ySpin, double zSpin); 

		int MatchingSide (int j); 

		CDoubleVector RectPoints (double angle, double radius, CVertex* origin, CVertex* normal); 

		void PolarPoints (double *angle, double *radius, CVertex* vertex, CVertex* origin, CVertex* normal); 

		void IncreaseSpline (void); 

		void DecreaseSpline (void); 

		void ComputeSpline (void); 
	};

extern CTunnelMaker tunnelMaker;

//------------------------------------------------------------------------------

#endif //__tunnelmaker_h