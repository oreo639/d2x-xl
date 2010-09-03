#ifndef __lightman_h
#define __lightman_h

#include "define.h"

# pragma pack(push, packing)
# pragma pack(1)

#include "Vector.h"
#include "cfile.h"

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

	virtual void Read (CFileManager& fp, short version = 0, bool bFlag = false);
	virtual void Write (CFileManager& fp, short version = 0, bool bFlag = false);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		CSideKey::Clear ();
		}
	virtual CGameItem* Next (void) { return this + 1; }
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

	virtual void Read (CFileManager& fp, short version, bool bD2X);
	virtual void Write (CFileManager& fp, short version, bool bD2X);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		CSideKey::Clear ();
		}
	virtual CGameItem* Next (void) { return this + 1; }
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
	public:
		variableLightList		variableLights;
		short						m_nCount;
		lightDeltaIndexList	lightDeltaIndices;
		lightDeltaValueList	lightDeltaValues;

		lightColorList			m_lightColors;
		texColorList			m_texColors;
		vertexColorList		m_vertexColors;

		bool						m_bUseTexColors;

		long						m_lightMap [MAX_TEXTURES_D2];
		//long						m_defLightMap [MAX_TEXTURES_D2];

		inline lightDeltaIndexList& LightDeltaIndex (void) { return m_lightDeltaIndices; }

		inline lightDeltaValueList& LightDeltaValues (void) { return m_lightDeltaValues; }

		inline variableLightList& VariableLights (void) { return m_variableLights; }

		inline CLightDeltaIndex* GetLightDeltaIndex (short i) { return &m_lightDeltaIndices [i]; }

		inline CLightDeltaValue* GetLightDeltaValue (short i) { return &m_lightDeltaValues [i]; }

		inline CVariableLight* GetVariableLight (short i) { return &m_variableLights [i]; }

		inline CColor* GetTexColor (short nTexture = 0) { return &m_texColors [nTexture & 0x1FFF]; }

		inline bool& UseTexColors (void) { return m_bUseTexColors; }

		inline void SetTexColor (short nBaseTex, CColor *pc)	{
			if (UseTexColors () && (IsLight (nBaseTex) != -1))
			*TexColors (nBaseTex) = *pc;
			}

		inline CColor *GetTexColor (short nBaseTex, bool bIsTranspWall = false)	
			{ return UseTexColors () && (bIsTranspWall || (IsLight (nBaseTex) != -1)) ? TexColors (nBaseTex) : null; }

		inline lightColorList& LightColors (void) { return m_lightColors; }

		CColor* LightColor (short nSegment = -1, short nSide = -1, bool bUseTexColors = true);

		inline CColor* GetLightColor (short nSegment, short nSide = 0) { return &m_lightColors [nSegment * 6 + nSide]; }

		short LoadDefaults (void);
		bool HasCustomLightMap (void);
		bool HasCustomLightColors (void);

		void CreateLightMap (void);
		int ReadLightMap (CFileManager& fp, uint nSize);
		int WriteLightMap (CFileManager& fp);
		short WriteColorMap (CFileManager& fp);
		short ReadColorMap (CFileManager& fp);

	private:
		void CLightManager::LoadColors (CColor *pc, int nColors, int nFirstVersion, int nNewVersion, CFileManager& fp);
		void CLightManager::SaveColors (CColor *pc, int nColors, CFileManager& fp);

};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif // __lightman_h

