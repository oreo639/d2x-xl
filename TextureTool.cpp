// dlcView.cpp : implementation of the CMineView class
//

#include <math.h>
#include "stdafx.h"
#include "afxpriv.h"
#include "dle-xp.h"

#include "dlcDoc.h"
#include "mineview.h"
#include "toolview.h"

#include "PaletteManager.h"
#include "textures.h"
#include "global.h"
#include "texedit.h"
#include "FileManager.h"
#include "TextureManager.h"
#include "AVLTree.h"
#include "SLL.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE [] = __FILE__;
#endif

#define TEXTOOLDLG 1

/////////////////////////////////////////////////////////////////////////////
// CToolView

BEGIN_MESSAGE_MAP(CTextureTool, CTexToolDlg)
	ON_WM_PAINT ()
	ON_WM_TIMER ()
	ON_WM_HSCROLL ()
	ON_WM_VSCROLL ()
	ON_WM_LBUTTONDOWN ()
	ON_BN_CLICKED (IDC_TEXTURE_SHOWFRAMES, LoadTextureListBoxes)
	ON_BN_CLICKED (IDC_TEXTURE_EDIT, OnEditTexture)
	ON_BN_CLICKED (IDC_TEXTURE_COPY, OnSaveTexture)
	ON_BN_CLICKED (IDC_TEXTURE_PASTESIDE, OnPasteSide)
	ON_BN_CLICKED (IDC_TEXTURE_PASTETOUCHING, OnPasteTouching)
	ON_BN_CLICKED (IDC_TEXTURE_TAG_PLANE, OnTagPlane)
	ON_BN_CLICKED (IDC_TEXTURE_REPLACE, OnReplace)
	ON_BN_CLICKED (IDC_TEXTURE_PASTE1ST, OnPaste1st)
	ON_BN_CLICKED (IDC_TEXTURE_PASTE2ND, OnPaste2nd)
	ON_BN_CLICKED (IDC_TEXTURE_CLEANUP, OnCleanup)
	ON_BN_CLICKED (IDC_TEXALIGN_HALEFT, OnAlignLeft)
	ON_BN_CLICKED (IDC_TEXALIGN_HARIGHT, OnAlignRight)
	ON_BN_CLICKED (IDC_TEXALIGN_VAUP, OnAlignUp)
	ON_BN_CLICKED (IDC_TEXALIGN_VADOWN, OnAlignDown)
	ON_BN_CLICKED (IDC_TEXALIGN_RALEFT, OnAlignRotLeft)
	ON_BN_CLICKED (IDC_TEXALIGN_RARIGHT, OnAlignRotRight)
	ON_BN_CLICKED (IDC_TEXALIGN_HFLIP, OnHFlip)
	ON_BN_CLICKED (IDC_TEXALIGN_VFLIP, OnVFlip)
	ON_BN_CLICKED (IDC_TEXALIGN_HSHRINK, OnHShrink)
	ON_BN_CLICKED (IDC_TEXALIGN_VSHRINK, OnVShrink)
	ON_BN_CLICKED (IDC_TEXALIGN_RESET, OnAlignReset)
	ON_BN_CLICKED (IDC_TEXALIGN_RESET_TAGGED, OnAlignResetTagged)
	ON_BN_CLICKED (IDC_TEXALIGN_STRETCH2FIT, OnAlignStretch2Fit)
	ON_BN_CLICKED (IDC_TEXALIGN_CHILDALIGN, OnAlignChildren)
	ON_BN_CLICKED (IDC_TEXALIGN_ALIGNALL, OnAlignAll)
	ON_BN_CLICKED (IDC_TEXALIGN_ZOOMIN, OnZoomIn)
	ON_BN_CLICKED (IDC_TEXALIGN_ZOOMOUT, OnZoomOut)
	ON_BN_CLICKED (IDC_TEXALIGN_ROT0, OnRot2nd0)
	ON_BN_CLICKED (IDC_TEXALIGN_ROT90, OnRot2nd90)
	ON_BN_CLICKED (IDC_TEXALIGN_ROT180, OnRot2nd180)
	ON_BN_CLICKED (IDC_TEXALIGN_ROT270, OnRot2nd270)
	ON_BN_CLICKED (IDC_TEXALIGN_IGNOREPLANE, OnAlignIgnorePlane)
	ON_BN_CLICKED (IDC_TEXLIGHT_1, OnLight1)
	ON_BN_CLICKED (IDC_TEXLIGHT_2, OnLight2)
	ON_BN_CLICKED (IDC_TEXLIGHT_3, OnLight3)
	ON_BN_CLICKED (IDC_TEXLIGHT_4, OnLight4)
	ON_BN_CLICKED (IDC_TEXLIGHT_5, OnLight5)
	ON_BN_CLICKED (IDC_TEXLIGHT_6, OnLight6)
	ON_BN_CLICKED (IDC_TEXLIGHT_7, OnLight7)
	ON_BN_CLICKED (IDC_TEXLIGHT_8, OnLight8)
	ON_BN_CLICKED (IDC_TEXLIGHT_9, OnLight9)
	ON_BN_CLICKED (IDC_TEXLIGHT_10, OnLight10)
	ON_BN_CLICKED (IDC_TEXLIGHT_11, OnLight11)
	ON_BN_CLICKED (IDC_TEXLIGHT_12, OnLight12)
	ON_BN_CLICKED (IDC_TEXLIGHT_13, OnLight13)
	ON_BN_CLICKED (IDC_TEXLIGHT_14, OnLight14)
	ON_BN_CLICKED (IDC_TEXLIGHT_15, OnLight15)
	ON_BN_CLICKED (IDC_TEXLIGHT_16, OnLight16)
	ON_BN_CLICKED (IDC_TEXLIGHT_17, OnLight17)
	ON_BN_CLICKED (IDC_TEXLIGHT_18, OnLight18)
	ON_BN_CLICKED (IDC_TEXLIGHT_19, OnLight19)
	ON_BN_CLICKED (IDC_TEXLIGHT_20, OnLight20)
	ON_BN_CLICKED (IDC_TEXLIGHT_21, OnLight21)
	ON_BN_CLICKED (IDC_TEXLIGHT_22, OnLight22)
	ON_BN_CLICKED (IDC_TEXLIGHT_23, OnLight23)
	ON_BN_CLICKED (IDC_TEXLIGHT_24, OnLight24)
	ON_BN_CLICKED (IDC_TEXLIGHT_25, OnLight25)
	ON_BN_CLICKED (IDC_TEXLIGHT_26, OnLight26)
	ON_BN_CLICKED (IDC_TEXLIGHT_27, OnLight27)
	ON_BN_CLICKED (IDC_TEXLIGHT_28, OnLight28)
	ON_BN_CLICKED (IDC_TEXLIGHT_29, OnLight29)
	ON_BN_CLICKED (IDC_TEXLIGHT_30, OnLight30)
	ON_BN_CLICKED (IDC_TEXLIGHT_31, OnLight31)
	ON_BN_CLICKED (IDC_TEXLIGHT_32, OnLight32)
	ON_BN_CLICKED (IDC_TEXLIGHT_OFF, OnLightOff)
	ON_BN_CLICKED (IDC_TEXLIGHT_ON, OnLightOn)
	ON_BN_CLICKED (IDC_TEXLIGHT_STROBE4, OnLightStrobe4)
	ON_BN_CLICKED (IDC_TEXLIGHT_STROBE8, OnLightStrobe8)
	ON_BN_CLICKED (IDC_TEXLIGHT_FLICKER, OnLightFlicker)
	ON_BN_CLICKED (IDC_TEXLIGHT_ADD, OnAddLight)
	ON_BN_CLICKED (IDC_TEXLIGHT_DELETE, OnDeleteLight)
	ON_BN_CLICKED (IDC_TEXLIGHT_RGBCOLOR, OnSelectColor)
	ON_CBN_SELCHANGE (IDC_TEXTURE_1ST, OnSelect1st)
	ON_CBN_SELCHANGE (IDC_TEXTURE_2ND, OnSelect2nd)
	ON_EN_KILLFOCUS (IDC_TEXTURE_LIGHT1, OnSetLight)
	ON_EN_KILLFOCUS (IDC_TEXTURE_LIGHT2, OnSetLight)
	ON_EN_KILLFOCUS (IDC_TEXTURE_LIGHT3, OnSetLight)
	ON_EN_KILLFOCUS (IDC_TEXTURE_LIGHT4, OnSetLight)
	ON_EN_KILLFOCUS (IDC_TEXALIGN_HALIGN, OnAlignX)
	ON_EN_KILLFOCUS (IDC_TEXALIGN_VALIGN, OnAlignY)
	ON_EN_KILLFOCUS (IDC_TEXALIGN_RALIGN, OnAlignRot)
	ON_EN_KILLFOCUS (IDC_TEXTURE_BRIGHTNESS, OnBrightnessEdit)
	ON_EN_KILLFOCUS (IDC_TEXLIGHT_TIMER, OnLightTimerEdit)
	ON_EN_KILLFOCUS (IDC_TEXLIGHT_EDIT, OnLightEdit)
