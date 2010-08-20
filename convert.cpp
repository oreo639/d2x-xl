// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dle-xp.h"
#include "mine.h"
#include "global.h"
#include "dlcdoc.h"
#include "textures.h"

BEGIN_MESSAGE_MAP (CConvertDlg, CDialog)
	ON_WM_PAINT ()
	ON_CBN_SELCHANGE (IDC_CONVERT_D1, OnSetD1)
	ON_CBN_SELCHANGE (IDC_CONVERT_D2, OnSetD2)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------
// DIALOG - CConvertDlg (constructor)
//------------------------------------------------------------------------

CConvertDlg::CConvertDlg (CWnd *pParent)
	: CDialog (IDD_CONVERT, pParent)
{
m_bInited = false;
}

//------------------------------------------------------------------------
// CConvertDlg - ~CConvertDlg (destructor)
//------------------------------------------------------------------------

void CConvertDlg::EndDialog (INT32 nResult) 
{
if (m_bInited) {
	m_showD1.DestroyWindow ();
	m_showD2.DestroyWindow ();
	}
CDialog::EndDialog (nResult);
}

                        /*--------------------------*/

void CConvertDlg::CreateImgWnd (CWnd *pImgWnd, INT32 nIdC)
{
CWnd *pParentWnd = GetDlgItem (nIdC);
CRect rc;
pParentWnd->GetClientRect (rc);
pImgWnd->Create (NULL, NULL, WS_CHILD | WS_VISIBLE, rc, pParentWnd, 0);
}

//------------------------------------------------------------------------
// CConvertDlg - SetupWindow
//------------------------------------------------------------------------

BOOL CConvertDlg::OnInitDialog () 
{
if (!theMine) return FALSE;

CDialog::OnInitDialog ();

CreateImgWnd (&m_showD1, IDC_CONVERT_SHOWD1);
CreateImgWnd (&m_showD2, IDC_CONVERT_SHOWD2);

m_hInst = AfxGetApp()->m_hInstance;
HRSRC hFind = FindResource (m_hInst, MAKEINTRESOURCE (IDR_TEXTURE_D1D2), "RC_DATA");
if (!hFind)
	return FALSE;
m_hTextures = LoadResource (m_hInst, hFind);
if (!m_hTextures)
	return FALSE;
m_pTextures = (INT16 *) LockResource (m_hTextures);
if (!m_pTextures)
	return FALSE;

CComboBox *pcb = CBD1 ();
INT16	nSeg,	nSide, nTextures;
INT16 tnum [2], segCount = theMine->SegCount ();
char	szName [80];
INT32 h;
CSegment *segP = theMine->Segments (0);
CSide *sideP;
// add textures that have been used to Texture 1 combo box
for (nSeg = segCount; nSeg; nSeg--, segP++) {
	for (sideP = segP->sides, nSide = 6; nSide; nSide--, sideP++) {
		tnum [0] = sideP->nBaseTex;
		tnum [1] = sideP->nOvlTex & 0x1fff;
		INT32 i;
		for (i = 0; i < 2; i++) {
			if (tnum [i] != -1) {
				// read name of texture from Descent 1 texture resource
				LoadString (m_hInst, D1_TEXTURE_STRING_TABLE + tnum [i], szName, sizeof (szName));
				// if not already in list box, add it
				if (pcb->FindStringExact (-1, szName) < 0) {
					h = pcb->AddString (szName);
					pcb->SetItemData (h, tnum [i]);
					}
				}
			}
		}
	}
pcb->SetCurSel (0);

  // add complete set for Texture 2 combo box
nTextures = theApp.IsD1File () ? MAX_D1_TEXTURES : MAX_D2_TEXTURES;
pcb = CBD2 ();
INT32 i;
for (i = 0; i < nTextures; i++) {
// read szName of texture from Descent 2 texture resource
	LoadString (m_hInst, D2_TEXTURE_STRING_TABLE + i, szName, sizeof (szName));
	if (*szName != '*') {
		h = pcb->AddString (szName);
		pcb->SetItemData (h, i);
		}
	}
m_bInited = true;
Refresh ();
return TRUE;
}

void CConvertDlg::DoDataExchange (CDataExchange *pDX)
{
Refresh ();
}

//------------------------------------------------------------------------
// CConvertDlg - RefreshData
//------------------------------------------------------------------------

