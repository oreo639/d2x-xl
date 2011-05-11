#ifndef _HUDICONS_H
#define _HUDICONS_H

extern int bHaveInvBms;

class CHUDIcons {
	private:
		float	xScale;
		float	yScale;
		int	nLineSpacing;

	public:
		int EquipmentActive (int bFlag);
		int LoadInventoryIcons (void);
		void DestroyInventoryIcons (void);
		int LoadTallyIcons (void);
		void DestroyTallyIcons (void);
		int LoadGaugeIcons (void);
		void DestroyGaugeIcons (void);
		void DrawTally (void);
		void ToggleWeaponIcons (void);
		void DrawWeapons (void);
		void DrawInventory (void);
		void Render (void);
		void Destroy (void);
		int LoadIcons (const char* pszIcons [], CBitmap* icons, int nIcons, int& bHaveIcons);
		void DestroyIcons (CBitmap* icons, int nIcons, int& bHaveIcons);

		inline bool Visible (void) { return !(gameStates.app.bNostalgia || gameStates.app.bEndLevelSequence || gameStates.render.bRearView); }
		inline bool Inventory (void) { return Visible () && gameOpts->render.weaponIcons.bEquipment && bHaveInvBms; }
	};

extern CHUDIcons hudIcons;

//	-----------------------------------------------------------------------------

#endif //_HUDICONS_H