END_MESSAGE_MAP()

//------------------------------------------------------------------------------

CTextureTool::CTextureTool (CPropertySheet *pParent)
	: CTexToolDlg (IDD_TEXTUREDATA_HORZ + nLayout, pParent, 1, IDC_TEXTURE_SHOW, RGB (0,0,0))
{
m_lastTexture [0] = -1;
m_lastTexture [1] = 0;
m_saveTexture [0] = -1;
m_saveTexture [1] = 0;
int i;
for (i = 0; i < 4; i++)
	m_saveUVLs [i].l = defaultUVLs [i].l;
#if TEXTOODLG == 0
m_frame [0] = 0;
m_frame [1] = 0;
#endif
m_bUse1st = TRUE;
m_bUse2nd = FALSE;
m_bShowChildren = TRUE;
m_bShowTexture = TRUE;
m_zoom = 1.0;
m_alignX = 0;
m_alignY = 0;
m_alignAngle = 0;
m_alignRot2nd = 0;
m_nLightDelay = 1000;
m_nLightTime = 1.0;
m_iLight = -1;
m_nHighlight = -1;
*m_szTextureBuf = '\0';
memset (m_szLight, 0, sizeof (m_szLight));
m_bLightEnabled = TRUE;
m_bIgnorePlane = FALSE;
m_bIgnoreWalls = TRUE;
m_nColorIndex = -1;
m_nEditFunc = -1;
m_bInitTextureListBoxes = true;
}

//------------------------------------------------------------------------------

CTextureTool::~CTextureTool ()
{
if (m_bInited) {
	if (IsWindow (m_textureWnd))
		m_textureWnd.DestroyWindow ();
	if (IsWindow (m_alignWnd))
		m_alignWnd.DestroyWindow ();
	if (IsWindow (m_lightWnd))
		m_lightWnd.DestroyWindow ();
	//if (!nLayout) 
		{
		if (IsWindow (m_colorWnd))
			m_colorWnd.DestroyWindow ();
		if (IsWindow (m_paletteWnd))
			m_paletteWnd.DestroyWindow ();
		}
	}
}

//------------------------------------------------------------------------------

