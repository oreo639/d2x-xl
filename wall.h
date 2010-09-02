#ifndef __wall_h
#define __wall_h

#include "define.h"

# pragma pack(push, packing)
# pragma pack(1)

#include "cfile.h"

//------------------------------------------------------------------------

#define WALL_HPS				I2X (100) // Normal wall's hp 
#define WALL_DOOR_INTERVAL	I2X (5)	 // How many seconds a door is open 

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

typedef struct tWall {
	int		hps;            // "Hit points" of the wall. 
	int		linkedWall;		 // number of linked wall
	byte		type;           // What kind of special wall.
	ushort	flags;          // Flags for the wall.    
	byte		state;          // Opening, closing, etc.
	byte		nTrigger;       // Which trigger is associated with the wall.
	char		nClip;          // Which  animation associated with the wall. 
	byte		keys;           // which keys are required
	// the following two Descent2 bytes replace the "short pad" of Descent1
	char		controllingTrigger; // which trigger causes something to happen here.
	// Not like "trigger" above, which is the trigger on this wall.
	//	Note: This gets stuffed at load time in gamemine.c.  
	// Don't try to use it in the editor.  You will be sorry!
	char		cloakValue;	// if this wall is cloaked, the fade value
} tWall;

//------------------------------------------------------------------------

class CWall : public CSideKey, public CGameItem {
public:
	tWall		m_info;
 
	void Read (CFileManager& fp, int version = 0, bool bFlag = false);
	void Write (CFileManager& fp, int version = 0, bool bFlag = false);
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	virtual CGameItem* Next (void) { return this + 1; }

	void Setup (short nSegment, short nSide, ushort nWall, byte type, char nClip, short nTexture, bool bRedefine);

	bool IsDoor (void);
	CSide* GetSide (void);
	inline void SetTrigger (short nTrigger) { m_info.nTrigger = (byte) nTrigger; }
	CTrigger* GetTrigger (void);
	int SetClip (short nTexture);
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

typedef struct tActiveDoor {
  int		nParts;				// for linked walls
  short	nFrontWall [2];	// front wall numbers for this door
  short	nBackWall [2];		// back wall numbers for this door
  int		time;					// how long been opening, closing, waiting
} tActiveDoor;

//------------------------------------------------------------------------

class CActiveDoor : public CGameItem {
public:
	tActiveDoor	m_info;

	virtual void Read (CFileManager& fp, int version = 0, bool bFlag = false);
	virtual void Write (CFileManager& fp, int version = 0, bool bFlag = false);
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	virtual CGameItem* Next (void) { return this + 1; }
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

typedef struct tCloakingWall {
	short		nFrontWall;		// front wall numbers for this door
	short		nBackWall; 		// back wall numbers for this door
	int		front_ls[4]; 	// front wall saved light values
	int		back_ls[4];		// back wall saved light values
	int		time;				// how long been cloaking or decloaking
} tCloakingWall;

//------------------------------------------------------------------------

class CCloakingWall : public CGameItem {    // NEW for Descent 2
public:
	tCloakingWall m_info;

	virtual void Read (CFileManager& fp, int version = 0, bool bFlag = false);
	virtual void Write (CFileManager& fp, int version = 0, bool bFlag = false);
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	virtual CGameItem* Next (void) { return this + 1; }
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

#endif // __wall_h

