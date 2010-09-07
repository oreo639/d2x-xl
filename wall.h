#ifndef __wall_h
#define __wall_h

#include "define.h"

# pragma pack(push, packing)
# pragma pack(1)

#include "FileManager.h"

//------------------------------------------------------------------------

#define WALL_HPS				I2X (100) // Normal wall's hp 
#define WALL_DOOR_INTERVAL	I2X (5)	 // How many seconds a door is open 

ushort wallFlags [MAX_WALL_FLAGS] = {
	WALL_BLASTED,
	WALL_DOOR_OPENED,
	WALL_DOOR_LOCKED,
	WALL_DOOR_AUTO,
	WALL_ILLUSION_OFF,
	WALL_WALL_SWITCH,
	WALL_BUDDY_PROOF,
	WALL_RENDER_ADDITIVE,
	WALL_IGNORE_MARKER
	};

byte animClipTable [NUM_OF_CLIPS_D2] = {
	 0,  1,  3,  4,  5,  6,  7,  9, 10, 11, 
	12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 
	22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 
	42, 43, 44, 45, 46, 47, 48, 49, 50
	};

byte doorClipTable [NUM_OF_CLIPS_D2] = {
	 1,  1,  4,  5, 10, 24,  8, 11, 13, 12, 
	14, 17, 18, 19, 20, 21, 22, 23, 25, 26, 
	28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 
	38, 39, 40, 41, 42, 43, 44, 45, 47, 48, 
	49, 50, 51, 52, 53, 54, 55, 56, 57
	};

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
	private:
		tWall		m_info;
	 
	public:
		inline tWall& Info (void) { return m_info; }

		void Read (CFileManager& fp, int version = 0, bool bFlag = false);
		
		void Write (CFileManager& fp, int version = 0, bool bFlag = false);
		
		void Setup (CSideKey key, ushort nWall, byte type, char nClip, short nTexture, bool bRedefine);
		
		void SetTextures (short nTexture);

		bool IsDoor (void);
		
		bool IsVisible (void);
		
		bool IsVariable (void);
		
		CSide* Side (void);
		
		inline void SetTrigger (short nTrigger) { m_info.nTrigger = (byte) nTrigger; }
		
		CTrigger* Trigger (void);
		
		int SetClip (short nTexture);

		virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }

		virtual CGameItem* Clone (void);

		virtual void Backup (eEditType editType = opModify);

		virtual CGameItem* Copy (CGameItem* destP);
	};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

typedef struct tDoor {
  int		nParts;				// for linked walls
  short	nFrontWall [2];	// front wall numbers for this door
  short	nBackWall [2];		// back wall numbers for this door
  int		time;					// how long been opening, closing, waiting
} tDoor;

//------------------------------------------------------------------------

class CDoor : public CGameItem {
	private:
		tDoor	m_info;

	public:
		void Read (CFileManager& fp, int version = 0, bool bFlag = false);

		void Write (CFileManager& fp, int version = 0, bool bFlag = false);

		virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }

		virtual CGameItem* Clone (void);

		virtual void Backup (eEditType editType = opModify);

		virtual CGameItem* Copy (CGameItem* destP);
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

	void Read (CFileManager& fp, int version = 0, bool bFlag = false);
	void Write (CFileManager& fp, int version = 0, bool bFlag = false);
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

#endif // __wall_h

