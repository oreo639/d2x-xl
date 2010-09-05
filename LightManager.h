#ifndef __lightman_h
#define __lightman_h

#include "define.h"

# pragma pack(push, packing)
# pragma pack(1)

#include "Vector.h"
#include "cfile.h"
#include "carray.h"

#define MAX_LIGHT_DEPTH 6

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct {
	short	ticks;
	short	impulse;
} tLightTimer;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct {
	bool	bIsOn;
	bool	bWasOn;
} tLightStatus;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tTextureLight {
  int    nBaseTex;
  long   light;
} tTextureLight;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tLightDeltaValue {
	byte vertLight [4];
} tLightDeltaValue;

// -----------------------------------------------------------------------------

class CLightDeltaValue : public CSideKey, public CGameItem {
public:
	tLightDeltaValue m_info;

	void Read (CFileManager& fp, int version = 0, bool bFlag = false);
	void Write (CFileManager& fp, int version = 0, bool bFlag = false);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		CSideKey::Clear ();
		}
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// Light at nSegment:nSide casts light on count sides beginning at index (in array CLightDeltaValues)
typedef struct tLightDeltaIndex {
	ushort count;
	ushort index;
} tLightDeltaIndex;

// -----------------------------------------------------------------------------

class CLightDeltaIndex : public CSideKey, public CGameItem {
public:
	tLightDeltaIndex m_info;

	void Read (CFileManager& fp, int version, bool bD2X);
	void Write (CFileManager& fp, int version, bool bD2X);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		CSideKey::Clear ();
		}
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tVariableLight {
	ushort mask;    // bits with 1 = on, 0 = off
	short timer;		 // always set to 0
	short delay;      // time for each bit in mask (short seconds)
} tVariableLight;

// -----------------------------------------------------------------------------

class CVariableLight : public CSideKey {
public:
	tVariableLight m_info;

	void Read (CFileManager& fp);
	void Write (CFileManager& fp);
	void Clear (void);
	void Setup (CSideKey key, short time, short mask);
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define MAX_VARIABLE_LIGHTS 100

#ifdef _DEBUG

typedef CStaticArray< CLightDeltaIndex, MAX_LIGHT_DELTA_INDICES_D2X > lightDeltaIndexList;
typedef CStaticArray< CLightDeltaValue, MAX_LIGHT_DELTA_VALUES_D2X > lightDeltaValueList;
typedef CStaticArray< CVariableLight, MAX_VARIABLE_LIGHTS > variableLightList;

typedef CStaticArray< CColor, SEGMENT_LIMIT * 6 > lightColorList;
typedef CStaticArray< CColor, MAX_TEXTURES_D2 > texColorList;
typedef CStaticArray< CColor, VERTEX_LIMIT > vertexColorList;

#else

typedef CLightDeltaIndex lightDeltaIndexList [MAX_LIGHT_DELTA_INDICES_D2X];
typedef CLightDeltaValue lightDeltaValueList [MAX_LIGHT_DELTA_VALUES_D2X];
typedef CVariableLight variableLightList [MAX_VARIABLE_LIGHTS];

typedef CColor lightColorList [SEGMENT_LIMIT * 6];
typedef CColor texColorList [MAX_TEXTURES_D2];
typedef CColor vertexColorList [VERTEX_LIMIT];

#endif

class CLightManager {
	private:
		variableLightList		m_variableLights;
		short						m_nCount;
		lightDeltaIndexList	m_deltaIndex;
		lightDeltaValueList	m_deltaValues;
		CMineItemInfo			m_deltaIndexInfo;
		CMineItemInfo			m_deltaValueInfo;

		lightColorList			m_lightColors;
		texColorList			m_texColors;
		vertexColorList		m_vertexColors;
		bool						m_bUseTexColors;

		tLightTimer				m_lightTimers [MAX_VARIABLE_LIGHTS];
		tLightStatus			m_lightStatus [SEGMENT_LIMIT][MAX_SIDES_PER_SEGMENT];

		long						m_lightMap [MAX_TEXTURES_D2];

		int						m_renderDepth;
		int						m_deltaRenderDepth;
		int						m_nNoLightDeltas;
		//long						m_defLightMap [MAX_TEXTURES_D2];

	public:
		inline void ResetInfo (void) {
			m_deltaIndexInfo.Reset ();
			m_deltaValueInfo.Reset ();
			}

		inline lightDeltaIndexList& LightDeltaIndices (void) { return m_deltaIndex; }

		inline lightDeltaValueList& LightDeltaValues (void) { return m_deltaValues; }

		inline variableLightList& VariableLights (void) { return m_variableLights; }