BOOL CTextureTool::OnInitDialog ()
{
	CRect	rc;

if (!CToolDlg::OnInitDialog ())
   return FALSE;
/*
m_btnZoomIn.SubclassDlgItem (IDC_TEXALIGN_ZOOMIN, this);
m_btnZoomOut.SubclassDlgItem (IDC_TEXALIGN_ZOOMOUT, this);
m_btnHShrink.SubclassDlgItem (IDC_TEXALIGN_HSHRINK, this);
m_btnVShrink.SubclassDlgItem (IDC_TEXALIGN_VSHRINK, this);
m_btnHALeft.SubclassDlgItem (IDC_TEXALIGN_HALEFT, this);
m_btnHARight.SubclassDlgItem (IDC_TEXALIGN_HARIGHT, this);
m_btnVAUp.SubclassDlgItem (IDC_TEXALIGN_VAUP, this);
m_btnVADown.SubclassDlgItem (IDC_TEXALIGN_VADOWN, this);
m_btnRALeft.SubclassDlgItem (IDC_TEXALIGN_RALEFT, this);
m_btnRARight.SubclassDlgItem (IDC_TEXALIGN_RARIGHT, this);
*/
m_btnZoomIn.AutoLoad (IDC_TEXALIGN_ZOOMIN, this);
m_btnZoomOut.AutoLoad (IDC_TEXALIGN_ZOOMOUT, this);
m_btnHShrink.AutoLoad (IDC_TEXALIGN_HSHRINK, this);
m_btnVShrink.AutoLoad (IDC_TEXALIGN_VSHRINK, this);
m_btnHALeft.AutoLoad (IDC_TEXALIGN_HALEFT, this);
m_btnHARight.AutoLoad (IDC_TEXALIGN_HARIGHT, this);
m_btnVAUp.AutoLoad (IDC_TEXALIGN_VAUP, this);
m_btnVADown.AutoLoad (IDC_TEXALIGN_VADOWN, this);
m_btnRALeft.AutoLoad (IDC_TEXALIGN_RALEFT, this);
m_btnRARight.AutoLoad (IDC_TEXALIGN_RARIGHT, this);

m_btnStretch2Fit.AutoLoad (IDC_TEXALIGN_STRETCH2FIT, this);
m_btnReset.AutoLoad (IDC_TEXALIGN_RESET, this);
m_btnResetTagged.AutoLoad (IDC_TEXALIGN_RESET_TAGGED, this);
m_btnChildAlign.AutoLoad (IDC_TEXALIGN_CHILDALIGN, this);
m_btnAlignAll.AutoLoad (IDC_TEXALIGN_ALIGNALL, this);
m_btnAddLight.AutoLoad (IDC_TEXLIGHT_ADD, this);
m_btnDelLight.AutoLoad (IDC_TEXLIGHT_DELETE, this);
m_btnHFlip.AutoLoad (IDC_TEXALIGN_HFLIP, this);
m_btnVFlip.AutoLoad (IDC_TEXALIGN_VFLIP, this);
#if 0
m_btnZoomIn.EnableToolTips (TRUE);
m_btnZoomOut.EnableToolTips (TRUE);
m_btnHShrink.EnableToolTips (TRUE);
m_btnVShrink.EnableToolTips (TRUE);
m_btnHALeft.EnableToolTips (TRUE);
m_btnHARight.EnableToolTips (TRUE);
m_btnVAUp.EnableToolTips (TRUE);
m_btnVADown.EnableToolTips (TRUE);
m_btnRALeft.EnableToolTips (TRUE);
m_btnRARight.EnableToolTips (TRUE);
m_btnStretch2Fit.EnableToolTips (TRUE);
m_btnReset.EnableToolTips (TRUE);
m_btnResetTagged.EnableToolTips (TRUE);
m_btnChildAlign.EnableToolTips (TRUE);
m_btnAlignAll.EnableToolTips (TRUE);
m_btnAddLight.EnableToolTips (TRUE);
m_btnDelLight.EnableToolTips (TRUE);
#endif
#if 1
m_brightnessCtrl.Init (this, (nLayout == 1) ? -IDC_TEXTURE_BRIGHTSLIDER : IDC_TEXTURE_BRIGHTSLIDER, IDC_TEXTURE_BRIGHTSPINNER, IDC_TEXTURE_BRIGHTNESS, 0, 100, 1.0, 1.0, 10);
m_lightTimerCtrl.Init (this, (nLayout == 1) ? -IDC_TEXLIGHT_TIMERSLIDER : IDC_TEXLIGHT_TIMERSLIDER, IDC_TEXLIGHT_TIMERSPINNER, IDC_TEXLIGHT_TIMER, 0, 1000, 50.0, 1.0, 2);
#else
InitSlider (IDC_TEXTURE_BRIGHTSLIDER, 0, 100);
BrightnessSlider ()->SetTicFreq (10);
InitSlider (IDC_TEXLIGHT_TIMERSLIDER, 0, 20);
BrightnessSpinner ()->SetRange (0, 100);
#endif
LoadTextureListBoxes ();
CreateImgWnd (&m_textureWnd, IDC_TEXTURE_SHOW);
CreateImgWnd (&m_alignWnd, IDC_TEXALIGN_SHOW);
HScrollAlign ()->SetScrollRange (-100, 100, FALSE);
HScrollAlign ()->SetScrollPos (0, TRUE);
VScrollAlign ()->SetScrollRange (-100, 100, FALSE);
VScrollAlign ()->SetScrollPos (0, TRUE);
CreateColorCtrl (&m_lightWnd, IDC_TEXLIGHT_SHOW);
//if (!nLayout) 
	{
	CreateColorCtrl (&m_colorWnd, IDC_TEXLIGHT_COLOR);
	m_paletteWnd.Create (GetDlgItem (IDC_TEXLIGHT_PALETTE), -1, -1);
	}
m_nTimer = -1; 
m_nLightTimer = -1;
m_nEditTimer = -1;
UpdateLightWnd ();
UpdateData (FALSE);
m_bInited = true;
return TRUE;
}

//------------------------------------------------------------------------------

void CTextureTool::DoDataExchange (CDataExchange *pDX)
{
if (!HaveData (pDX)) 
	return;


for (int i = 0; i < 4; i++)
	DDX_Double (pDX, IDC_TEXTURE_LIGHT1 + i, m_lights [i], 0, 200, "%1.1f");
DDX_Check (pDX, IDC_TEXTURE_PASTE1ST, m_bUse1st);
DDX_Check (pDX, IDC_TEXTURE_PASTE2ND, m_bUse2nd);
DDX_Check (pDX, IDC_TEXALIGN_SHOWTEXTURE, m_bShowTexture);
DDX_Check (pDX, IDC_TEXALIGN_SHOWCHILDREN, m_bShowChildren);
DDX_Check (pDX, IDC_TEXALIGN_IGNOREPLANE, m_bIgnorePlane);
//DDX_Check (pDX, IDC_TEXALIGN_IGNOREWALLS, m_bIgnoreWalls);
DDX_Double (pDX, IDC_TEXALIGN_HALIGN, m_alignX);
DDX_Double (pDX, IDC_TEXALIGN_VALIGN, m_alignY);
DDX_Double (pDX, IDC_TEXALIGN_RALIGN, m_alignAngle);
DDX_Radio (pDX, IDC_TEXALIGN_ROT0, m_alignRot2nd);
//DDX_Double (pDX, IDC_TEXLIGHT_TIMER, m_nLightTime);
DDX_Text (pDX, IDC_TEXTURE_PASTEBUF, m_szTextureBuf, sizeof (m_szTextureBuf));
//if (!nLayout) 
	{
	DDX_Text (pDX, IDC_TEXLIGHT_EDIT, m_szLight, sizeof (m_szLight));
	DDX_Text (pDX, IDC_TEXLIGHT_COLORINDEX, m_nColorIndex);
	}

#if 1
if (pDX->m_bSaveAndValidate)
	m_nBrightness = m_brightnessCtrl.GetValue ();
#else
int nBrightness;
char szBrightness [20];
sprintf_s (szBrightness, sizeof (szBrightness), "%d", m_nBrightness);
DDX_Text (pDX, IDC_TEXTURE_BRIGHTNESS, szBrightness, sizeof (szBrightness));
if (pDX->m_bSaveAndValidate && *szBrightness) {
	m_nBrightness = atoi (szBrightness);
	nBrightness = (m_nBrightness < 0) ? 0 : (m_nBrightness > 100) ? 100 : m_nBrightness;
	BrightnessSlider ()->SetPos (100 - nBrightness);
	BrightnessSpinner ()->SetPos (nBrightness);
	SetBrightness (nBrightness);
	}
#endif
}

//------------------------------------------------------------------------------

void CTextureTool::LoadTextureListBoxes (void) 
{
CHECKMINE;

	int			bShowFrames;
	int			nTextures, i, index;
	CComboBox	*cbTexture1 = CBTexture1 ();
	CComboBox	*cbTexture2 = CBTexture2 ();

bShowFrames = GetCheck (IDC_TEXTURE_SHOWFRAMES);

short texture1 = current->Side ()->BaseTex ();
short texture2 = current->Side ()->OvlTex (0);

if ((texture1 < 0) || (texture1 >= MAX_TEXTURES))
	texture1 = 0;
if ((texture2 < 0) || (texture2 >= MAX_TEXTURES))
	texture2 = 0;

cbTexture1->ResetContent ();
cbTexture2->ResetContent ();
index = cbTexture1->AddString ("(none)");
nTextures = textureManager.MaxTextures ();
for (i = 0; i < nTextures; i++) {
	if (textureManager.m_header [1].m_nVersion == 0)
		textureManager.m_header [1].m_nVersion = 1;
	char* p = textureManager.Name (-1, i);
	if (bShowFrames || !strstr (p, "frame")) {
		index = cbTexture1->AddString (p);
		cbTexture1->SetItemData (index, i);
		if (texture1 == i)
			cbTexture1->SetCurSel (index);
		index = cbTexture2->AddString (i ? p : "(none)");
		if (texture2 == i)
			cbTexture2->SetCurSel (index);
		cbTexture2->SetItemData (index, i);
	if (textureManager.m_header [1].m_nVersion == 0)
		textureManager.m_header [1].m_nVersion = 1;
		}
	}
}

