// dlcView.h : interface of the CMineView class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __toolview_h
#define __toolview_h

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "afxcview.h"
#include "dle-xp.h"
#include "DlcDoc.h"
#include "Matrix.h"
#include "poly.h"
#include "define.h"
#include "types.h"
#include "global.h"
#include "dle-xp-res.h"
#include "textures.h"
#include "texedit.h"

                        /*--------------------------*/
#if 1
class CExtBitmapButton : public CBitmapButton
{
	public:
		DECLARE_DYNCREATE(CExtBitmapButton)
	public:
		INT32	m_nId;
		CWnd	*m_pParent;
		UINT	m_nState;
		INT32	m_nPos;
		//virtual INT32 OnToolHitTest (CPoint point, TOOLINFO* pTI);
		BOOL AutoLoad (UINT nId, CWnd* pParent) {
			m_nId = nId;
			m_pParent = pParent;
			m_nState = WM_LBUTTONUP;
			m_nPos = -1;
			return CBitmapButton::AutoLoad (nId, pParent);
			}
		afx_msg void OnLButtonUp (UINT nFlags, CPoint point);
		afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
		afx_msg void OnMouseMove (UINT nFlags, CPoint point);
		afx_msg void OnSetFocus (CWnd* pOldWnd);
		afx_msg void OnKillFocus (CWnd* pNewWnd);
		void Notify (UINT nMsg);

		DECLARE_MESSAGE_MAP ()
};
#endif
                        /*--------------------------*/

class CConvertDlg : public CDialog
{
	public:
		bool			m_bInited;
		CWnd			m_showD1;
		CWnd			m_showD2;
		HINSTANCE	m_hInst;
		HGLOBAL		m_hTextures;
		INT16			*m_pTextures;

		CConvertDlg (CWnd *pParent = NULL);
      virtual BOOL OnInitDialog ();
		void EndDialog (INT32 nResult);
      virtual void DoDataExchange (CDataExchange *pDX);
		void CreateImgWnd (CWnd *pImgWnd, INT32 nIdC);
		void Reset ();
		void Refresh ();
		afx_msg void OnPaint ();
		afx_msg void OnSetD1 ();
		afx_msg void OnSetD2 ();
		virtual void OnOK (void);
		inline CComboBox *CBD1 ()
			{ return (CComboBox *) GetDlgItem (IDC_CONVERT_D1); }
		inline CComboBox *CBD2 ()
			{ return (CComboBox *) GetDlgItem (IDC_CONVERT_D2); }

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CToolDlg : public CPropertyPage
{
	public:
		DECLARE_DYNCREATE(CToolDlg)
   protected:
      CPropertySheet *  m_pParent;
	public:
		bool	m_bInited;
		bool	m_bHaveData;

		CToolDlg (UINT nIdTemplate = 0, CPropertySheet *pParent = NULL);
		~CToolDlg ()
			{ m_pParent = NULL; m_bInited = m_bHaveData = false; }
      virtual BOOL OnInitDialog () 
			{ return CPropertyPage::OnInitDialog (); }
      virtual BOOL OnSetActive () {
			if ((theMine == NULL) || !CPropertyPage::OnSetActive ())
				return FALSE;
         UpdateData (FALSE);
         return TRUE;
         };
      virtual void DoDataExchange (CDataExchange * pDX) {};
		inline bool HaveData (CDataExchange * pDX) { 
			if (!(theMine && m_bInited))
				return false;
			if (!pDX->m_bSaveAndValidate)
				m_bHaveData = true;
			return m_bHaveData;
			}
		void Refresh (void)
			{ UpdateData (FALSE); }
		void DDX_Double (CDataExchange * pDX, INT32 nIDC, double& fVal, double min = 1, double max = 0, LPSTR pszFmt = "%1.2f", LPSTR pszErrMsg = NULL);
		void DDX_Slider (CDataExchange * pDX, INT32 nIdC, INT32& nTic);
		INT32 DDX_Int (CDataExchange * pDX, INT32 nIdC, INT32 i);
		INT32 DDX_Flag (CDataExchange * pDX, INT32 nIdC, INT32 i);
		void InitSlider (INT32 nIdC, INT32 nMin, INT32 nMax);
		INT32 GetCheck (INT32 nIdC);
		INT32 CBAddString (CComboBox *pcb, char *str);
		void SelectItemData (CComboBox *pcb, INT32 nItemData);
		void EnableControls (INT32 nIdFirst, INT32 nIdLast, BOOL bEnable);
#if 0
		INT32 OnToolHitTest (CPoint point, TOOLINFO* pTI);
#endif
		BOOL OnToolTipNotify (UINT id, NMHDR *pNMHDR, LRESULT *pResult);
		void CreateImgWnd (CWnd * pImgWnd, INT32 nIdC);
		afx_msg void OnSelectPrevTab ();
		afx_msg void OnSelectNextTab ();
		void GetCtrlClientRect (CWnd *pWnd, CRect& rc);
		inline bool Inited ()
			{ return m_bInited; }
		inline CComboBox *CBCtrl (INT32 nId)
			{ return (CComboBox *) GetDlgItem (nId); }
		inline CListBox *LBCtrl (INT32 nId)
			{ return (CListBox *) GetDlgItem (nId); }
		inline CButton *BtnCtrl (INT32 nId)
			{ return (CButton *) GetDlgItem (nId); }
		inline CSliderCtrl *SlCtrl (INT32 nId)
			{ return (CSliderCtrl *) GetDlgItem (nId); }
		inline CScrollBar *SBCtrl (INT32 nId)
			{ return (CScrollBar *) GetDlgItem (nId); }

		DECLARE_MESSAGE_MAP()
};

                        /*--------------------------*/

class CTexToolDlg : public CToolDlg
{
	public:
		DECLARE_DYNCREATE(CTexToolDlg)

	CWnd		m_textureWnd;
	UINT_PTR	m_nTimer;
	INT32		m_nTexWndId;
	INT32		m_nTimerId;
	COLORREF	m_bkColor;
	INT32		m_frame [2];
	bool		m_bOtherSeg;

	CTexToolDlg (UINT nIdTemplate = 0, CPropertySheet *pParent = NULL, 
					 INT32 nTexWndId = 0, INT32 nTimerId = -1, COLORREF bkColor = RGB (0,0,0),
					 bool bOtherSeg = false);
	~CTexToolDlg ();
	bool Refresh (INT16 nBaseTex = -1, INT16 nOvlTex = -1, INT16 nVisible = -1);
	void AnimateTexture (void);

   virtual BOOL OnInitDialog ();
	afx_msg void OnPaint ();
	afx_msg void OnTimer (UINT_PTR nIdEvent);
	virtual BOOL OnSetActive ();
	virtual BOOL OnKillActive ();
	virtual BOOL TextureIsVisible ();

	DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CAdvObjTool : public CToolDlg
{
	public:
		INT32		m_mass;
		INT32		m_drag;
		INT32		m_brakes;
		INT32		m_turnRoll;
		INT32		m_size;
		INT32		m_flags;
		INT32		m_shields;
		INT32		m_vx;
		INT32		m_vy;
		INT32		m_vz;
		INT32		m_tx;
		INT32		m_ty;
		INT32		m_tz;
		INT32		m_rvx;
		INT32		m_rvy;
		INT32		m_rvz;
		INT32		m_rtx;
		INT32		m_rty;
		INT32		m_rtz;
		INT32		m_model;
		INT32		m_frame;
		INT32		m_frameNo;
		CAdvObjTool (CPropertySheet *pParent = NULL);
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		virtual BOOL OnKillActive ();
		void Reset ();
		void Refresh ();
		afx_msg void OnAccept ();

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CDiagTool : public CToolDlg
{
	public:
		INT32			m_nTrigger;
		INT32			m_targets;
		INT32			m_iTarget;
		INT32			m_nCountDown;
		INT32			m_nSecretReturn;
		char			m_szTarget [40];
		INT32			m_nObjects [16];
		INT32			m_nContained [2];
		INT32			m_nErrors [2];
		INT32			m_bAutoFixBugs;
		INT32			m_bShowWarnings;
		CReactorTrigger	*m_pTrigger;
		bool			m_bCheckMsgs;

		CDiagTool (CPropertySheet *pParent = NULL);
		~CDiagTool ();
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		void Reset ();
		void Refresh ();
		LPSTR ItemText (INT32 nValue, LPSTR pszPrefix = NULL);
		void CountObjects (void);
		INT32 CountTextures (void);
		afx_msg void OnCheckMine ();
		afx_msg void OnShowBug ();
		afx_msg void OnClearList ();
		afx_msg void OnFixBugs ();
		afx_msg void OnShowWarnings ();
		INT32 AddMessage (const char *pszMsg, INT32 nMaxMsgs = 100, bool bCheckMsg = false);
		bool UpdateStats (char *szError, INT32 nErrorLevel, 
							   INT32 nSegment = -1, INT32 nSide = -1, INT32 linenum = -1, INT32 pointnum = -1, 
							   INT32 childnum = -1, INT32 nWall = -1, INT32 nTrigger = -1, INT32 objnum = -1);
		double CalcFlatnessRatio (INT16 nSegment, INT16 nSide);
		double CalcDistance (CFixVector* v1,CFixVector* v2,CFixVector* v3);
		double CalcAngle (INT16 vert0,INT16 vert1,INT16 vert2,INT16 vert3);
		void ClearBugList ();
		INT32 CheckId (CGameObject *objP);
		bool CheckSegments ();
		bool CheckSegTypes ();
		bool CheckWalls ();
		bool CheckTriggers ();
		bool CheckObjects ();
		bool CheckAndFixPlayer (INT32 nMin, INT32 nMax, INT32 nObject, INT32* players);
		bool CheckVertices ();
		bool CheckBotGens ();
		bool CheckEquipGens ();
		bool MarkSegment (INT16 nSegment);
		INT8 FindMatCen (CRobotMaker* matCenP, INT16 nSegment, INT16* refList = NULL);
		void CountMatCenRefs (INT32 nSpecialType, INT16* refList, CRobotMaker* matCenP, INT16 nMatCens);
		INT16 FixMatCens (INT32 nSpecialType, INT16* segList, INT16* refList, CRobotMaker* matCenP, INT16 nMatCens, char* pszType);
		INT16 AssignMatCens (INT32 nSpecialType, INT16* segList, INT16* refList, CRobotMaker* matCenP, INT16 nMatCens);
		INT16 CleanupMatCens (INT16* refList, CRobotMaker* matCenP, INT16 nMatCens);

		inline CListView *LVStats ()
			{ return (CListView *) GetDlgItem (IDC_DIAG_STATS); }
		inline CListBox *LBBugs ()
			{ return LBCtrl (IDC_DIAG_BUGLIST); }
		inline CWnd *TargetEdit ()
			{ return GetDlgItem (IDC_REACTOR_TARGET); }
		CWall *OppWall (UINT16 nSegment, UINT16 nSide);

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CReactorTool : public CToolDlg
{
	public:
		INT32			m_nTrigger;
		INT32			m_targets;
		INT32			m_iTarget;
		INT32			m_nCountDown;
		INT32			m_nSecretReturn;
		char			m_szTarget [40];
		CReactorTrigger	*m_pTrigger;

		CReactorTool (CPropertySheet *pParent = NULL);
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		void EnableControls (BOOL bEnable);
		void InitLBTargets ();
		void Reset ();
		void Refresh ();
		void AddTarget (INT16 nSegment, INT16 nSide);
		INT32 FindTarget (INT16 nSegment, INT16 nSide);
		afx_msg void OnAddTarget ();
		afx_msg void OnDeleteTarget ();
		afx_msg void OnAddWallTarget ();
		afx_msg void OnDeleteWallTarget ();
		afx_msg void OnCountDown ();
		afx_msg void OnSecretReturn ();
		afx_msg void OnSetTarget ();
		inline CListBox *LBTargets ()
			{ return LBCtrl (IDC_REACTOR_TARGETLIST); }
		inline CWnd *TargetEdit ()
			{ return GetDlgItem (IDC_REACTOR_TARGET); }

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CMissionTool : public CToolDlg
{
	public:
		MISSION_DATA	m_missionData;
		char				m_szLevel [26];

		CMissionTool (CPropertySheet *pParent = NULL);
	   virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual void OnOK (void);
		virtual void OnCancel (void);
		afx_msg void OnSetLevelName (void);
		afx_msg void OnAdd (void);
		afx_msg void OnDelete (void);
		afx_msg void OnRename (void);
		afx_msg void OnUp (void);
		afx_msg void OnDown (void);
		afx_msg void OnFromHog (void);
		afx_msg void OnLoadLevel (void);
		void Refresh (void);
		virtual BOOL OnSetActive ();
		void BuildLevelList (void);
		LPSTR CopyLevelName (LPSTR pszDest, LPSTR pszSrc);
		LPSTR FixLevelName (LPSTR pszName);
		inline CListBox *LBLevels ()
			{ return LBCtrl (IDC_MISSION_LEVELLIST); }
		DECLARE_MESSAGE_MAP()
};

                        /*--------------------------*/

class CObjectTool : public CToolDlg
{
	public:
		CWnd		m_showObjWnd;
		CWnd		m_showSpawnWnd;
		CWnd		m_showTextureWnd;
		INT32		m_nSpawnQty;
		INT32		m_bEndsLevel;
		char		m_szInfo [200];

		CObjectTool (CPropertySheet *pParent = NULL);
		~CObjectTool ();
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		void Reset ();
		void EnableControls (BOOL bEnable);
		void Refresh ();
		void RefreshRobot ();
		void DrawObjectImages ();
		void DrawObject (CWnd *pWnd, INT32 type, INT32 id);
		void SetTextureOverride ();
		bool SetPlayerId (CGameObject *objP, INT32 objType, INT32 *ids, INT32 maxIds, char *pszError);
		void SetObjectId (CComboBox *pcb, INT16 type, INT16 id, INT16 flag = 0);

		void CBInit (CComboBox *pcb, char* pszNames [], UINT8 *pIndex, UINT8 *pItemData, INT32 nMax, INT32 nType = 0, bool bAddNone = false);
		void InitSliders ();
		void UpdateSliders (INT32 i = -1);
		INT32 GetSliderData (CScrollBar *pScrollBar);
		double SliderFactor (INT32 nId);
		void UpdateRobot ();
		INT32 ObjOfAKindCount (INT32 nType = -1, INT32 nId = -1);
		INT32 GetObjectsOfAKind (INT32 nType, CGameObject *objList []);
		void SetNewObjId (CGameObject *objP, INT32 nType, INT32 nId, INT32 nMaxId);

		afx_msg void OnPaint ();
		afx_msg void OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);

		afx_msg void OnAdd ();
		afx_msg void OnDelete ();
		afx_msg void OnDeleteAll ();
		afx_msg void OnMove ();
		afx_msg void OnReset ();
		afx_msg void OnDefault ();
		afx_msg void OnAdvanced ();
		afx_msg void OnSetObject ();
		afx_msg void OnSetObjType ();
		afx_msg void OnSetObjId ();
		afx_msg void OnSetTexture ();
		afx_msg void OnSetSpawnType ();
		afx_msg void OnSetSpawnId ();
		afx_msg void OnSetSpawnQty ();
		afx_msg void OnSetSoundExplode ();
		afx_msg void OnSetSoundSee ();
		afx_msg void OnSetSoundAttack ();
		afx_msg void OnSetSoundClaw ();
		afx_msg void OnSetSoundDeath ();
		afx_msg void OnSetObjAI ();
		afx_msg void OnSetObjClassAI ();
		afx_msg void OnSetWeapon1 ();
		afx_msg void OnSetWeapon2 ();
		afx_msg void OnSetContType ();
		afx_msg void OnSetContId ();
		afx_msg void OnAIKamikaze ();
		afx_msg void OnAICompanion ();
		afx_msg void OnAIThief ();
		afx_msg void OnAISmartBlobs ();
		afx_msg void OnAIPursue ();
		afx_msg void OnAICharge ();
		afx_msg void OnAIEDrain ();
		afx_msg void OnAIEndsLevel ();
		afx_msg void OnAIBossType ();
		afx_msg void OnBright ();
		afx_msg void OnCloaked ();
		afx_msg void OnMultiplayer ();
		afx_msg void OnSort ();

		inline CComboBox *CBObjNo ()
			{ return CBCtrl (IDC_OBJ_OBJNO); }
		inline CComboBox *CBObjType ()
			{ return CBCtrl (IDC_OBJ_TYPE); }
		inline CComboBox *CBObjId ()
			{ return CBCtrl (IDC_OBJ_ID); }
		inline CComboBox *CBObjAI ()
			{ return CBCtrl (IDC_OBJ_AI); }
		inline CComboBox *CBObjTexture ()
			{ return CBCtrl (IDC_OBJ_TEXTURE); }
		inline CComboBox *CBSpawnType ()
			{ return CBCtrl (IDC_OBJ_SPAWN_TYPE); }
		inline CComboBox *CBBossType ()
			{ return CBCtrl (IDC_OBJ_AI_BOSSTYPE); }
		inline CComboBox *CBSpawnId ()
			{ return CBCtrl (IDC_OBJ_SPAWN_ID); }
		inline CComboBox *CBObjClassAI ()
			{ return CBCtrl (IDC_OBJ_CLASS_AI); }
		inline CComboBox *CBContType ()
			{ return CBCtrl (IDC_OBJ_CONT_TYPE); }
		inline CComboBox *CBContId ()
			{ return CBCtrl (IDC_OBJ_CONT_ID); }
		inline CComboBox *CBExplType ()
			{ return CBCtrl (IDC_OBJ_EXPLTYPE); }
		inline CComboBox *CBWeapon1 ()
			{ return CBCtrl (IDC_OBJ_WEAPON1); }
		inline CComboBox *CBWeapon2 ()
			{ return CBCtrl (IDC_OBJ_WEAPON2); }
		inline CComboBox *CBSoundExpl ()
			{ return CBCtrl (IDC_OBJ_SOUND_EXPLODE); }
		inline CComboBox *CBSoundSee ()
			{ return CBCtrl (IDC_OBJ_SOUND_SEE); }
		inline CComboBox *CBSoundAttack ()
			{ return CBCtrl (IDC_OBJ_SOUND_ATTACK); }
		inline CComboBox *CBSoundClaw ()
			{ return CBCtrl (IDC_OBJ_SOUND_CLAW); }
		inline CComboBox *CBSoundDeath ()
			{ return CBCtrl (IDC_OBJ_SOUND_DEATH); }

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CEffectTool : public CToolDlg
{
	public:
		CGameObject*		m_obj;
		CSmokeInfo			m_smoke;
		CLightningInfo		m_lightning;
		CSoundInfo			m_sound;
		INT32					m_nBufferId;

		CEffectTool (CPropertySheet *pParent = NULL);
		~CEffectTool ();
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		virtual BOOL OnKillActive ();
		void Reset ();
		void EnableControls (BOOL bEnable);
		void Refresh ();
		bool AddEffect ();
		void LoadEffectList ();
		void HiliteTarget (void);
		bool FindSlider (CScrollBar *pScrollBar);

		inline CComboBox *CBEffects ()
			{ return CBCtrl(IDC_EFFECT_OBJECTS); }
		inline CComboBox *CBStyle ()
			{ return CBCtrl(IDC_LIGHTNING_STYLE); }
		inline CComboBox *CBType ()
			{ return CBCtrl(IDC_SMOKE_TYPE); }

		afx_msg void OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
		afx_msg void OnSetObject ();
		afx_msg void OnSetStyle ();
		afx_msg void OnEdit ();
		afx_msg void OnAddSmoke ();
		afx_msg void OnAddLightning ();
		afx_msg void OnAddSound ();
		afx_msg void OnDelete ();
		afx_msg void OnCopy ();
		afx_msg void OnPaste ();
		afx_msg void OnPasteAll ();
#if 0
		afx_msg void OnSetSmokeLife ();
		afx_msg void OnSetSmokeSize ();
		afx_msg void OnSetSmokeDensity ();
		afx_msg void OnSetSmokeSpeed ();
		afx_msg void OnSetSmokeDrift ();
		afx_msg void OnSetSmokeBrightness ();

		afx_msg void OnSetLightningSmoothe ();
		afx_msg void OnSetLightningClamp ();
		afx_msg void OnSetLightningPlasma ();
		afx_msg void OnSetLightningSound ();
		afx_msg void OnSetLightningRandom ();
#endif
		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CSegmentTool : public CToolDlg
{
	public:
		INT32		m_nSegment;
		INT32		m_nSide;
		INT32		m_nPoint;
		INT32		m_nVertex;
		INT32		m_nType;
		INT32		m_nProps;
		INT32		m_nOwner;
		INT32		m_nGroup;
		INT32		m_bEndOfExit;
		INT16		m_nDamage [2];
		double	m_nLight;
		double	m_nCoord [3];
		INT32		m_nLastCube;
		INT32		m_nLastSide;
		INT32		m_bSetDefTexture;

		CSegmentTool (CPropertySheet *pParent = NULL);
		virtual BOOL OnInitDialog ();
		virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		void InitCBCubeNo ();
		void Reset ();
		void EnableControls (BOOL bEnable);
		void Refresh ();
		void OnSide (INT32 nSide);
		void OnPoint (INT32 nPoint);
		INT32 FindBot (CListBox *plb, LPSTR pszBot = NULL);
		INT32 FindEquip (CListBox *plb, LPSTR pszBot = NULL);
		bool IsBotMaker (CSegment *segP);
		bool IsEquipMaker (CSegment *segP);
		void SetDefTexture (INT16 nTexture);
	
		afx_msg void OnSetCube ();
		afx_msg void OnSetType ();
		afx_msg void OnSetOwner ();
		afx_msg void OnSetGroup ();
		afx_msg void OnSetCoord ();
		afx_msg void OnResetCoord ();
		afx_msg void OnLight ();
		void OnDamage (INT32 i);
		afx_msg void OnDamage0 ();
		afx_msg void OnDamage1 ();
		void OnProp (INT32 nProp);
		afx_msg void OnProp1 ();
		afx_msg void OnProp2 ();
		afx_msg void OnProp3 ();
		afx_msg void OnProp4 ();
		afx_msg void OnProp5 ();
		void OnFlag (INT32 nID, INT32& nFlag);
		afx_msg void OnSetDefTexture ();
		afx_msg void OnSide1 ();
		afx_msg void OnSide2 ();
		afx_msg void OnSide3 ();
		afx_msg void OnSide4 ();
		afx_msg void OnSide5 ();
		afx_msg void OnSide6 ();
		afx_msg void OnPoint1 ();
		afx_msg void OnPoint2 ();
		afx_msg void OnPoint3 ();
		afx_msg void OnPoint4 ();
		afx_msg void OnAddCube ();
		afx_msg void OnAddBotGen ();
		afx_msg void OnAddFuelCen ();
		afx_msg void OnAddEquipGen ();
		afx_msg void OnAddRepairCen ();
		afx_msg void OnAddControlCen ();
		afx_msg void OnSplitCube ();
		afx_msg void OnDeleteCube ();
		afx_msg void OnOtherCube ();
		afx_msg void OnAddObj ();
		afx_msg void OnDeleteObj ();
		afx_msg void OnWallDetails ();
		afx_msg void OnTriggerDetails ();
		afx_msg void OnEndOfExit ();

		void AddBot (void);
		void DeleteBot (void);
		void AddEquip (void);
		void DeleteEquip (void);

		inline CComboBox *CBCubeNo ()
			{ return CBCtrl(IDC_CUBE_CUBENO); }
		inline CComboBox *CBType ()
			{ return CBCtrl(IDC_CUBE_TYPE); }
		inline CListBox *LBTriggers ()
			{ return LBCtrl (IDC_CUBE_TRIGGERS); }
		inline CListBox *LBUsedBots ()
			{ return LBCtrl (IDC_CUBE_USEDBOTS); }
		inline CListBox *LBAvailBots ()
			{ return LBCtrl (IDC_CUBE_AVAILBOTS); }
		inline CButton *EndOfExit ()
			{ return BtnCtrl (IDC_CUBE_ENDOFEXIT); }
		inline CButton *SetDefTexture ()
			{ return BtnCtrl (IDC_CUBE_SETDEFTEXTURE); }
		inline CButton *Prop (INT32 nProp)
			{ return BtnCtrl (IDC_CUBE_WATER + nProp); }
		inline CComboBox *CBOwner ()
			{ return CBCtrl(IDC_CUBE_OWNER); }

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CWallTool : public CTexToolDlg
{
	public:
		CWnd		m_textureWnd;
		INT32		m_nSegment;
		INT32		m_nSide;
		INT32		m_nTrigger;
		INT32		m_nWall [2];
		INT32		m_nType;
		INT32		m_nClip;
		double	m_nStrength;
		double	m_nCloak;
		INT32		m_bFlyThrough;
		INT32		m_bKeys [4];
		INT32		m_bFlags [MAX_WALL_FLAGS];
		char		m_szMsg [256];
		CWall	*m_pWall [2];
		CWall	m_defWall;
		CWall	m_defDoor;
		INT16		m_defTexture;
		INT16		m_defDoorTexture;
		INT16		m_defOvlTexture;
		BOOL		m_bBothSides;
		bool		m_bLock;
		bool		m_bDelayRefresh;

		CWallTool (CPropertySheet *pParent = NULL);
		~CWallTool ();
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		virtual BOOL OnKillActive ();
		void InitCBWallNo ();
		void Reset ();
		void EnableControls (BOOL bEnable);
		void Refresh ();
		virtual BOOL TextureIsVisible ();
		bool GetWalls ();
		CWall *GetOtherWall (void);

		afx_msg void OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
		afx_msg void OnBothSides ();
		afx_msg void OnAddWall ();
		afx_msg void OnDeleteWall ();
		afx_msg void OnDeleteWallAll ();
		afx_msg void OnLock ();
		afx_msg void OnOtherSide ();
		afx_msg void OnSetWall ();
		afx_msg void OnSetType ();
		afx_msg void OnSetClip ();
		afx_msg void OnNoKey ();
		afx_msg void OnBlueKey ();
		afx_msg void OnGoldKey ();
		afx_msg void OnRedKey ();
		afx_msg void OnBlasted ();
		afx_msg void OnDoorOpen ();
		afx_msg void OnDoorLocked ();
		afx_msg void OnDoorAuto ();
		afx_msg void OnIllusionOff ();
		afx_msg void OnSwitch ();
		afx_msg void OnBuddyProof ();
		afx_msg void OnRenderAdditive ();
		afx_msg void OnIgnoreMarker ();
		afx_msg void OnAddDoorNormal ();
		afx_msg void OnAddDoorExit ();
		afx_msg void OnAddDoorSecretExit ();
		afx_msg void OnAddDoorPrison ();
		afx_msg void OnAddDoorGuideBot ();
		afx_msg void OnAddWallFuelCell ();
		afx_msg void OnAddWallIllusion ();
		afx_msg void OnAddWallForceField ();
		afx_msg void OnAddWallFan ();
		afx_msg void OnAddWallGrate ();
		afx_msg void OnAddWallWaterfall ();
		afx_msg void OnAddWallLavafall ();

		void OnKey (INT32 i);
		void OnFlag (INT32 i); 
		void OnStrength ();
		void OnCloak ();

		inline CComboBox *CBWallNo ()
			{ return CBCtrl(IDC_WALL_WALLNO); }
		inline CComboBox *CBType ()
			{ return CBCtrl(IDC_WALL_TYPE); }
		inline CComboBox *CBClipNo ()
			{ return CBCtrl(IDC_WALL_CLIPNO); }
		inline CButton *KeyBtn (INT32 i)
			{ return BtnCtrl (IDC_WALL_NOKEY + i); }
		inline CButton *FlagBtn (INT32 i)
			{ return BtnCtrl (IDC_WALL_BLASTED + i); }
		inline CScrollBar *TransparencySlider ()
			{ return (CScrollBar *) GetDlgItem (IDC_WALL_TRANSPARENCY); }

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CTriggerTool : public CTexToolDlg
{
	public:
		INT32					m_nClass;
		INT32					m_nType;
		INT32					m_nTime;
		INT32					m_targets;
		INT32					m_iTarget;
		INT32					m_bD1Flags [MAX_TRIGGER_FLAGS];
		INT32					m_bD2Flags [16];
		INT32					m_nSliderValue;
		double				m_nStrength;
		CTrigger			m_defTrigger;
		CTrigger			*m_pTrigger;
		INT32					m_nTrigger;
		CTrigger			*m_pStdTrigger;
		INT32					m_nStdTrigger;
		CTrigger			*m_pObjTrigger;
		INT32					m_nObjTrigger;
		char					m_szTarget [40];
		INT32					m_bAutoAddWall;
		CWnd					m_showObjWnd;
		CWnd					m_showTexWnd;
		bool					m_bFindTrigger;

		CTriggerTool (CPropertySheet *pParent = NULL);
		~CTriggerTool ();
		virtual BOOL OnInitDialog ();
		virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		virtual BOOL OnKillActive ();
		void LoadTextureListBoxes ();
		void InitCBTriggerNo ();
		void InitLBTargets ();
		void Reset ();
		void EnableControls (BOOL bEnable);
		void Refresh ();
		void AddTarget (INT16 nSegment, INT16 nSide);
		INT32 FindTarget (INT16 nSegment, INT16 nSide);
		bool OnD1Flag (INT32 i, INT32 j = -1);
		void OnD2Flag (INT32 i, INT32 j = 0);
		void SetTriggerPtr (void);
		void ClearObjWindow (void);
		void DrawObjectImage ();
		virtual BOOL TextureIsVisible ();
		bool FindTrigger (INT16 &nTrigger);
		bool TriggerHasSlider (void);
		INT32 NumTriggers ();
		void OnSelect1st ();
		void OnSelect2nd ();
		void SelectTexture (INT32 nIdC, bool bFirst);
		afx_msg void OnPaint ();
		afx_msg void OnAddTrigger ();
		afx_msg void OnDeleteTrigger ();
		afx_msg void OnDeleteTriggerAll ();
		afx_msg void OnSetTrigger ();
		afx_msg void OnSetType ();
		afx_msg void OnSetTarget ();
		afx_msg void OnAddTarget ();
		afx_msg void OnDeleteTarget ();
		afx_msg void OnAddWallTarget ();
		afx_msg void OnAddObjTarget ();
		afx_msg void OnCopyTrigger ();
		afx_msg void OnPasteTrigger ();
		afx_msg void OnStrength ();
		afx_msg void OnTime ();
		afx_msg void OnStandardTrigger ();
		afx_msg void OnObjectTrigger ();
		afx_msg void OnD2Flag1 ();
		afx_msg void OnD2Flag2 ();
		afx_msg void OnD2Flag3 ();
		afx_msg void OnD2Flag4 ();
		afx_msg void OnD2Flag5 ();
		afx_msg void OnD2Flag6 ();
		afx_msg void OnD2Flag7 ();
		afx_msg void OnD1Flag1 ();
		afx_msg void OnD1Flag2 ();
		afx_msg void OnD1Flag3 ();
		afx_msg void OnD1Flag4 ();
		afx_msg void OnD1Flag5 ();
		afx_msg void OnD1Flag6 ();
		afx_msg void OnD1Flag7 ();
		afx_msg void OnD1Flag8 ();
		afx_msg void OnD1Flag9 ();
		afx_msg void OnD1Flag10 ();
		afx_msg void OnD1Flag11 ();
		afx_msg void OnD1Flag12 ();
		afx_msg void OnAddOpenDoor ();
		afx_msg void OnAddRobotMaker ();
		afx_msg void OnAddShieldDrain ();
		afx_msg void OnAddEnergyDrain ();
		afx_msg void OnAddControlPanel ();
		afx_msg void OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
		inline CComboBox *CBTriggerNo ()
			{ return CBCtrl(IDC_TRIGGER_TRIGGERNO); }
		inline CComboBox *CBType ()
			{ return CBCtrl(IDC_TRIGGER_D2TYPE); }
		inline CListBox *LBTargets ()
			{ return (CListBox *) GetDlgItem (IDC_TRIGGER_TARGETLIST); }
		inline CWnd *TargetEdit ()
			{ return GetDlgItem (IDC_TRIGGER_TARGET); }
		inline CScrollBar *SpeedBoostSlider ()
			{ return (CScrollBar *) GetDlgItem (IDC_TRIGGER_SLIDER); }
		inline CComboBox *CBTexture1 ()
			{ return CBCtrl(IDC_TRIGGER_TEXTURE1); }
		inline CComboBox *CBTexture2 ()
			{ return CBCtrl(IDC_TRIGGER_TEXTURE2); }
		inline INT16 Texture1 (void)
			{ return (INT16) ((m_nTrigger >= 0) && (m_pTrigger && (m_nType == TT_CHANGE_TEXTURE)) ? m_pTrigger->value & 0xffff : 0); }
		inline INT16 Texture2 (void)
			{ return (INT16) ((m_nTrigger >= 0) && (m_pTrigger && (m_nType == TT_CHANGE_TEXTURE)) ? m_pTrigger->value >> 16 : 0); }
		inline void SetTexture (INT16 texture1, INT16 texture2) {
			if ((m_nTrigger >= 0) && m_pTrigger && (m_nType == TT_CHANGE_TEXTURE)) {
				if (texture1 < 0)
					texture1 = Texture1 ();
				if (texture2 < 0)
					texture2 = Texture2 ();
				m_pTrigger->value = (FIX) (texture2 << 16) + (FIX) texture1; 
				}
			}

		DECLARE_MESSAGE_MAP ()
};

                        /*--------------------------*/

class CTxtFilterTool : public CToolDlg
{
	public:
		CTxtFilterTool (CPropertySheet *pParent = NULL);
		~CTxtFilterTool ();
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		//virtual BOOL OnSetActive ();
		virtual BOOL OnKillActive ();
		
		void SetFilter (INT32 i);
		void SetFilterRange (UINT32 nFlags, INT32 nValue);

		afx_msg void OnGrayRock ();
		afx_msg void OnBrownRock ();
		afx_msg void OnRedRock ();
		afx_msg void OnYellowRock ();
		afx_msg void OnGreenRock ();
		afx_msg void OnBlueRock ();
		afx_msg void OnIce ();
		afx_msg void OnStones ();
		afx_msg void OnGrass ();
		afx_msg void OnSand ();
		afx_msg void OnLava ();
		afx_msg void OnWater ();
		afx_msg void OnSteel ();
		afx_msg void OnConcrete ();
		afx_msg void OnBricks ();
		afx_msg void OnTarmac ();
		afx_msg void OnWalls ();
		afx_msg void OnFloors ();
		afx_msg void OnCeilings ();
		afx_msg void OnGrates ();
		afx_msg void OnFans ();
		afx_msg void OnLights ();
		afx_msg void OnEnergy ();
		afx_msg void OnForcefields ();
		afx_msg void OnTech ();
		afx_msg void OnLabels ();
		afx_msg void OnMonitors ();
		afx_msg void OnStripes ();
		afx_msg void OnMovers ();
		afx_msg void OnDoors ();
		afx_msg void OnSwitches ();
		afx_msg void OnRockAll ();
		afx_msg void OnRockNone ();
		afx_msg void OnNatureAll ();
		afx_msg void OnNatureNone ();
		afx_msg void OnBuildingAll ();
		afx_msg void OnBuildingNone ();
		afx_msg void OnTechAll ();
		afx_msg void OnTechNone ();
		afx_msg void OnSignAll ();
		afx_msg void OnSignNone ();
		afx_msg void OnOtherAll ();
		afx_msg void OnOtherNone ();
		afx_msg void OnTxtAll ();
		afx_msg void OnTxtNone ();
		afx_msg void OnTxtInvert ();

		DECLARE_MESSAGE_MAP()
};

                        /*--------------------------*/

class CTextureTool : public CTexToolDlg
{
	public:
		CExtBitmapButton	m_btnZoomIn;
		CExtBitmapButton	m_btnZoomOut;
		CExtBitmapButton	m_btnHShrink;
		CExtBitmapButton	m_btnVShrink;
		CExtBitmapButton	m_btnHALeft;
		CExtBitmapButton	m_btnHARight;
		CExtBitmapButton	m_btnVAUp;
		CExtBitmapButton	m_btnVADown;
		CExtBitmapButton	m_btnRALeft;
		CExtBitmapButton	m_btnRARight;
		CBitmapButton		m_btnStretch2Fit;
		CBitmapButton		m_btnReset;
		CBitmapButton		m_btnResetMarked;
		CBitmapButton		m_btnAlignAll;
		CBitmapButton		m_btnChildAlign;
		CBitmapButton		m_btnAddLight;
		CBitmapButton		m_btnDelLight;
		CBitmapButton		m_btnHFlip;
		CBitmapButton		m_btnVFlip;

		CPaletteWnd			m_paletteWnd;

		char					m_szTextureBuf [100];
		INT32					last_texture1,
								last_texture2,
								last_mode;
		INT32					save_texture1,
								save_texture2;
		CUVL					save_uvls[4];
//		INT32					frame [2];
		double				m_lights [4];
//		CWnd					m_textureWnd;
		CWnd					m_alignWnd;
		CWnd					m_lightWnd;
		CWnd					m_colorWnd;
		INT32					m_bUse1st;
		INT32					m_bUse2nd;
		INT32					m_bShowTexture;
		INT32					m_bShowChildren;
		double				m_alignX;
		double				m_alignY;
		double				m_alignAngle;
		double				m_alignUvPoint [4];
		double				m_zoom;
		INT32					m_alignRot2nd;
		POINT					m_apts [4];
		POINT					m_minPt,
								m_maxPt,
								m_centerPt;
		UINT					m_nTimerDelay;
		UINT_PTR				m_nTimer;
		UINT_PTR				m_nEditTimer;
		UINT_PTR				m_nLightTimer;
		INT32					m_nLightDelay;
		double				m_nLightTime;
		INT32					m_nHighlight;
		char					m_szLight [33];
		INT32					m_iLight;
		BOOL					m_bLightEnabled;
		BOOL					m_bIgnorePlane;
		INT32					m_nBrightness;
		INT32					m_nColorIndex;
		PALETTEENTRY		m_rgbColor;
		COLORREF				m_custColors [16];
		INT32					m_nEditFunc;

		CTextureTool (CPropertySheet *pParent = NULL);
		~CTextureTool ();
      virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		virtual BOOL OnKillActive ();
		afx_msg void LoadTextureListBoxes ();
		afx_msg void OnPaint ();
		afx_msg void OnTimer (UINT_PTR nIdEvent);
		afx_msg void OnSaveTexture ();
		afx_msg void OnEditTexture ();
		afx_msg void OnPasteSide ();
		afx_msg void OnPasteTouching ();
		afx_msg void OnPasteMarked ();
		afx_msg void OnReplace ();
		afx_msg void OnSetLight ();
		afx_msg void OnSelect1st ();
		afx_msg void OnSelect2nd ();
		afx_msg void OnPaste1st ();
		afx_msg void OnPaste2nd ();
		afx_msg void OnAlignX ();
		afx_msg void OnAlignY ();
		afx_msg void OnAlignRot ();
		afx_msg void OnAlignLeft ();
		afx_msg void OnAlignRight ();
		afx_msg void OnAlignUp ();
		afx_msg void OnAlignDown ();
		afx_msg void OnAlignRotLeft ();
		afx_msg void OnAlignRotRight ();
		afx_msg void OnHShrink ();
		afx_msg void OnVShrink ();
		afx_msg void OnAlignReset ();
		afx_msg void OnAlignResetMarked ();
		afx_msg void OnAlignStretch2Fit ();
		afx_msg void OnAlignAll ();
		afx_msg void OnAlignChildren ();
		afx_msg void OnZoomIn ();
		afx_msg void OnZoomOut ();
		afx_msg void OnRot2nd0 ();
		afx_msg void OnRot2nd90 ();
		afx_msg void OnRot2nd180 ();
		afx_msg void OnRot2nd270 ();
		afx_msg void OnHFlip ();
		afx_msg void OnVFlip ();
		afx_msg void OnLight1 ();
		afx_msg void OnLight2 ();
		afx_msg void OnLight3 ();
		afx_msg void OnLight4 ();
		afx_msg void OnLight5 ();
		afx_msg void OnLight6 ();
		afx_msg void OnLight7 ();
		afx_msg void OnLight8 ();
		afx_msg void OnLight9 ();
		afx_msg void OnLight10 ();
		afx_msg void OnLight11 ();
		afx_msg void OnLight12 ();
		afx_msg void OnLight13 ();
		afx_msg void OnLight14 ();
		afx_msg void OnLight15 ();
		afx_msg void OnLight16 ();
		afx_msg void OnLight17 ();
		afx_msg void OnLight18 ();
		afx_msg void OnLight19 ();
		afx_msg void OnLight20 ();
		afx_msg void OnLight21 ();
		afx_msg void OnLight22 ();
		afx_msg void OnLight23 ();
		afx_msg void OnLight24 ();
		afx_msg void OnLight25 ();
		afx_msg void OnLight26 ();
		afx_msg void OnLight27 ();
		afx_msg void OnLight28 ();
		afx_msg void OnLight29 ();
		afx_msg void OnLight30 ();
		afx_msg void OnLight31 ();
		afx_msg void OnLight32 ();
		afx_msg void OnLightEdit ();
		afx_msg void OnLightOff ();
		afx_msg void OnLightOn ();
		afx_msg void OnLightStrobe4 ();
		afx_msg void OnLightStrobe8 ();
		afx_msg void OnLightFlicker ();
		afx_msg void OnLightDefault ();
		afx_msg void OnBrightnessEdit ();
		afx_msg void OnLightTimerEdit ();
		afx_msg void OnAddLight ();
		afx_msg void OnDeleteLight ();
		afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
		afx_msg void ChooseRGBColor (void);

		virtual BOOL OnNotify (WPARAM wParam, LPARAM lParam, LRESULT *pResult);
		void OnEditTimer (void);

		void SetLightString ();
		void SetLightButtons (LPSTR szLight = NULL, INT32 nSpeed = -1);
		bool SetLightDelay (INT32 nSpeed = -1);
		void GetBrightness (INT32 nTexture);
		void SetBrightness (INT32 nBrightness = 0);
		void UpdateLightWnd (void);
		void UpdateLight (void);
		void EnableLightControls (BOOL bEnable);

		void ToggleLight (INT32 i);
		void AnimateTexture (void);
		void AnimateLight (void);
		void OnHScroll(UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
		void OnVScroll(UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
		void Rot2nd (INT32 iAngle);
		void HAlign (INT32 dir);
		void VAlign (INT32 dir);
		void RAlign (INT32 dir);
		void HFlip (void);
		void VFlip (void);
		void SelectTexture (INT32 nIdC, bool bFirst);
		void PasteTexture (INT16 nSegment, INT16 nSide, INT16 nDepth);
		bool GetAdjacentSide (INT16 start_segment, INT16 start_side, INT16 linenum,
									 INT16 *neighbor_segnum, INT16 *neighbor_sidenum);
		bool SideHasLight (void);
		void CreateColorCtrl (CWnd *pWnd, INT32 nIdC);
		void UpdateColorCtrl (CWnd *pWnd, COLORREF color);
		void RotateUV (double angle, bool bUpdate = true);
		void Refresh ();
		void RefreshTextureWnd ();
		void RefreshAlignWnd ();
		void UpdateTextureWnd (void);
		void RefreshAlignment ();
		void UpdateAlignWnd (void);
		void UpdatePaletteWnd (void);
		void DrawTexture (INT16 texture1, INT16 texture2, INT32 x0, INT32 y0);
		void DrawAlignment (CDC *pDC);
		INT32 ScrollSpeed (UINT16 texture,INT32 *x,INT32 *y);
		INT32 AlignTextures (INT16 start_segment, INT16 start_side, INT16 only_child);
		void AlignChildTextures (INT32 nSegment, INT32 nSide, INT32 nDepth);
		void AlignChildren (INT16 nSegment, INT16 nSide, bool bStart);
		void SetWallColor (void);

		inline CScrollBar *HScrollAlign ()
			{ return (CScrollBar *) GetDlgItem (IDC_TEXALIGN_HSCROLL); }
		inline CScrollBar *VScrollAlign ()
			{ return (CScrollBar *) GetDlgItem (IDC_TEXALIGN_VSCROLL); }
		inline CComboBox *CBTexture1 ()
			{ return CBCtrl(IDC_TEXTURE_1ST); }
		inline CComboBox *CBTexture2 ()
			{ return CBCtrl(IDC_TEXTURE_2ND); }
		inline CButton *LightButton (INT32 i)
			{ return BtnCtrl (IDC_TEXLIGHT_1 + i); }
		inline CScrollBar *TimerSlider ()
			{ return (CScrollBar *) GetDlgItem (IDC_TEXLIGHT_TIMERSLIDER); }
		inline CScrollBar *BrightnessSlider ()
			{ return (CScrollBar *) GetDlgItem (IDC_TEXTURE_BRIGHTSLIDER); }

		DECLARE_MESSAGE_MAP()
};

                        /*--------------------------*/

class CLightTool : public CToolDlg
{
	public:
		INT32		m_bIlluminate;
		INT32		m_bAvgCornerLight;
		INT32		m_bScaleLight;
		INT32		m_bCubeLight;
		INT32		m_bDynCubeLights;
		INT32		m_bDeltaLight;
		double	m_fBrightness;
		double	m_fLightScale;
		double	m_fCubeLight;
		double	m_fDeltaLight;
		double	m_fVertexLight;
		INT32		m_nNoLightDeltas;
		INT32		m_lightRenderDepth;
		INT32		m_deltaLightRenderDepth;
		INT32		m_deltaLightFrameRate;
		INT32		m_bShowLightSource;
		INT32		m_bCopyTexLights;

		CLightTool (CPropertySheet *pParent = NULL);
      virtual BOOL OnInitDialog ();
		void SetDefaults (void);
		void OnOK (void);
		void OnCancel (void);
      virtual void DoDataExchange (CDataExchange *pDX);
		afx_msg void OnShowDelta ();
		afx_msg void OnShowLightSource ();
		afx_msg void OnSetVertexLight ();
		afx_msg void OnDefaultLightAndColor ();
		afx_msg void OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);

		DECLARE_MESSAGE_MAP()
	};

                        /*--------------------------*/

class CPrefsDlg : public CToolDlg
{
	public:
		CBitmapButton	m_btnBrowseD1PIG;
		CBitmapButton	m_btnBrowseD2PIG;
		CBitmapButton	m_btnBrowseMissions;
		char				m_d1Path [256];
		char				m_d2Path [256];
		char				m_missionsPath [256];
		UINT32			m_mineViewFlags;
		UINT32			m_objViewFlags;
		UINT32			m_texViewFlags;
		INT32				m_iDepthPerception;
		double			m_depthPerceptions [4];
		INT32				m_iRotateRate;
		double			m_rotateRates [5];
		double			m_moveRate;
		INT32				m_bExpertMode;
		INT32				m_bUseTexColors;
		INT32				m_nViewDist;
		INT32				m_nMineCenter;
		INT32				m_nMaxUndo;
		BOOL				m_bSplashScreen;
		bool				m_bNoRefresh;
		bool				m_bInvalid;

		CPrefsDlg (CPropertySheet *pParent = NULL);
		~CPrefsDlg ();
		bool BrowseFile (LPSTR fileType, LPSTR fileName, LPSTR fileExt, BOOL bOpen);
	   virtual BOOL OnInitDialog ();
      virtual void DoDataExchange (CDataExchange *pDX);
		virtual BOOL OnSetActive ();
		virtual void OnOK (void);
		virtual void OnCancel (void);
		afx_msg void OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
		afx_msg void OnOpenD1PIG (void);
		afx_msg void OnOpenD2PIG (void);
		afx_msg void OnOpenMissions (void);
		afx_msg void OnViewMineNone (void);
		afx_msg void OnViewMineAll (void);
		afx_msg void OnViewObjectsNone (void);
		afx_msg void OnViewObjectsAll (void);
		afx_msg void OnLayout0 (void);
		afx_msg void OnLayout1 (void);
		afx_msg void OnSetMineCenter (void);
		void SetLayout (INT32 nLayout);
		void FreeTextureHandles (bool bDeleteModified = true);
		void Refresh (void);
		void WritePrivateProfileInt (LPSTR szKey, INT32 nValue);
		void WritePrivateProfileDouble (LPSTR szKey, double nValue);
		void GetAppSettings ();
		void SetAppSettings (bool bInitApp = false);
		void SaveAppSettings (bool bSaveFolders = true);
		void CompletePath (LPSTR pszPath, LPSTR pszFile, LPSTR pszExt);
		inline CScrollBar *ViewDistSlider ()
			{ return (CScrollBar *) GetDlgItem (IDC_PREFS_VIEWDIST); }
		inline CComboBox *CBMineCenter ()
			{ return CBCtrl(IDC_PREFS_MINECENTER); }

		DECLARE_MESSAGE_MAP()
};

                        /*--------------------------*/

class CToolView : public CWnd {
	private:
		INT32				m_scrollRange [2];
		INT32				m_scrollPage [2];
		INT32				m_scrollOffs [2];
		CSize				m_toolSize;
		BOOL				m_bHScroll;
		BOOL				m_bVScroll;
		BOOL				m_bRecalcLayout;

	public:
		DECLARE_DYNCREATE(CToolView)

		CPropertySheet	*m_pTools;
		CSegmentTool	*m_segmentTool;
		CWallTool		*m_wallTool;
		CTriggerTool	*m_triggerTool;
		CTextureTool	*m_textureTool;
		CObjectTool		*m_objectTool;
		CEffectTool		*m_effectTool;
		CAdvObjTool		*m_advObjTool;
		CLightTool		*m_lightTool;
		CPrefsDlg		*m_prefsDlg;
		CMissionTool	*m_missionTool;
		CReactorTool	*m_reactorTool;
		CDiagTool		*m_diagTool;
		CTxtFilterTool	*m_txtFilterTool;

		CToolView ();
		~CToolView ();
		void Setup ();
		void Reset () {};
		afx_msg INT32 OnCreate (LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnDestroy (void);
		afx_msg void OnPaint ();
		afx_msg BOOL OnEraseBkgnd (CDC* pDC);
		afx_msg void OnHScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnVScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg void OnSelectPrevTab ();
		afx_msg void OnSelectNextTab ();
		void CalcToolSize (void);
		void RecalcLayout (INT32 nToolMode = 0, INT32 nTextureMode = 0);
		void MoveWindow (INT32 x, INT32 y, INT32 nWidth, INT32 nHeight, BOOL bRepaint = TRUE);
		void MoveWindow (LPCRECT lpRect, BOOL bRepaint = TRUE);
		inline CSize& ToolSize ()
			{ return m_toolSize; }
		void SetActive (INT32 nPage);
		inline void EditTexture ()
			{ SetActive (0); }
		inline void EditCube ()
			{ SetActive (1); }
		inline void EditWall ()
			{ SetActive (2); }
		inline void EditTrigger ()
			{ SetActive (3); }
		inline void EditObject ()
			{ SetActive (4); }
		inline void EditEffect ()
			{ SetActive (4); }
		inline void EditAdvObj ()
			{ SetActive (6); }
		inline void EditLight ()
			{ SetActive (7); }
		inline void EditReactor ()
			{ SetActive (8); }
		inline void EditMission ()
			{ SetActive (9); }
		inline void EditDiag ()
			{ SetActive (10); }
		inline void EditTxtFilters ()
			{ SetActive (11); }
		inline void EditPrefs ()
			{ SetActive (12); }
		inline CSegmentTool *CubeTool ()
			{ return m_segmentTool; }
		inline CTextureTool *TextureTool ()
			{ return m_textureTool; }
		inline CWallTool *WallTool ()
			{ return m_wallTool; }
		inline CTriggerTool *TriggerTool ()
			{ return m_triggerTool; }
		inline CLightTool *LightTool ()
			{ return m_lightTool; }
		inline CObjectTool *ObjectTool ()
			{ return m_objectTool; }
		inline CMissionTool *MissionTool ()
			{ return m_missionTool; }
		inline CDiagTool *DiagTool ()
			{ return m_diagTool; }
		inline CTxtFilterTool *TextureFilter ()
			{ return m_txtFilterTool; }
		inline CPrefsDlg *PrefsDlg ()
			{ return m_prefsDlg; }
		void Refresh ();
		void CycleTab (INT32 nDir);
		void NextTab ();
		void PrevTab ();
	DECLARE_MESSAGE_MAP()
};
                        
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif //__toolview_h
