// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <stdio.h>
#include <string.h>

#include "mine.h"
#include "dle-xp.h"
#include "dlcdoc.h"
#include "PaletteManager.h"
#include "TextureManager.h"

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

void CConvertDlg::EndDialog (int nResult) 
{
if (m_bInited) {
	m_showD1.DestroyWindow ();
	m_showD2.DestroyWindow ();
	}
CDialog::EndDialog (nResult);
}

//------------------------------------------------------------------------------

void CConvertDlg::CreateImgWnd (CWnd *pImgWnd, int nIdC)
{
CWnd *pParentWnd = GetDlgItem (nIdC);
CRect rc;
pParentWnd->GetClientRect (rc);
pImgWnd->Create (null, null, WS_CHILD | WS_VISIBLE, rc, pParentWnd, 0);
}

//------------------------------------------------------------------------
// CConvertDlg - SetupWindow
//------------------------------------------------------------------------

BOOL CConvertDlg::OnInitDialog () 
{
CDialog::OnInitDialog ();

CreateImgWnd (&m_showD1, IDC_CONVERT_SHOWD1);
CreateImgWnd (&m_showD2, IDC_CONVERT_SHOWD2);

if (!(m_pTextures = (short *) m_res.Load (IDR_TEXTURE_D1D2)))
	return FALSE;

CComboBox *pcb = CBD1 ();
short	nSeg,	nSide, nTextures;
short tnum [2], segCount = segmentManager.Count ();
char	szName [80];
int h;
CSegment *segP = segmentManager.Segment (0);
CSide *sideP;
// add textures that have been used to Texture 1 combo box
for (nSeg = segCount; nSeg; nSeg--, segP++) {
	for (sideP = segP->m_sides, nSide = 6; nSide; nSide--, sideP++) {
		tnum [0] = sideP->m_info.nBaseTex;
		tnum [1] = sideP->m_info.nOvlTex & 0x1fff;
		int i;
		for (i = 0; i < 2; i++) {
			if (tnum [i] != -1) {
				// read name of texture from Descent 1 texture resource
				LoadString (m_hInst, TEXTURE_STRING_TABLE_D1 + tnum [i], szName, sizeof (szName));
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
nTextures = DLE.IsD1File () ? MAX_TEXTURES_D1 : MAX_TEXTURES_D2;
pcb = CBD2 ();
int i;
for (i = 0; i < nTextures; i++) {
// read szName of texture from Descent 2 texture resource
	LoadString (m_hInst, TEXTURE_STRING_TABLE_D2 + i, szName, sizeof (szName));
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

	short texture1,texture2;
	short fileTypeBackup;
#ifdef _DEBUG
	DWORD nError;
#endif

// find matching entry for Texture 1
//  CBD1 ()->GetSelString (message,sizeof (message));
//  texture1 = GetTextureID (message);
texture1 = (short) (CBD1 ()->GetItemData (CBD1 ()->GetCurSel ()));
texture2 = m_pTextures [texture1];
if (LoadString (m_hInst, TEXTURE_STRING_TABLE_D2 + texture2, message, sizeof (message)))
	CBD2 ()->SelectString (-1, message);
#ifdef _DEBUG
else
	nError = GetLastError ();
#endif
fileTypeBackup = DLE.FileType ();
theMine->SetFileType (RL2_FILE);
paletteManager.Reload ();
PaintTexture (&m_showD2, 0, -1, -1, texture2, 0);
theMine->SetFileType (RDL_FILE);
paletteManager.Reload ();
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
short texture1 = (short) (CBD1 ()->GetItemData (CBD1 ()->GetCurSel ()));
short texture2 = (short) (CBD2 ()->GetItemData (CBD2 ()->GetCurSel ()));
m_pTextures [texture1] = texture2;
Refresh ();
}

//------------------------------------------------------------------------
// CConvertDlg - Convert
//------------------------------------------------------------------------

void CConvertDlg::OnOK () 
{
CHECKMINE;

  short			i,j;
  CSegment		*segP;
  CSide			*sideP;
  CWall			*wallP;
  CTrigger		*trigP;
  CGameObject	*objP;
  short			nSegment, nSide, d1Texture, mode,
					segCount = segmentManager.Count (),
					wallCount = wallManager.WallCount ();

textureManager.Release ();
undoManager.Reset ();	//no undo possible; palette changes to difficult to handle
// reload internal stuff for d2
theMine->SetFileType (RL2_FILE);
paletteManager.Reload ();

  // convert textures
for (nSegment = 0, segP = segmentManager.Segment (0); nSegment < segCount; nSegment++, segP++) {
	for (nSide = 0, sideP = segP->m_sides; nSide < 6; nSide++) {
		if ((segP->Child (nSide) == -1) || (segP->m_sides [nSide].m_info.nWall < wallCount)) {
			d1Texture = segP->m_sides [nSide].m_info.nBaseTex;
			if ((d1Texture >= 0) && (d1Texture < MAX_TEXTURES_D1))
				segP->m_sides[nSide].m_info.nBaseTex = m_pTextures [d1Texture];
			else { 
				DEBUGMSG (" Level converter: Invalid texture 1 found")
				segP->m_sides [nSide].m_info.nBaseTex = 0;
				}
			d1Texture = segP->m_sides [nSide].m_info.nOvlTex & 0x1fff;
			mode = segP->m_sides[nSide].m_info.nOvlTex & 0xc000;
			if (d1Texture > 0 && d1Texture < MAX_TEXTURES_D1)
				segP->m_sides [nSide].m_info.nOvlTex = m_pTextures [d1Texture] | mode;
			else if (d1Texture < 0) {
				DEBUGMSG (" Level converter: Invalid texture 2 found")
				segP->m_sides [nSide].m_info.nOvlTex = 0;
				}
			}
		}
	}

// defined D2 wall parameters
//--------------------------------------
for (i = 0, wallP = wallManager.Wall (0); i < wallCount; i++, wallP++) {
	wallP->Info ().controllingTrigger = 0;
	wallP->Info ().cloakValue = 0;
	}

// change trigP type and flags
//-------------------------------------------
for (i = 0, trigP = triggerManager.Trigger (0); i < triggerManager.WallTriggerCount (); i++, trigP++) {
	switch (trigP->Info ().flags) {
		case TRIGGER_CONTROL_DOORS:
			trigP->Type () = TT_OPEN_DOOR;
			break;
		case TRIGGER_EXIT:
			trigP->Type () = TT_EXIT;
			break;
		case TRIGGER_MATCEN:
			trigP->Type () = TT_MATCEN;
			break;
		case TRIGGER_ILLUSION_OFF:
			trigP->Type () = TT_ILLUSION_OFF;
			break;
		case TRIGGER_ILLUSION_ON:
			trigP->Type () = TT_ILLUSION_ON;
			break;
		case TRIGGER_SECRET_EXIT:
			trigP->Type () = TT_SECRET_EXIT;
			break;

		// unsupported types
		case TRIGGER_ON:
		case TRIGGER_ONE_SHOT:
		case TRIGGER_SHIELD_DAMAGE:
		case TRIGGER_ENERGY_DRAIN:
		default:
			DEBUGMSG (" Level converter: Unsupported trigP type; trigP deleted")
			triggerManager.Delete (i);
			i--;
			trigP--;
			continue;
		}
	trigP->Info ().flags = 0;
	}

// set robot_center nFuelCen and robot_flags2
//-----------------------------------------------
for (i = 0; i < segmentManager.RobotMakerCount (); i++) {
	segmentManager.RobotMaker (i)->m_info.objFlags [1] = 0;
	for (j = 0, segP = segmentManager.Segment (0); j <= segCount; j++, segP++)
		if ((segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER) && (segP->m_info.nMatCen == i))
				segmentManager.RobotMaker (i)->m_info.nFuelCen = (short)(segP->m_info.value);
	}

// set equip_center nFuelCen and robot_flags2
//-----------------------------------------------
for (i = 0; i < segmentManager.EquipMakerCount (); i++) {
	segmentManager.EquipMaker (i)->m_info.objFlags [1] = 0;
	for (j = 0, segP = segmentManager.Segment (0); j <= segCount; j++, segP++)
		if ((segP->m_info.function == SEGMENT_FUNC_EQUIPMAKER) && (segP->m_info.nMatCen == i))
				segmentManager.EquipMaker (i)->m_info.nFuelCen = (short)(segP->m_info.value);
	}

// Objects ()
//-----------------------------------------------

for (i = 0, objP = objectManager.Object (0); i < objectManager.Count (); i++, objP++) {
// int clip numbers for poly Objects () (except robots)
	switch (objP->Type ()) {
		case OBJ_PLAYER: // the player on the console
			objP->rType.polyModelInfo.nModel = D2_PLAYER_CLIP_NUMBER;
			break;
		case OBJ_CNTRLCEN: // the control center
			objP->rType.polyModelInfo.nModel = D2_REACTOR_CLIP_NUMBER;
			break;
		case OBJ_COOP: // a cooperative player object
			objP->rType.polyModelInfo.nModel = D2_COOP_CLIP_NUMBER;
			break;
		}
	}

// d2 light data and indicies
//--------------------------------------------
lightManager.DeltaIndexCount () = 0;
lightManager.DeltaValueCount () = 0;
lightManager.Count () = 0;
lightManager.ComputeStaticLight (50.0, true);
lightManager.CalcAverageCornerLight (true);
lightManager.ScaleCornerLight (100.0, true);
lightManager.SetSegmentLight (50.0, true);
lightManager.ComputeVariableLight (50.0, 1);

// d2 reactor and secret segment
//----------------------------------------------
triggerManager.ReactorTime () = 0x1e;
theMine->ReactorStrength () = 0xffffffffL;
objectManager.SecretSegment () = 0L;

objectManager.SecretOrient ().Set (1, 0, 0, 0, 0, 1, 0, 1, 0);
DLE.MineView ()->ResetView (true);
CDialog::OnOK ();
}

// eof convert.cpp