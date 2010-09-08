#ifndef __lightman_h
#define __lightman_h

#include "define.h"

# pragma pack(push, packing)
# pragma pack(1)

#include "Vector.h"
#include "FileManager.h"
#include "carray.h"

#define MAX_LIGHT_DEPTH 6

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define MAX_VARIABLE_LIGHTS 100

#ifdef _DEBUG

typedef CStaticArray< CLightDeltaIndex, MAX_LIGHT_DELTA_INDICES_D2X > lightDeltaIndexList;
typedef CStaticArray< CLightDeltaValue, MAX_LIGHT_DELTA_VALUES_D2X > lightDeltaValueList;
typedef CStaticArray< CVariableLight, MAX_VARIABLE_LIGHTS > variableLightList;

typedef CStaticArray< CFaceColor, SEGMENT_LIMIT * 6 > faceColorList;
typedef CStaticArray< CTextureColor, MAX_TEXTURES_D2 > texColorList;
typedef CStaticArray< CVertexColor, VERTEX_LIMIT > vertexColorList;

#else

typedef CLightDeltaIndex lightDeltaIndexList [MAX_LIGHT_DELTA_INDICES_D2X];
typedef CLightDeltaValue lightDeltaValueList [MAX_LIGHT_DELTA_VALUES_D2X];
typedef CVariableLight variableLightList [MAX_VARIABLE_LIGHTS];

typedef CFaceColor faceColorList [SEGMENT_LIMIT * 6];
typedef CTextureColor texColorList [MAX_TEXTURES_D2];
typedef CVertexColor vertexColorList [VERTEX_LIMIT];

#endif

class CLightManager {
	private:
		variableLightList		m_variableLights;
		int						m_nCount;
		lightDeltaIndexList	m_deltaIndex;
		lightDeltaValueList	m_deltaValues;
		CMineItemInfo			m_deltaIndexInfo;
		CMineItemInfo			m_deltaValueInfo;

		faceColorList			m_faceColors;
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

		inline CTextureColor* TexColor (short nTexture = 0) { return &m_texColors [nTexture & 0x1FFF]; }

		inline bool& UseTexColors (void) { return m_bUseTexColors; }

		inline long* LightMap (int i = 0) { return &m_lightMap [i]; }

		inline int& Count (void) { return m_nCount; }

		inline int& DeltaIndexCount (void) { return m_deltaIndexInfo.count; }

		inline int& DeltaValueCount (void) { return m_deltaValueInfo.count; }

		inline void SetTexColor (short nBaseTex, CColor* colorP)	{
			if (UseTexColors () && (IsLight (nBaseTex) != -1))
				m_texColors [nBaseTex] = *colorP;
			}

		inline CColor* GetTexColor (short nTexture, bool bIsTranspWall)	
			{ return m_bUseTexColors && (bIsTranspWall || (IsLight (nTexture) != -1)) ? &m_texColors [nTexture] : null; }

		inline faceColorList& FaceColors (void) { return m_faceColors; }

		inline CFaceColor* FaceColor (short nSegment, short nSide = 0) { return &m_faceColors [nSegment * 6 + nSide]; }

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
		CVariableLight* AddVariableLight (short index = -1);
		short AddVariableLight (CSideKey key, uint mask = 0xAAAAAAAA, int time = 0x10000 / 4);
		bool DeleteVariableLight (CSideKey key);
		void DeleteVariableLight (short index, bool bUndo = false);
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
		void ReadLightDeltas (CFileManager& fp, int nFileVersion);
		void WriteLightDeltas (CFileManager& fp, int nFileVersion);

		inline void ReadDeltaIndexInfo (CFileManager& fp) { m_deltaIndexInfo.Read (fp); }
		inline void WriteDeltaIndexInfo (CFileManager& fp) { m_deltaIndexInfo.Write (fp); }
		inline void ReadDeltaValueInfo (CFileManager& fp) { m_deltaValueInfo.Read (fp); }
		inline void WriteDeltaValueInfo (CFileManager& fp) { m_deltaValueInfo.Write (fp); }

		inline void ReadLightDeltaInfo (CFileManager& fp) {
			ReadDeltaIndexInfo (fp);
			ReadDeltaValueInfo (fp);
			}

		void SortDeltaIndex (void);

	private:
		void CLightManager::LoadColors (CColor *pc, int nColors, int nFirstVersion, int nNewVersion, CFileManager& fp);
		void CLightManager::SaveColors (CColor *pc, int nColors, CFileManager& fp);

		bool CalcSideLights (int nSegment, int nSide, CVertex& sourceCenter, CVertex* sourceCorner, CVertex& A, 
									double* effect, double fLightScale, bool bIgnoreAngle);

		bool CalcLightDeltas (double fLightScale, int force, int nDepth);

		int FindLightDelta (short nSegment, short nSide, short *pi);

		byte LightWeight (short nBaseTex);
		
		void SortDeltaIndex (int left, int right);
};

extern CLightManager lightManager;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif // __lightman_h

