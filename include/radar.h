#ifndef _RADAR_H
#define _RADAR_H

#define RADAR_SLICES	64
#define BLIP_SLICES	32

class CRadar {
	private:
		static int				radarRanges [5];
		static float			radarSizes [3];
		static float			sizeOffsets [2][3];

		static CAngleVector	aRadar;
		static CFixMatrix		mRadar;
		static float			yOffs [2][CM_LETTERBOX + 1];
		static float			yRadar;
		static float			fRadius;
		static float			fLineWidth;

		static tSinCosf		sinCosRadar [RADAR_SLICES];
		static tSinCosf		sinCosBlip [BLIP_SLICES];
		static int				bInitSinCos;

		static tRgbColorf		shipColors [8];
		static tRgbColorf		guidebotColor;
		static tRgbColorf		robotColor;
		static tRgbColorf		powerupColor;
		static tRgbColorf		radarColor [2];
		static int				bHaveShipColors;

	private:
		CFloatVector	m_vertices [RADAR_SLICES];
		CFixVector		m_vCenter;
		CFloatVector	m_vCenterf;
		float				m_lineWidth;
		CFloatVector	m_offset;
		float				m_radius;

	public:
		CRadar ();

		void Render (void);

	private:
		void RenderSetup (void);
		void RenderBackground (void);
		void RenderDevice (void);
		void RenderBlip (CObject *objP, float r, float g, float b, float a, int bAbove);
		void RenderObjects (int bAbove);

	};

extern CRadar radar;

#endif /* _RADAR_H */