//------------------------------------------------------------------------------

bool CTextureTool::SideHasLight (void)
{
if (theMine == null) return false;

if	((lightManager.IsLight (current->Side ()->BaseTex ()) != -1) ||
	 ((current->Side ()->OvlTex (0) != 0) &&
	  (lightManager.IsLight (current->Side ()->OvlTex (0)) != -1)))
	return true;
CWall *wallP = current->Wall ();
return (wallP != null) && (wallP->Type () == WALL_COLORED);

}


//------------------------------------------------------------------------------

void CTextureTool::UpdatePaletteWnd (void)
{
if (m_paletteWnd.m_hWnd) {
	if (/*!nLayout && (DLE.IsD2XLevel ()) &&*/ SideHasLight ()) {
		EnableControls (IDC_TEXLIGHT_PALETTE + 1, IDC_TEXLIGHT_COLOR, TRUE);
		m_paletteWnd.ShowWindow (SW_SHOW);
		m_paletteWnd.DrawPalette ();
		UpdateColorCtrl (
			&m_colorWnd, 
			(m_nColorIndex > 0) ? 
			RGB (m_rgbColor.peRed, m_rgbColor.peGreen, m_rgbColor.peBlue) :
			RGB (0,0,0));
		}
	else {
		EnableControls (IDC_TEXLIGHT_PALETTE + 1, IDC_TEXLIGHT_COLOR, FALSE);
		m_paletteWnd.ShowWindow (SW_HIDE);
		}
	}
}

//------------------------------------------------------------------------------

