// dlcView.cpp : implementation of the CMineView class
//

#include "stdafx.h"
#include "afxpriv.h"
#include "dle-xp.h"

#include "dlcDoc.h"
#include "mineview.h"
#include "toolview.h"

#include "PaletteManager.h"
#include "textures.h"
#include "global.h"
#include "FileManager.h"
#include "Selection.h"
#include "TextureManager.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CTexToolDlg, CToolDlg)

BEGIN_MESSAGE_MAP(CTexToolDlg, CToolDlg)
	ON_WM_PAINT ()
	ON_WM_TIMER ()
END_MESSAGE_MAP()

//------------------------------------------------------------------------------

CTexToolDlg::CTexToolDlg (UINT nIdTemplate, CPropertySheet *pParent, 
								  int nTexWndId, int nTimerId, COLORREF bkColor,
								  bool bOtherSegment)
	: CToolDlg (nIdTemplate, pParent)
{
m_nTexWndId = nTexWndId;
m_nTimerId = nTimerId;
m_bkColor = bkColor;
m_nTimer = -1;
m_bOtherSegment = bOtherSegment;
}

//------------------------------------------------------------------------------

CTexToolDlg::~CTexToolDlg ()
{
if (m_bInited) {
	if (IsWindow (m_textureWnd))
		m_textureWnd.DestroyWindow ();
	}
}

//------------------------------------------------------------------------------

BOOL CTexToolDlg::OnInitDialog ()
{
if (!CToolDlg::OnInitDialog ())
	return FALSE;
CreateImgWnd (&m_textureWnd, m_nTexWndId);
m_bInited = true;
return TRUE;
}

//------------------------------------------------------------------------------

BOOL CTexToolDlg::TextureIsVisible ()
{
return TRUE;
}

//------------------------------------------------------------------------------

bool CTexToolDlg::Refresh (short nBaseTex, short nOvlTex, short nVisible)
{
m_frame [0] = 0;
m_frame [1] = 0;
if (nVisible < 0)
	nVisible = (short) TextureIsVisible ();
if (nVisible > 0) {
	if (nBaseTex < 0) {
		short nSegment = m_bOtherSegment ? other->m_nSegment : current->SegmentId ();
		short nSide = m_bOtherSegment ? other->m_nSide : current->SideId ();
		if (nVisible = segmentManager.IsWall (CSideKey (nSegment, nSide))) {
			CSide *sideP = m_bOtherSegment ? other->Side () : current->Side ();
			nBaseTex = sideP->BaseTex ();
			nOvlTex = sideP->OvlTex (0);
			}
		}
	}
if (nVisible > 0)
	return PaintTexture (&m_textureWnd, m_bkColor, nBaseTex, nOvlTex);
return PaintTexture (&m_textureWnd, m_bkColor, 0);
}

//------------------------------------------------------------------------------

void CTexToolDlg::OnPaint ()
{
CToolDlg::OnPaint ();
if (m_textureWnd.m_hWnd) {
	if (TextureIsVisible ()) {
		CSide *sideP = m_bOtherSegment ? other->Side () : current->Side ();
		PaintTexture (&m_textureWnd, m_bkColor, sideP->BaseTex (), sideP->OvlTex (0));
		}
	else
		PaintTexture (&m_textureWnd, m_bkColor, -1);
	m_textureWnd.InvalidateRect (null);
	m_textureWnd.UpdateWindow ();
	}
}

//------------------------------------------------------------------------------

BOOL CTexToolDlg::OnSetActive ()
{
if (m_nTimerId >= 0)
	m_nTimer = SetTimer ((UINT) m_nTimerId, 100U, null);
return CToolDlg::OnSetActive ();
}

//------------------------------------------------------------------------------

BOOL CTexToolDlg::OnKillActive ()
{
if (m_nTimer >= 0) {
	KillTimer (m_nTimer);
	m_nTimer = -1;
	}
return CToolDlg::OnKillActive ();
}

//------------------------------------------------------------------------------

bool CTexToolDlg::ScrollTexture (short texIds [])
{
	int x, y;

if (!textureManager.ScrollSpeed (texIds [0], &x, &y)) {
	m_xScrollOffset [0] =
	m_yScrollOffset [0] = 0;
	return false;
	}

PaintTexture (&m_textureWnd, m_bkColor, texIds [0], texIds [1], m_xScrollOffset [0], m_yScrollOffset [0]);
//	DrawTexture (texIds [0], texIds [1], m_xScrollOffset [0], m_yScrollOffset [0]);
if (m_xScrollOffset [1] != x || m_yScrollOffset [1] != y) {
	m_xScrollOffset [0] = 0;
	m_yScrollOffset [0] = 0;
	}
m_xScrollOffset [1] = x;
m_yScrollOffset [1] = y;
m_xScrollOffset [0] += x;
m_yScrollOffset [0] += y;
m_xScrollOffset [0] &= 63;
m_yScrollOffset [0] &= 63;
return true;
}

//------------------------------------------------------------------------------

