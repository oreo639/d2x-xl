#ifndef __wall_h
#define __wall_h

#include "define.h"

# pragma pack(push, packing)
# pragma pack(1)

#include "FileManager.h"

//------------------------------------------------------------------------------

#define WALL_HPS				I2X (100) // Normal wall's hp 
#define WALL_DOOR_INTERVAL	I2X (5)	 // How many seconds a door is open 

extern byte animClipTable [NUM_OF_CLIPS_D2];
extern byte doorClipTable [NUM_OF_CLIPS_D2];

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

class CWall : public CSideKey, public CGameItem {
	private:
		tWall		m_info;
	 
	public:
		inline tWall& Info (void) { return m_info; }

		inline byte& Type (void) { return m_info.type; }

		void Read (CFileManager* fp, bool bFlag = false);
		
		void Write (CFileManager* fp, bool bFlag = false);
		
		void Setup (CSideKey key, ushort nWall, byte type, char nClip, short nTexture, bool bRedefine);
		
		void SetTextures (short nTexture);

		bool IsDoor (void);
		
		bool IsVisible (void);
		
		bool IsVariable (void);

		inline bool IsTransparent (void) { return m_info.type == WALL_COLORED; }

		inline bool IsCloaked (void) { return m_info.type == WALL_CLOAKED; }

		inline bool IsIllusion (void) { return m_info.type == WALL_ILLUSION; }

		inline bool IsClosed (void) { return m_info.type == WALL_CLOSED; }

		inline byte Alpha (void) { 
			if (IsTransparent ())
				return (m_info.hps == 0) ? 128 : X2I (255 * m_info.hps); 
			if (IsCloaked () || IsIllusion () || IsClosed ())
				return 255 * (31 - m_info.cloakValue % 32) / 31;
			return 255;
			}
		
		CSide* Side (void);
		
		inline void SetTrigger (short nTrigger) { m_info.nTrigger = (byte) nTrigger; }
		
		CTrigger* Trigger (void);
		
		int SetClip (short nTexture);

		virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }

		virtual CGameItem* Clone (void);

		virtual void Backup (eEditType editType = opModify);

		virtual CGameItem* Copy (CGameItem* destP);

		virtual void Undo (void);

		virtual void Redo (void);
	};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

typedef struct tDoor {
  int		nParts;				// for linked walls
  short	nFrontWall [2];	// front wall numbers for this door
  short	nBackWall [2];		// back wall numbers for this door
  int		time;					// how long been opening, closing, waiting
} tDoor;

//------------------------------------------------------------------------------

class CDoor : public CGameItem {
	private:
		tDoor	m_info;

	public:
		void Read (CFileManager* fp, bool bFlag = false);

		void Write (CFileManager* fp, bool bFlag = false);

		virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }

		virtual CGameItem* Clone (void);

		virtual void Backup (eEditType editType = opModify);

		virtual CGameItem* Copy (CGameItem* destP);
	};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

typedef struct tCloakingWall {
	short		nFrontWall;		// front wall numbers for this door
	short		nBackWall; 		// back wall numbers for this door
	int		front_ls[4]; 	// front wall saved light values
	int		back_ls[4];		// back wall saved light values
	int		time;				// how long been cloaking or decloaking
} tCloakingWall;

//------------------------------------------------------------------------------

class CCloakingWall : public CGameItem {    // NEW for Descent 2
public:
	tCloakingWall m_info;

	void Read (CFileManager* fp, bool bFlag = false);
	void Write (CFileManager* fp, bool bFlag = false);
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#endif // __wall_h