void CTextureTool::OnPaint ()
{
if (theMine == null) 
	return;
CTexToolDlg::OnPaint ();
#if TEXTOOLDLG == 0
UpdateTextureWnd ();
#endif
UpdateAlignWnd ();
UpdatePaletteWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::UpdateTextureWnd (void)
{
#if TEXTOOLDLG == 0
RefreshTextureWnd ();
m_textureWnd.InvalidateRect (null);
m_textureWnd.UpdateWindow ();
#endif
}

//------------------------------------------------------------------------------

int lightIdxFromMode [4] = {0, 3, 2, 1};

void CTextureTool::Refresh (void)
{
if (!(m_bInited && theMine))
	return;
if (m_bInitTextureListBoxes) {
	m_bInitTextureListBoxes = false;
	LoadTextureListBoxes ();
	}

//Beep (1000,100);
	CComboBox	*cbTexture1 = CBTexture1 ();
	CComboBox	*cbTexture2 = CBTexture2 ();
	bool			bShowTexture;
	int			i, j;
// enable buttons as required
/*
EditButton->EnableWindow((IsD1File () || path [0] == null) ? FALSE: TRUE);
LightButton->EnableWindow((m_fileType==RDL_FILE) ? FALSE:TRUE);
PickButton->EnableWindow(path [0] ? TRUE:FALSE);
*/
// set animation frames to zero
#if TEXTOOLDLG == 0
m_frame [0] = 0;
m_frame [1] = 0;
#endif

CSegment* segP = current->Segment ();
CSide* sideP = current->Side ();
CWall* wallP = current->Wall ();
CColor* colorP = current->LightColor ();
int nSide = current->m_nSide;
short texture1 = sideP->BaseTex ();
short texture2 = sideP->OvlTex (0);
m_nColorIndex = ((wallP != null) && (wallP->Type () == WALL_COLORED)) ? wallP->Info ().cloakValue : colorP->m_info.index;
m_rgbColor.peRed = (char) (255.0 * colorP->m_info.color.r);
m_rgbColor.peGreen = (char) (255.0 * colorP->m_info.color.g);
m_rgbColor.peBlue = (char) (255.0 * colorP->m_info.color.b);
if ((texture1 < 0) || (texture1 >= MAX_TEXTURES))
	texture1 = 0;
if ((texture2 < 0) || (texture2 >= MAX_TEXTURES))
	texture2 = 0;
GetBrightness ((m_bUse2nd && !m_bUse1st) ? texture2 : texture1);
short ovlAlign = sideP->OvlAlignment ();
// set edit fields to % of light and enable them
for (i = 0; i < 4; i++) {
	j = (i + lightIdxFromMode [ovlAlign]) % 4;
	double h = 327.68 * lightManager.LightScale ();
	m_lights [j] = (sideP->m_info.uvls [i].l >= h) ? h / 327.68 : (double) (sideP->m_info.uvls [i].l) / 327.68;
	}

if (segP->ChildId (nSide)==-1)
	bShowTexture = TRUE;
else {
	ushort nWall = sideP->m_info.nWall;
	bShowTexture = (nWall >= 0) && (nWall < wallManager.WallCount ());
	}
if (bShowTexture) {
	if ((texture1 != m_lastTexture [0]) || (texture2 != m_lastTexture [1]) || (ovlAlign != m_lastMode)) {
		m_lastTexture [0] = texture1;
		m_lastTexture [1] = texture2;
		m_lastMode = ovlAlign;
		strcpy_s (message, sizeof (message), textureManager.Name (-1, texture1));
		cbTexture1->SetCurSel (i = cbTexture1->SelectString (-1, message));  // unselect if string not found
		if (texture2) {
			strcpy_s (message, sizeof (message), textureManager.Name (-1, texture2));
			cbTexture2->SetCurSel (cbTexture2->SelectString (-1, message));  // unselect if string not found
			}
		else
			cbTexture2->SelectString (-1, "(none)");
		}
	}
else {
	m_lastTexture [0] = -1;
	m_lastTexture [1] = -1;
	cbTexture1->SelectString (-1, "(none)");
	cbTexture2->SelectString (-1, "(none)");
	}
RefreshAlignment ();
UpdateData (FALSE);
#if TEXTOOLDLG
CTexToolDlg::Refresh ();
#else
UpdateTextureWnd ();
#endif
UpdateAlignWnd ();
UpdateLightWnd ();
UpdatePaletteWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::RefreshTextureWnd ()
{
CHECKMINE;

#if TEXTOOLDLG
if (!CTexToolDlg::Refresh ())
#else
if (!PaintTexture (&m_textureWnd))
#endif
	m_lastTexture [0] =
	m_lastTexture [1] = -1;
}

//------------------------------------------------------------------------------
#if TEXTOOLDLG == 0
void CTextureTool::DrawTexture (short texture1, short texture2, int x0, int y0) 
{
	ubyte bitmap_array [64*64];
	int x_offset;
	int y_offset;
	x_offset = 0;
	y_offset = 0;
	CDC	*pDC = m_textureWnd.GetDC ();
	CTexture tx (bmBuf);

memset (bitmap_array,0,sizeof (bitmap_array));
if (textureManager.BlendTextures(texture1,texture2,&tx,x0,y0))
	return;
BITMAPINFO *bmi;
bmi = MakeBitmap ();
CPalette	*oldPalette = pDC->SelectPalette (m_currentPalette, FALSE);
pDC->RealizePalette ();
//SetDIBitsToDevice (pDC->m_hDC, x_offset, y_offset, 64, 64, 0, 0, 0, 64, bitmap_array, bmi, DIB_RGB_COLORS);
CRect	rc;
m_textureWnd.GetClientRect (&rc);
StretchDIBits (pDC->m_hDC, x_offset, y_offset, rc.Width (), rc.Height (), 0, 0, tx.width, tx.height,
		        	(void *) bitmap_array, bmi, DIB_RGB_COLORS, SRCCOPY);
pDC->SelectPalette(oldPalette, FALSE);
m_textureWnd.ReleaseDC (pDC);
}
#endif
//------------------------------------------------------------------------------

BOOL CTextureTool::OnSetActive ()
{
#if TEXTOOLDLG
CTexToolDlg::OnSetActive ();
#else
m_nTimer = SetTimer (1, 100U, null);
#endif
if (m_iLight >= 0)
	m_nLightTimer = SetTimer (2, m_nLightDelay, null);
Refresh ();
return TRUE; //CTexToolDlg::OnSetActive ();
}

//------------------------------------------------------------------------------

BOOL CTextureTool::OnKillActive ()
{
#if TEXTOOLDLG
CTexToolDlg::OnKillActive ();
#else
if (m_nTimer >= 0) {
	KillTimer (m_nTimer);
	m_nTimer = -1;
	}
#endif
if (m_nLightTimer >= 0) {
	KillTimer (m_nLightTimer);
	m_nLightTimer = -1;
	}
if (m_nEditTimer >= 0) {
	KillTimer (m_nEditTimer);
	m_nEditTimer = -1;
	}
return CToolDlg::OnKillActive ();
}

//------------------------------------------------------------------------------

#if TEXTOOLDLG == 0

void CTextureTool::AnimateTexture (void)
{
	CSegment *segP = current->Segment ();

	ushort texture [2];
	static int scroll_offset_x = 0;
	static int scroll_offset_y = 0;
	int bScroll;
	int x,y;
	static int old_x,old_y;

	CSide	*sideP = current->Side ();

texture [0] = sideP->BaseTex ();
texture [1] = sideP->OvlTex (1);

// if texture1 is a scrolling texture, then offset the textures and
// redraw them, then return
bScroll = textureManager.ScrollSpeed (texture [0], &x, &y);
if (bScroll) {
	DrawTexture (texture [0], texture [1], scroll_offset_x, scroll_offset_y);
	if (old_x != x || old_y != y) {
		scroll_offset_x = 0;
		scroll_offset_y = 0;
		}
	old_x = x;
	old_y = y;
	scroll_offset_x += x;
	scroll_offset_y += y;
	scroll_offset_x &= 63;
	scroll_offset_y &= 63;
	return;
	}

scroll_offset_x = 0;
scroll_offset_y = 0;

// abort if this is not a wall
#ifndef _DEBUG
ushort nWall = sideP->m_info.nWall;
if (nWall >= wallManager.WallCount ())
	return;

// abort if this wall is not a door
if (wallManager.Wall () [nWall].type != WALL_DOOR)
	return;
#endif
	int i;
	static int hold_time [2] = {0,0};
	static int inc [2]= {1,1}; // 1=forward or -1=backwards
	int index [2];
	static ushort d1_anims [] = {
		371, 376, 387, 399, 413, 419, 424, 436, 444, 459,
		472, 486, 492, 500, 508, 515, 521, 529, 536, 543,
		550, 563, 570, 577, 584, 0
		};
// note: 584 is not an anim texture, but it is used to calculate
//       the number of frames in 577
// The 0 is used to end the search

	static ushort d2_anims [] = {
		435, 440, 451, 463, 477, 483, 488, 500, 508, 523,
		536, 550, 556, 564, 572, 579, 585, 593, 600, 608,
		615, 628, 635, 642, 649, 664, 672, 687, 702, 717,
		725, 731, 738, 745, 754, 763, 772, 780, 790, 806,
		817 ,827 ,838 ,849 ,858 ,863 ,871 ,886, 901 ,910,
		0
		};
// note: 910 is not an anim texture, but it is used to calculate
//       the number of frames in 901
// The 0 is used to end the search
	ushort *anim; // points to d1_anim or d2_anim depending on m_fileType

// first find out if one of the textures is animated
anim = (IsD1File ()) ? d1_anims : d2_anims;

for (i=0; i<2;i++)
	for (index [i] = 0; anim [index [i]]; index [i]++)
		if (texture [i] == anim [index [i]])
			break;

if (anim [index [0]] || anim [index [1]]) {
	// calculate new texture numbers
	for (i = 0; i < 2; i++) {
		if (anim [index [i]]) {
		// if hold time has not expired, then return
			if (hold_time [i] < 5)
				hold_time [i]++;
			else
				m_frame [i] += inc [i];
			if (m_frame [i] < 0) {
				m_frame [i] = 0;
				hold_time [i] = 0;
				inc [i] = 1;
				}
			if (anim [index [i]] + m_frame [i] >= anim [index [i] + 1]) {
				m_frame [i] = (anim [index [i] + 1] - anim [index [i]]) - 1;
				hold_time [i] = 0;
				inc [i] = -1;
				}
			texture [i] = anim [index [i]] + m_frame [i];
			}
		}
	DrawTexture (texture [0], texture [1], 0, 0);
	}
}
#endif
//------------------------------------------------------------------------------

void CTextureTool::OnTimer (UINT_PTR nIdEvent)
{
#if TEXTOOLDLG
if (nIdEvent == 2)
	AnimateLight ();
else if (nIdEvent == 3)
	OnEditTimer ();
else 
	CTexToolDlg::OnTimer (nIdEvent);
#else
if (nIdEvent == 1)
	AnimateTexture ();
else if (nIdEvent == 2)
	AnimateLight ();
else if (nIdEvent == 3)
	OnEditTimer ();
else 
	CToolDlg::OnTimer (nIdEvent);
#endif
}

//------------------------------------------------------------------------------

void CTextureTool::SelectTexture (int nIdC, bool bFirst)
{
CHECKMINE;

	CSide*		sideP = current->Side ();
	CComboBox*	pcb = bFirst ? CBTexture1 () : CBTexture2 ();
	int			index = pcb->GetCurSel ();
	
if (index <= 0)
	sideP->m_info.nOvlTex = 0;
else {
	short texture = (short) pcb->GetItemData (index);
	if (bFirst)
		segmentManager.SetTextures (*current, texture, -1);
	else
		segmentManager.SetTextures (*current, -1, texture);
	}
Refresh ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnSetLight () 
{
CHECKMINE;

UpdateData (TRUE);
CSide *sideP = current->Side ();
short ovlAlign = sideP->OvlAlignment ();
int i, j;
for (i = 0; i < 4; i++) {
	j = (i + lightIdxFromMode [ovlAlign]) % 4;
	sideP->m_info.uvls [i].l = (ushort) (m_lights [j] * 327.68);
	}
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnSelect1st () 
{
SelectTexture (IDC_TEXTURE_1ST, true);
}

//------------------------------------------------------------------------------

void CTextureTool::OnSelect2nd () 
{
SelectTexture (IDC_TEXTURE_2ND, false);
}

//------------------------------------------------------------------------------

void CTextureTool::OnEditTexture () 
{
int i = (int) CBTexture1 ()->GetItemData (CBTexture1 ()->GetCurSel ());
int j = m_bUse1st ? 0 : (int) CBTexture2 ()->GetItemData (CBTexture1 ()->GetCurSel ());

CTextureEdit e (j != 0, textureManager.Name (-1, (short) (j ? j : i)));

i = int (e.DoModal ());
DLE.MineView ()->Refresh (false);
Refresh ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnSaveTexture () 
{
CHECKMINE;

	char			*t1Name, *t2Name;
	CSide		*sideP = current->Side ();
	CComboBox	*pcb;

m_saveTexture [0] = sideP->BaseTex ();
m_saveTexture [1] = sideP->OvlTex (0);
int i;
for (i = 0; i < 4; i++)
	m_saveUVLs [i].l = sideP->m_info.uvls [i].l;

pcb = CBTexture1 ();
t1Name = textureManager.Name (-1, (short) pcb->GetItemData (pcb->GetCurSel ()));
pcb = CBTexture2 ();
if (i = int (pcb->GetItemData (pcb->GetCurSel ())))
	t2Name = textureManager.Name (-1, (short) i);
else
	t2Name = "(none)";
sprintf_s (m_szTextureBuf, sizeof (m_szTextureBuf ), "%s,%s", t1Name, t2Name);
UpdateData (FALSE);
//SaveTextureStatic->SetText(message);
}

//------------------------------------------------------------------------------

void CTextureTool::OnPaste1st () 
{
m_bUse1st = !m_bUse1st;
Refresh ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnPaste2nd () 
{
m_bUse2nd = !m_bUse2nd;
Refresh ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnCleanup () 
{
textureManager.RemoveTextures ();
Refresh ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnPasteSide () 
{
CHECKMINE;

UpdateData (TRUE);
if (!(m_bUse1st || m_bUse2nd))
	return;

	CSide *sideP = current->Side ();

//CheckForDoor ();
undoManager.Begin (udSegments);
segmentManager.SetTextures (*current, m_bUse1st ? m_saveTexture [0] : -1, m_bUse2nd ? m_saveTexture [1] : -1);
for (int i = 0; i < 4; i++)
	sideP->m_info.uvls [i].l = m_saveUVLs [i].l;
undoManager.End ();
Refresh ();
DLE.MineView ()->Refresh ();
}


//------------------------------------------------------------------------------

void CTextureTool::OnPasteTouching ()
{
CHECKMINE;

UpdateData (TRUE);
if (!(m_bUse1st || m_bUse2nd))
	return;
if (m_saveTexture [0] == -1 || m_saveTexture [1] == -1)
	return;
//CheckForDoor ();
// set all segment sides as not "pasted" yet
undoManager.Begin (udSegments);
CSegment *segP = segmentManager.Segment (0);
for (short nSegment = segmentManager.Count (); nSegment; nSegment--, segP++)
	segP->Index () = 0;
PasteTexture (current->m_nSegment, current->m_nSide, 1000);
undoManager.End ();
Refresh ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------
// TODO: Beginning with the current side, walk through all adjacent sides and 
// mark all in plane

class CPlaneSide {
	public:
		short nParent;
		short	nSegment;
};

void CTextureTool::OnTagPlane () 
{
CHECKMINE;

UpdateData (TRUE);
vertexManager.UnTagAll ();

CDynamicArray<CSideKey> sideList;
sideList.Create (segmentManager.VisibleSideCount ());

int nHead = 0;
int nTail = 1;
sideList [nHead] = CSideKey (*current);

CAVLTree <CEdgeTreeNode, uint> edgeTree;
segmentManager.GatherEdges (edgeTree);

CEdgeList edgeList;

while (nHead < nTail) {
	CSideKey key = sideList [nHead++];
	CSegment* segP = segmentManager.Segment (key);
	CSide* sideP = segmentManager.Side (key);
	segP->ComputeNormals (key.m_nSide);
	edgeList.Reset ();
	int nEdges = segP->BuildEdgeList (edgeList, ubyte (key.m_nSide), true);
	for (int nEdge = 0; nEdge < nEdges; nEdge++) {
		ubyte side1, side2, i1, i2;
		edgeList.Get (nEdge, side1, side2, i1, i2);
		if (side1 > 5) {
			side1 = side2;
			if (side1 > 5)
				continue;
			}
		ushort v1 = segP->VertexId (side1, i1);
		ushort v2 = segP->VertexId (side1, i2);
		CEdgeTreeNode* node = edgeTree.Find ((v1 < v2) ? v1 + (uint (v2) << 16) : v2 + (uint (v1) << 16));
		if (!node)
			continue;
		CSLLIterator<CSideKey, CSideKey>* iter = new CSLLIterator<CSideKey, CSideKey> (node->m_sides);
		for (CSideKey* keyP = iter->Begin (); keyP != iter->End (); (*iter)++) {
			CSegment* childSegP = segmentManager.Segment (*keyP);
			if (!childSegP->IsTagged (keyP->m_nSide)) {
				CSide* childSideP = segmentManager.Side (*keyP);
				childSegP->ComputeNormals (keyP->m_nSegment);
				if (fabs (Dot (sideP->Normal (), childSideP->Normal ())) < 0.3)
					childSegP->Tag (keyP->m_nSide);
				sideList [nTail++] = *keyP;
				}
			}
		}
	}

Refresh ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnReplace () 
{
CHECKMINE;

UpdateData (TRUE);
if (!(m_bUse1st || m_bUse2nd))
	return;

	short			nSegment,
					nSide;
	CSegment*	segP = segmentManager.Segment (0);
	CSide*		sideP;
	bool			bAll = !segmentManager.HaveTaggedSides ();

if (bAll && (QueryMsg ("Replace textures in entire mine?") != IDYES))
	return;
undoManager.Begin (udSegments);
if (bAll)
	INFOMSG (" Replacing textures in entire mine.");
for (nSegment = 0; nSegment < segmentManager.Count (); nSegment++, segP++)
	for (nSide = 0, sideP = segP->m_sides; nSide < 6; nSide++, sideP++)
		if (bAll || segmentManager.IsTagged (CSideKey (nSegment, nSide))) {
			if (m_bUse1st && (sideP->BaseTex () != m_lastTexture [0]))
				continue;
			if (m_bUse2nd && ((sideP->OvlTex (0)) != m_lastTexture [1]))
				continue;
			if ((segP->ChildId (nSide) >= 0) && (sideP->m_info.nWall == NO_WALL))
				 continue;
			segmentManager.SetTextures (CSideKey (nSegment, nSide), m_bUse1st ? m_saveTexture [0] : -1, m_bUse2nd ? m_saveTexture [1] : -1);
			}
undoManager.End ();
Refresh ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CTextureTool::PasteTexture (short nSegment, short nSide, short nDepth) 
{
CHECKMINE;

if (nDepth <= 0) 
	return;

	CSegment*	segP = segmentManager.Segment (nSegment);
	CSide*		sideP = segP->m_sides + nSide;
	short			oldTexture1, 
					oldTexture2;
	int			i;

// remember these texture for a comparison below
oldTexture1 = sideP->BaseTex ();
oldTexture2 = sideP->OvlTex ();
if ((oldTexture1 < 0) || (oldTexture1 >= MAX_TEXTURES))
	oldTexture1 = 0;
if ((oldTexture2 < 0) || (oldTexture2 >= MAX_TEXTURES))
	oldTexture2 = 0;
// mark segment as "pasted"
segP->Index () = 1;
// paste texture
segmentManager.SetTextures (CSideKey (nSegment, nSide), m_bUse1st ? m_saveTexture [0] : -1, m_bUse2nd ? m_saveTexture [1] : -1);
for (i = 0; i < 4; i++)
	sideP->m_info.uvls [i].l = m_saveUVLs [i].l;

// now check each adjing side to see it has the same texture
for (i = 0; i < sideP->VertexCount (); i++) {
	short nAdjSeg, nAdjSide;
	if (GetAdjacentSide (nSegment, nSide, i, &nAdjSeg, &nAdjSide)) {
		// if adj matches and its not "pasted" yet
		segP = segmentManager.Segment (nAdjSeg);
		sideP = segP->m_sides + nAdjSide;
		if ((segP->Index () == 0) &&
			 (!m_bUse1st || (sideP->BaseTex () == oldTexture1)) &&
			 (!m_bUse2nd || (sideP->OvlTex () == oldTexture2))) {
			PasteTexture (nAdjSeg, nAdjSide, --nDepth);
			}
		}
	}
}

//------------------------------------------------------------------------------

bool CTextureTool::GetAdjacentSide (short nStartSeg, short nStartSide, short nLine, short *nAdjSeg, short *nAdjSide) 
{
	CSegment *	segP;
	short			nSide, nChild;
	ushort		nEdgeVerts [2], nChildEdgeVerts [2];
	short			nChildSide, nChildLine;

// figure out which side of child shares two points w/ nStartSide
// find vert numbers for the line's two end points
segP = segmentManager.Segment (nStartSeg);
nEdgeVerts [0] = segP->VertexId (nStartSide, nLine);
nEdgeVerts [1] = segP->VertexId (nStartSide, nLine + 1);

nSide = segP->AdjacentSide (nStartSide, nEdgeVerts);
nChild = segP->ChildId (nSide);
if (nChild < 0 || nChild >= segmentManager.Count ())
	return false;
for (nChildSide = 0; nChildSide < 6; nChildSide++) {
	CSegment* childSegP = segmentManager.Segment (nChild);
	if ((childSegP->ChildId (nChildSide) == -1) || (childSegP->m_sides [nChildSide].Wall () != null)) {
		CSide* childSideP = childSegP->Side (nChildSide);
		for (nChildLine = 0; nChildLine < childSideP->VertexCount (); nChildLine++) {
			// find vert numbers for the line's two end points
			nChildEdgeVerts [0] = childSegP->VertexId (nChildSide, nChildLine);
			nChildEdgeVerts [1] = childSegP->VertexId (nChildSide, nChildLine + 1);
			// if points of child's line == corresponding points of parent
			if (((nChildEdgeVerts [0] == nEdgeVerts [1]) && (nChildEdgeVerts [1] == nEdgeVerts [0])) ||
				 ((nChildEdgeVerts [0] == nEdgeVerts [0]) && (nChildEdgeVerts [1] == nEdgeVerts [1]))) {
				// now we know the child's side & line which touches the parent
				// child:  nChild, nChildSide, nChildLine, nChildPoint0, nChildPoint1
				// parent: nStartSeg, nStartSide, nLine, point0, point1
				*nAdjSeg = nChild;
				*nAdjSide = nChildSide;
				return true;
				}
			}
		}
	}
return false;
}
//------------------------------------------------------------------------------

void CTextureTool::GetBrightness (int nTexture)
{
	int nBrightness;

if (nTexture < 0)
	nBrightness = 0;
else {
	nBrightness = int (*lightManager.LightMap (nTexture) /*/ TEXLIGHT_SCALE*/);
	if (nBrightness == MAX_TEXLIGHT)
		nBrightness = 100;
	else
		nBrightness = (100 * nBrightness + MAX_TEXLIGHT / 2) / MAX_TEXLIGHT;
#if 1
	m_brightnessCtrl.SetValue (nBrightness);
#else
	BrightnessSlider ()->SetPos (100 - nBrightness);
	BrightnessSpinner ()->SetPos (nBrightness);
#endif
	}
m_nBrightness = nBrightness;
}

//------------------------------------------------------------------------------

void CTextureTool::SetBrightness (int nBrightness)
{
	static	BOOL	bSemaphore = FALSE;

if (!bSemaphore) {
	bSemaphore = TRUE;

	CComboBox	*pcb = (m_bUse2nd && !m_bUse1st) ? CBTexture2 () : CBTexture1 ();
	int			index = pcb->GetCurSel ();
	short			texture = (short) pcb->GetItemData (index);

	if (texture >= 0) {
		m_nBrightness = nBrightness;
		nBrightness = int (nBrightness /** TEXLIGHT_SCALE*/);
		*lightManager.LightMap (texture) = ((nBrightness == 100) ? MAX_TEXLIGHT : nBrightness * (MAX_TEXLIGHT / 100));
		}
	Refresh ();
	bSemaphore = FALSE;
	}
}

//------------------------------------------------------------------------------

void CTextureTool::OnBrightnessEdit (void)
{
if (m_brightnessCtrl.OnEdit ())
	UpdateData (TRUE);
}

//------------------------------------------------------------------------------

void CTextureTool::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
#if 1
if (m_brightnessCtrl.OnScroll (scrollCode, thumbPos, pScrollBar)) {
	SetBrightness (m_brightnessCtrl.GetValue ());
	}
else if (m_lightTimerCtrl.OnScroll (scrollCode, thumbPos, pScrollBar)) {	
	SetLightDelay (m_lightTimerCtrl.GetValue ());
	}
#else
	int	nPos;

if (pScrollBar == (CScrollBar *) BrightnessSlider ()) {
	nPos = 100 - pScrollBar->GetScrollPos ();
	switch (scrollCode) {
		case SB_LINEUP:
			nPos--;
			break;
		case SB_LINEDOWN:
			nPos++;
			break;
		case SB_PAGEUP:
			nPos -= 10;
			break;
		case SB_PAGEDOWN:
			nPos += 10;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	if (nPos < 0)
		nPos = 0;
	else if (nPos > 100)
		nPos = 100;
	SetBrightness (nPos);
	UpdatePaletteWnd ();
	UpdateData (FALSE);
	pScrollBar->SetScrollPos (100 - nPos, TRUE);
	}
#endif
else {
	int nPos = pScrollBar->GetScrollPos ();
	CRect rc;
	m_alignWnd.GetClientRect (rc);
	switch (scrollCode) {
		case SB_LINEUP:
			nPos--;
			break;
		case SB_LINEDOWN:
			nPos++;
			break;
		case SB_PAGEUP:
			nPos -= rc.Width () / 4;
			break;
		case SB_PAGEDOWN:
			nPos += rc.Width () / 4;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	pScrollBar->SetScrollPos (nPos, TRUE);
	UpdateAlignWnd ();
	}
}

//------------------------------------------------------------------------------

void CTextureTool::OnVScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
#if 1
if (m_brightnessCtrl.OnScroll (scrollCode, thumbPos, pScrollBar)) {
	SetBrightness (m_brightnessCtrl.GetValue ());
	}
else if (m_lightTimerCtrl.OnScroll (scrollCode, thumbPos, pScrollBar)) {	
	SetLightDelay (m_lightTimerCtrl.GetValue ());
	}
#else
	int	nPos;
	CRect	rc;

if (pScrollBar == (CScrollBar *) TimerSlider ()) {
	nPos = pScrollBar->GetScrollPos ();
	if (m_iLight < 0)
		return;
	switch (scrollCode) {
		case SB_LINEUP:
			nPos++;
			break;
		case SB_LINEDOWN:
			nPos--;
			break;
		case SB_PAGEUP:
			nPos += 5;
			break;
		case SB_PAGEDOWN:
			nPos -= 5;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = 20 - thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	if (nPos < 0)
		nPos = 0;
	else if (nPos > 20)
		nPos = 20;
	SetLightDelay (nPos * 50);
//	pScrollBar->SetScrollPos (nPos, TRUE);
	}
else if (pScrollBar == (CScrollBar *) BrightnessSlider ()) {
	nPos = 100 - pScrollBar->GetScrollPos ();
	switch (scrollCode) {
		case SB_LINEUP:
			nPos--;
			break;
		case SB_LINEDOWN:
			nPos++;
			break;
		case SB_PAGEUP:
			nPos -= 10;
			break;
		case SB_PAGEDOWN:
			nPos += 10;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	if (nPos < 0)
		nPos = 0;
	else if (nPos > 100)
		nPos = 100;
	SetBrightness (nPos);
	BrightnessSpinner ()->SetPos (100 - nPos);
	UpdateData (FALSE);
	pScrollBar->SetScrollPos (100 - nPos, TRUE);
	}
else if (pScrollBar == (CScrollBar *) BrightnessSpinner ()) {
	if (scrollCode != SB_THUMBPOSITION)
		return;
	nPos = thumbPos;
	SetBrightness (nPos);
	BrightnessSlider ()->SetPos (100 - nPos);
	UpdateData (FALSE);
	}
#endif
else {
	int nPos = pScrollBar->GetScrollPos ();
	CRect rc;
	m_alignWnd.GetClientRect (rc);
	switch (scrollCode) {
		case SB_LINEUP:
			nPos--;
			break;
		case SB_LINEDOWN:
			nPos++;
			break;
		case SB_PAGEUP:
			nPos -= rc.Height () / 4;
			break;
		case SB_PAGEDOWN:
			nPos += rc.Height () / 4;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	pScrollBar->SetScrollPos (nPos, TRUE);
	UpdateAlignWnd ();
	}
}

//------------------------------------------------------------------------------

BOOL CTextureTool::OnNotify (WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	LPNMHDR	nmHdr = (LPNMHDR) lParam;
	int		nMsg = nmHdr->code;

switch (wParam) {
	case IDC_TEXALIGN_HALIGN:
	case IDC_TEXALIGN_VALIGN:
	case IDC_TEXALIGN_RALIGN:
	case IDC_TEXALIGN_ZOOMIN:
	case IDC_TEXALIGN_ZOOMOUT:
	case IDC_TEXALIGN_HALEFT:
	case IDC_TEXALIGN_HARIGHT:
	case IDC_TEXALIGN_VAUP:
	case IDC_TEXALIGN_VADOWN:
	case IDC_TEXALIGN_RALEFT:
	case IDC_TEXALIGN_RARIGHT:
	case IDC_TEXALIGN_HSHRINK:
	case IDC_TEXALIGN_VSHRINK:
		if (((LPNMHDR) lParam)->code == WM_LBUTTONDOWN) {
			m_nEditFunc = int (wParam);
			m_nEditTimer = SetTimer (3, m_nTimerDelay = 250U, null);
			}
		else {
			m_nEditFunc = -1;
			if (m_nEditTimer >= 0) {
				KillTimer (m_nEditTimer);
				m_nEditTimer = -1;
				}
			}
		break;
	case IDC_TEXLIGHT_COLOR:
		return 0;
	default:
/*		if (((LPNMHDR) lParam)->code == WM_LBUTTONDOWN)
			OnLButtonDown ();
		else 
*/		return CTexToolDlg::OnNotify (wParam, lParam, pResult);
	}
*pResult = 0;
return TRUE;
}
		
//------------------------------------------------------------------------------

void CTextureTool::OnEditTimer (void)
{		
switch (m_nEditFunc) {
	case IDC_TEXALIGN_HALIGN:
		OnAlignX ();
		break;
	case IDC_TEXALIGN_VALIGN:
		OnAlignY ();
		break;
	case IDC_TEXALIGN_ZOOMIN:
		OnZoomIn ();
		break;
	case IDC_TEXALIGN_ZOOMOUT:
		OnZoomOut ();
		break;
	case IDC_TEXALIGN_HALEFT:
		OnAlignLeft ();
		break;
	case IDC_TEXALIGN_HARIGHT:
		OnAlignRight ();
		break;
	case IDC_TEXALIGN_VAUP:
		OnAlignUp ();
		break;
	case IDC_TEXALIGN_VADOWN:
		OnAlignDown ();
		break;
	case IDC_TEXALIGN_RALEFT:
		OnAlignRotLeft ();
		break;
	case IDC_TEXALIGN_RARIGHT:
		OnAlignRotRight ();
		break;
	case IDC_TEXALIGN_HSHRINK:
		OnHShrink ();
		break;
	case IDC_TEXALIGN_VSHRINK:
		OnVShrink ();
		break;
	default:
		break;
	}
UINT i = (m_nTimerDelay * 9) / 10;
if (i >= 25) {
	KillTimer (m_nTimer);
	m_nTimer = SetTimer (3, m_nTimerDelay = i, null);
	}
}

//------------------------------------------------------------------------------
		
		//eof texturedlg.cpp