void CTexToolDlg::UpdateTextureClip (short texIds [])
{
	static int direction [2] = {1, 1};
	static int prevTexIds [2] = {-1, -1};

	int			nVersion = DLE.IsD1File ();
	bool			bAnimate = false;
	CTexture*	textures [2] = { null, null };

for (int i = 0; i < (texIds [1] ? 2 : 1); i++) 
	if (prevTexIds [i] != texIds [i]) {
		prevTexIds [i] = texIds [i];
		direction [0] = direction [1] = 1;
		m_frame [0] = m_frame [1] = 0;
		}

for (int i = 0; i < (texIds [1] ? 2 : 1); i++) {
	int nIndex = textureManager.Index (texIds [i]);
	CAnimationClipInfo* aicP = textureManager.AnimationIndex (nIndex);
	if (aicP && (aicP->m_nType != 1) && aicP->FrameCount () && (aicP->Frame (0) == nIndex)) {
		m_frame [i] += direction [i];
		if ((m_frame [i] < 0) || (m_frame [i] >= (int) aicP->FrameCount ())) {
			if (aicP->Bidirectional ()) {
				direction [i] = -direction [i];
				m_frame [i] += direction [i];
				}
			else
				m_frame [i] = 0;
			}
		texIds [i] = aicP->Frame (m_frame [i]);
		textures [i] = textureManager.Textures (-texIds [i] - 1);
		if (textures [i]->Format ())
			textures [i]->SetCurrentFrame (m_frame [i]);
		bAnimate = true;
		}
	else
		textures [i] = textureManager.Textures (texIds [i]);
	}
if (bAnimate)
	PaintTexture (&m_textureWnd, m_bkColor, textures [0], textures [1]);
}

//------------------------------------------------------------------------------

void CTexToolDlg::AnimateTexture (void)
{
if (!TextureIsVisible ())
	return;

	CSegment *segP = m_bOtherSegment ? other->Segment () : current->Segment ();
	CSide	*sideP = m_bOtherSegment ? other->Side () : current->Side ();
	short texture [2] = { sideP->BaseTex (), sideP->OvlTex (0) };

if (ScrollTexture (texture))
	return;

#if 1
	UpdateTextureClip (texture);
#else
// abort if this is not a wall
#ifndef _DEBUG
ushort nWall = sideP->m_info.nWall;
if (nWall >= wallManager.WallCount ())
	return;

// abort if this wall is not a door
//if (wallManager.Wall (nWall)->m_info.type != WALL_DOOR)
//	return;
#endif
	int i;
	static int hold_time [2] = {0,0};
	static int inc [2] = {1,1}; // 1=forward or -1=backwards
	int index [2];
	static ushort d1_anims [] = {
		371, 376, 387, 399, 413, 419, 424, 436, 444, 459,
		472, 486, 492, 500, 508, 515, 521, 529, 536, 543,
		550, 563, 570, 577, 584, 0
		};
// note: 584 is not an anim texture, but it is used to calculate
//       the number of m_frames in 577
// The 0 is used to end the search

	// these are only the animations the frames of which have successive texture ids
	// To properly implement texture animation, the effect and video clip info from the descent 2 data need to be read and used
	static ushort d2_anims [] = { 
		435, 440, 451, 463, 477, 483, 488, 500, 508, 523,
		536, 550, 556, 564, 572, 579, 585, 593, 600, 608,
		615, 628, 635, 642, 649, 664, 672, 687, 702, 717,
		725, 731, 738, 745, 754, 763, 772, 780, 790, 806,
		817, 827, 838, 849, 858, 863, 871, 886, 901, 910,
		0
		};
// note: 910 is not an anim texture, but it is used to calculate
//       the number of m_frames in 901
// The 0 is used to end the search
	ushort *anim; // points to d1_anim or d2_anim depending on m_fileType

// first find out if one of the textures is animated
anim = (DLE.IsD1File ()) ? d1_anims : d2_anims;

for (i = 0; i < 2; i++)
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
	PaintTexture (&m_textureWnd, m_bkColor, texture [0], texture [1]);
	}
#endif
}

//------------------------------------------------------------------------------

void CTexToolDlg::OnTimer (UINT_PTR nIdEvent)
{
if (nIdEvent == (UINT) m_nTimerId)
	AnimateTexture ();
else 
	CToolDlg::OnTimer (nIdEvent);
}

//------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CAnimTexWnd, CWnd)
	ON_WM_TIMER ()
END_MESSAGE_MAP()

//------------------------------------------------------------------------------

CAnimTexWnd::CAnimTexWnd ()
	: CWnd ()
{
m_nAnimTimer = -1;
}

//------------------------------------------------------------------------------

void CAnimTexWnd::OnDestroy (void)
{
StopAnimation ();
}

//------------------------------------------------------------------------------

bool CAnimTexWnd::StopAnimation ()
{
	BOOL result = TRUE;

if (m_nAnimTimer >= 0) {
	result = KillTimer (m_nAnimTimer);
	m_nAnimTimer = -1;
	}

return TRUE == result;
}

//------------------------------------------------------------------------------

bool CAnimTexWnd::StartAnimation (const CTexture *pTexture)
{
if (m_nAnimTimer >= 0)
	StopAnimation ();

m_nAnimTimer = SetTimer (1, pTexture->FrameTime (), null);
m_nTexAll = pTexture->Index ();
m_nFrame = 0;

if (0 == m_nAnimTimer) {
	// failed
	m_nAnimTimer = -1;
	return false;
	}
return true;
}

//------------------------------------------------------------------------------

void CAnimTexWnd::OnTimer (UINT_PTR nIdEvent)
{
if (nIdEvent == (UINT) m_nAnimTimer) {
	const CTexture *pTexture = textureManager.TextureByIndex (m_nTexAll);
	PaintTexture (this, IMG_BKCOLOR, pTexture->GetFrame (m_nFrame));
	m_nFrame++;
	m_nFrame %= pTexture->GetFrameCount ();
	}
else 
	CWnd::OnTimer (nIdEvent);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//eof tooldlg.cpp