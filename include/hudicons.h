#ifndef _HUDICONS_H
#define _HUDICONS_H

extern int bHaveInvBms;

class CHUDIcons {
	private:
		float	m_xScale;
		float	m_yScale;
		int	m_nLineSpacing;

	public:
		int EquipmentActive (int bFlag);
		int LoadInventoryIcons (void);
		void DestroyInventoryIcons (void);
		int LoadTallyIcons (void);
		void DestroyTallyIcons (void);
		int LoadGaugeIcons (void);
		void DestroyGaugeIcons (void);
		CBitmap& GaugeIcon (int i);
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

	private:
		int GetWeaponState (int& bHave, int& bAvailable, int& bActive, int i, int j, int l);
		int GetAmmo (char* szAmmo, int i, int j, int l);
		int GetWeaponIndex (int i, int j, int& nMaxAutoSelect);
		CBitmap* LoadWeaponIcon (int i, int l);
		void SetWeaponFillColor (int bHave, int bAvailable, float alpha);
		void SetWeaponFrameColor (int bHave, int bAvailable, int bActive, float alpha);

	};

extern CHUDIcons hudIcons;

//	-----------------------------------------------------------------------------

#endif //_HUDICONS_H