void CConvertDlg::Refresh () 
{
if (!(m_bInited && theMine))
	return;

	INT16 texture1,texture2;
	INT16 fileTypeBackup;
#ifdef _DEBUG
	DWORD nError;
#endif

// find matching entry for Texture 1
//  CBD1 ()->GetSelString (message,sizeof (message));
//  texture1 = GetTextureID (message);
texture1 = (INT16) (CBD1 ()->GetItemData (CBD1 ()->GetCurSel ()));
texture2 = m_pTextures [texture1];
if (LoadString (m_hInst, D2_TEXTURE_STRING_TABLE + texture2, message, sizeof (message)))
	CBD2 ()->SelectString (-1, message);
#ifdef _DEBUG
else
	nError = GetLastError ();
#endif
fileTypeBackup = theApp.FileType ();
theMine->SetFileType (RL2_FILE);
theMine->LoadPalette ();
PaintTexture (&m_showD2, 0, -1, -1, texture2, 0);
theMine->SetFileType (RDL_FILE);
theMine->LoadPalette ();
PaintTexture (&m_showD1, 0, -1, -1, texture1, 0);

  // restore file type (should always be RDL_TYPE)
  theMine->SetFileType (fileTypeBackup);
}

//------------------------------------------------------------------------
// CConvertDlg - Sent if we need to redraw an item
//------------------------------------------------------------------------

void CConvertDlg::OnPaint ()
{
CDialog::OnPaint ();
Refresh ();
}

//------------------------------------------------------------------------
// CConvertDlg - Ok
//------------------------------------------------------------------------
/*
void CConvertDlg::OnOK ()
{
Convert ();
CDialog::OnOK ();
}
*/
//------------------------------------------------------------------------
// CConvertDlg - Texture1 Msg
//------------------------------------------------------------------------

void CConvertDlg::OnSetD1 ()
{
Refresh ();
}

//------------------------------------------------------------------------
// CConvertDlg - Texture1 Msg
//------------------------------------------------------------------------

void CConvertDlg::OnSetD2 ()
{
INT16 texture1 = (INT16) (CBD1 ()->GetItemData (CBD1 ()->GetCurSel ()));
INT16 texture2 = (INT16) (CBD2 ()->GetItemData (CBD2 ()->GetCurSel ()));
m_pTextures [texture1] = texture2;
Refresh ();
}

//------------------------------------------------------------------------
// CConvertDlg - Convert
//------------------------------------------------------------------------