		inline vertexColorList& VertexColors (void) { return m_vertexColors; }

		inline CLightDeltaIndex* LightDeltaIndex (short i) { return &m_deltaIndex [i]; }

		inline CLightDeltaValue* LightDeltaValue (short i) { return &m_deltaValues [i]; }

		inline CVariableLight* VariableLight (short i) { return &m_variableLights [i]; }

		inline CColor* VertexColor (int i) { return &m_vertexColors [i]; }

		//inline CColor* GetTexColor (short nTexture = 0) { return &m_texColors [nTexture & 0x1FFF]; }

		inline bool& UseTexColors (void) { return m_bUseTexColors; }

		inline long* LightMap (int i = 0) { return &m_lightMap [i]; }

		inline short& Count (void) { return m_nCount; }

		inline void SetTexColor (short nBaseTex, CColor *pc)	{
			if (UseTexColors () && (IsLight (nBaseTex) != -1))
			m_texColors [nBaseTex] = *pc;
			}

		inline CColor* GetTexColor (short nBaseTex, bool bIsTranspWall)	
			{ return m_bUseTexColors && (bIsTranspWall || (IsLight (nBaseTex) != -1)) ? &m_texColors [nBaseTex] : null; }

		inline lightColorList& LightColors (void) { return m_lightColors; }

		inline CColor* LightColor (short nSegment, short nSide = 0) { return &m_lightColors [nSegment * 6 + nSide]; }

		CColor* LightColor (CSideKey key = CSideKey (), bool bUseTexColors = true);

		short LoadDefaults (void);
		bool HasCustomLightMap (void);
		bool HasCustomLightColors (void);

		void ScaleCornerLight (double fLight, bool bAll = false);
		void CalcAverageCornerLight (bool bAll = false);
		void AutoAdjustLight (double fBrightness, bool bAll = false, bool bCopyTexLights = false);
		void BlendColors (CColor *psc, CColor *pdc, double srcBr, double destBr);
		void Illuminate (short nSrcSide, short nSrcSeg, uint brightness, 
							  double fLightScale, bool bAll = false, bool bCopyTexLights = false);
		void IlluminateSide (CSegment* segP, short nSide, uint brightness, CColor* lightColorP, double* effect, double fLightScale);
		void SetLight (double fLight, bool bAll = false, bool bDynSegLights = false);

		int FindLight (int nTexture, tTextureLight* texLightP, int nLights);

		int IsLight (int nBaseTex);
		bool IsBlastableLight (int nBaseTex);
		bool IsVariableLight (CSideKey key);

		bool CalcDeltaLights (double fLightScale, int force, int recursion_depth);
		void CalcDeltaLightData (double fLightScale = 1.0, int force = 1);
		int FindDeltaLight (short nSegment, short nSide, short *pi = null);
		short VariableLight (CSideKey key);
		short AddVariableLight (CSideKey key, uint mask = 0xAAAAAAAA, int time = 0x10000 / 4);
		bool DeleteVariableLight (CSideKey key);
		void DeleteVariableLight (short index);
		int IsExplodingLight(int nBaseTex);

		void CreateLightMap (void);
		void ReadLightMap (CFileManager& fp, uint nSize);
		void WriteLightMap (CFileManager& fp);
		void WriteColorMap (CFileManager& fp);
		void ReadColorMap (CFileManager& fp);
		void ReadColors (CFileManager& fp);
		void WriteColors (CFileManager& fp);
		void ReadVariableLights (CFileManager& fp);
		void WriteVariableLights (CFileManager& fp);
		inline void ReadIndexInfo (CFileManager& fp) { m_deltaIndexInfo.Read (fp); }
		inline void WriteIndexInfo (CFileManager& fp) { m_deltaIndexInfo.Write (fp); }
		inline void ReadValueInfo (CFileManager& fp) { m_deltaValueInfo.Read (fp); }
		inline void WriteValueInfo (CFileManager& fp) { m_deltaValueInfo.Write (fp); }

	private:
		void CLightManager::LoadColors (CColor *pc, int nColors, int nFirstVersion, int nNewVersion, CFileManager& fp);
		void CLightManager::SaveColors (CColor *pc, int nColors, CFileManager& fp);

		bool CalcSideLights (int nSegment, int nSide, CVertex& sourceCenter, CVertex* sourceCorner, CVertex& A, 
									double* effect, double fLightScale, bool bIgnoreAngle);

		bool CalcLightDeltas (double fLightScale, int force, int nDepth);

		int FindLightDelta (short nSegment, short nSide, short *pi);

		byte LightWeight (short nBaseTex);
};

extern CLightManager lightManager;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif // __lightman_h