void CConvertDlg::OnOK () 
{
if (!theMine) return;

  INT16		i,j;
  CSegment *segP;
  CSide		*sideP;
  CWall		*wallP;
  CTrigger	*trigP;
  CGameObject	*objP;
  INT16		nSegment, nSide, d1Texture, mode,
				segCount = theMine->SegCount (),
				wallCount = theMine->GameInfo ().walls.count;

FreeTextureHandles ();
theApp.ResetUndoBuffer ();	//no undo possible; palette changes to difficult to handle
// reload internal stuff for d2
theMine->SetFileType (RL2_FILE);
texture_resource = D2_TEXTURE_STRING_TABLE;
theMine->LoadPalette ();

  // convert textures
for (nSegment = 0, segP = theMine->Segments (0); nSegment < segCount; nSegment++, segP++) {
	segP->s2_flags = 0;
	for (nSide = 0, sideP = segP->sides; nSide < 6; nSide++) {
		if ((segP->children [nSide] == -1) || (segP->sides [nSide].nWall < wallCount)) {
			d1Texture = segP->sides [nSide].nBaseTex;
			if ((d1Texture >= 0) && (d1Texture < MAX_D1_TEXTURES))
				segP->sides[nSide].nBaseTex = m_pTextures [d1Texture];
			else { 
				DEBUGMSG (" Level converter: Invalid texture 1 found")
				segP->sides [nSide].nBaseTex = 0;
				}
			d1Texture = segP->sides [nSide].nOvlTex & 0x1fff;
			mode = segP->sides[nSide].nOvlTex & 0xc000;
			if (d1Texture > 0 && d1Texture < MAX_D1_TEXTURES)
				segP->sides [nSide].nOvlTex = m_pTextures [d1Texture] | mode;
			else if (d1Texture < 0) {
				DEBUGMSG (" Level converter: Invalid texture 2 found")
				segP->sides [nSide].nOvlTex = 0;
				}
			}
		}
	}

// defined D2 wall parameters
//--------------------------------------
for (i = 0, wallP = theMine->Walls (0); i < wallCount; i++, wallP++) {
	wallP->controlling_trigger = 0;
	wallP->cloak_value = 0;
	}

// change trigP type and flags
//-------------------------------------------
for (i = 0, trigP = theMine->Triggers (0); i < theMine->GameInfo ().triggers.count; i++, trigP++) {
	switch (trigP->flags) {
		case TRIGGER_CONTROL_DOORS:
			trigP->type = TT_OPEN_DOOR;
			break;
		case TRIGGER_EXIT:
			trigP->type = TT_EXIT;
			break;
		case TRIGGER_MATCEN:
			trigP->type = TT_MATCEN;
			break;
		case TRIGGER_ILLUSION_OFF:
			trigP->type = TT_ILLUSION_OFF;
			break;
		case TRIGGER_ILLUSION_ON:
			trigP->type = TT_ILLUSION_ON;
			break;
		case TRIGGER_SECRET_EXIT:
			trigP->type = TT_SECRET_EXIT;
			break;

		// unsupported types
		case TRIGGER_ON:
		case TRIGGER_ONE_SHOT:
		case TRIGGER_SHIELD_DAMAGE:
		case TRIGGER_ENERGY_DRAIN:
		default:
			DEBUGMSG (" Level converter: Unsupported trigP type; trigP deleted")
			theMine->DeleteTrigger (i);
			i--;
			trigP--;
			continue;
		}
	trigP->flags = 0;
	}

// set robot_center nFuelCen and robot_flags2
//-----------------------------------------------
for (i = 0; i < theMine->GameInfo ().botgen.count; i++) {
	theMine->BotGens (i)->objFlags [1] = 0;
	for (j = 0, segP = theMine->Segments (0); j <= segCount; j++, segP++)
		if ((segP->function == SEGMENT_FUNC_ROBOTMAKER) && (segP->nMatCen == i))
				theMine->BotGens (i)->nFuelCen = (INT16)(segP->value);
	}

// set equip_center nFuelCen and robot_flags2
//-----------------------------------------------
for (i = 0; i < theMine->GameInfo ().equipgen.count; i++) {
	theMine->EquipGens (i)->objFlags [1] = 0;
	for (j = 0, segP = theMine->Segments (0); j <= segCount; j++, segP++)
		if ((segP->function == SEGMENT_FUNC_EQUIPMAKER) && (segP->nMatCen == i))
				theMine->EquipGens (i)->nFuelCen = (INT16)(segP->value);
	}

// Objects ()
//-----------------------------------------------

for (i = 0, objP = theMine->Objects (0); i < theMine->GameInfo ().objects.count; i++, objP++) {
// fix clip numbers for poly Objects () (except robots)
	switch (objP->type) {
		case OBJ_PLAYER   : // the player on the console
			objP->rType.polyModelInfo.model_num = D2_PLAYER_CLIP_NUMBER;
			break;
		case OBJ_CNTRLCEN : // the control center
			objP->rType.polyModelInfo.model_num = D2_REACTOR_CLIP_NUMBER;
			break;
		case OBJ_COOP     : // a cooperative player object
			objP->rType.polyModelInfo.model_num = D2_COOP_CLIP_NUMBER;
			break;
		}
	}

// d2 light data and indicies
//--------------------------------------------
theMine->GameInfo ().lightDeltaIndices.count = 0;
theMine->GameInfo ().lightDeltaValues.count = 0;
theMine->FlickerLightCount () = 0;
theMine->AutoAdjustLight (50.0, true);
theMine->CalcAverageCornerLight (true);
theMine->ScaleCornerLight (100.0, true);
theMine->SetCubeLight (50.0, true);
theMine->CalcDeltaLightData (50.0, (INT32) true);

// d2 reactor and secret cube
//----------------------------------------------
theMine->ReactorTime () = 0x1e;
theMine->ReactorStrength () = 0xffffffffL;
theMine->SecretCubeNum () = 0L;

theMine->SecretOrient ().rvec.x = F1_0;
theMine->SecretOrient ().rvec.y = 0L;
theMine->SecretOrient ().rvec.z = 0L;

theMine->SecretOrient ().uvec.x = 0L;
theMine->SecretOrient ().uvec.y = 0L;
theMine->SecretOrient ().uvec.z = F1_0;

theMine->SecretOrient ().fvec.x = 0L;
theMine->SecretOrient ().fvec.y = F1_0;
theMine->SecretOrient ().fvec.z = 0L;
theApp.MineView ()->ResetView (true);
CDialog::OnOK ();
}

// eof convert.cpp