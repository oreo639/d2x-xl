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

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolView

BEGIN_MESSAGE_MAP(CLightTool, CToolDlg)
	ON_WM_HSCROLL ()
	ON_BN_CLICKED (IDC_LIGHT_SHOWDELTA, OnShowDelta)
	ON_BN_CLICKED (IDC_LIGHT_DELTA_SHOWSOURCE, OnShowLightSource)
	ON_BN_CLICKED (IDC_LIGHT_SET_VERTEXLIGHT, OnSetVertexLight)
	ON_BN_CLICKED (IDC_DEFAULT_LIGHT_AND_COLOR, OnDefaultLightAndColor)
END_MESSAGE_MAP()

//------------------------------------------------------------------------------

CLightTool::CLightTool (CPropertySheet *pParent)
	: CToolDlg (nLayout ? IDD_LIGHTDATA2 : IDD_LIGHTDATA, pParent)
{
SetDefaults ();
}

//------------------------------------------------------------------------------

void CLightTool::SetDefaults (void)
{
m_bIlluminate = 1;
m_bAvgCornerLight = 1;
m_bScaleLight = 0;
m_bSegmentLight = 1;
m_bDynSegmentLights = 1;
m_bDeltaLight = 1;
m_fBrightness = 100.0;
m_fLightScale = 100.0;
m_fSegmentLight = 50.0;
m_fDeltaLight = 50.0;
m_fVertexLight = 50.0;
m_nNoLightDeltas = 2;
m_bCopyTexLights = 0;
m_staticRenderDepth = DEFAULT_LIGHT_RENDER_DEPTH;
m_deltaRenderDepth = DEFAULT_LIGHT_RENDER_DEPTH;
lightManager.SetRenderDepth ();
m_deltaLightFrameRate = 100;
m_bShowLightSource = 0;
}

//------------------------------------------------------------------------------

BOOL CLightTool::OnInitDialog ()
{
if (!CToolDlg::OnInitDialog ())
	return FALSE;
InitSlider (IDC_LIGHT_RENDER_DEPTH, 1, 10);
InitSlider (IDC_LIGHT_DELTA_RENDER_DEPTH, 1, 10);
InitSlider (IDC_LIGHT_DELTA_FRAMERATE, 10, 100);
for (int i = 20; i < 100; i += 10)
	SlCtrl (IDC_LIGHT_DELTA_FRAMERATE)->SetTic (i);
m_bInited = true;
return TRUE;
}

//------------------------------------------------------------------------------

void CLightTool::DoDataExchange (CDataExchange * pDX)
{
if (!HaveData (pDX)) 
	return;
DDX_Check (pDX, IDC_LIGHT_ILLUMINATE, m_bIlluminate);
DDX_Check (pDX, IDC_LIGHT_AVGCORNERLIGHT, m_bAvgCornerLight);
DDX_Check (pDX, IDC_LIGHT_SCALE, m_bScaleLight);
DDX_Check (pDX, IDC_LIGHT_CUBELIGHT, m_bSegmentLight);
DDX_Check (pDX, IDC_LIGHT_DYNCUBELIGHTS, m_bDynSegmentLights);
DDX_Check (pDX, IDC_LIGHT_CALCDELTA, m_bDeltaLight);
DDX_Double (pDX, IDC_LIGHT_EDIT_ILLUMINATE, m_fBrightness, 0, 1000, null, "brightness must be between 0 and 1000");
DDX_Double (pDX, IDC_LIGHT_EDIT_SCALE, m_fLightScale, 0, 200, null, "light scale must be between 0 and 200%");
DDX_Double (pDX, IDC_LIGHT_EDIT_CUBELIGHT, m_fSegmentLight, 0, 100, null, "robot brightness must be between 0 and 100%");
DDX_Double (pDX, IDC_LIGHT_EDIT_DELTA, m_fDeltaLight, 0, 1000, null, "exploding/blinking light brightness must be between 0 and 1000");
DDX_Radio (pDX, IDC_LIGHT_DELTA_ALL, m_nNoLightDeltas);
DDX_Slider (pDX, IDC_LIGHT_RENDER_DEPTH, m_staticRenderDepth);
DDX_Slider (pDX, IDC_LIGHT_DELTA_RENDER_DEPTH, m_deltaRenderDepth);
DDX_Slider (pDX, IDC_LIGHT_DELTA_FRAMERATE, m_deltaLightFrameRate);
DDX_Double (pDX, IDC_LIGHT_VERTEXLIGHT, m_fVertexLight);
if (!pDX->m_bSaveAndValidate)
	DDX_Text (pDX, IDT_LIGHT_DELTA_FRAMERATE, m_deltaLightFrameRate);
DDX_Check (pDX, IDC_LIGHT_DELTA_SHOWSOURCE, m_bShowLightSource);
if (!pDX->m_bSaveAndValidate)
	((CWnd *) GetDlgItem (IDC_LIGHT_SHOWDELTA))->SetWindowText (bEnableDeltaShading ? "stop" : "animate");
DDX_Check (pDX, IDC_LIGHT_COPYTEXLIGHTS, m_bCopyTexLights);
}

//------------------------------------------------------------------------------

void CLightTool::OnOK ()
{
CHECKMINE;

	bool		bAll;

UpdateData (TRUE);
// make sure there are marked blocks
theMine->m_nNoLightDeltas = m_nNoLightDeltas;
lightManager.SetRenderDepth (m_staticRenderDepth, m_deltaRenderDepth);
if (bAll = !segmentManager.HaveMarkedSides ())
	INFOMSG (" light processing entire mine");
undoManager.Begin (udLight | udSegments);
if (m_bIlluminate)
	lightManager.ComputeStaticLight (m_fBrightness, bAll, m_bCopyTexLights != 0);
if (m_bAvgCornerLight)
	lightManager.CalcAverageCornerLight (bAll);
if (m_bScaleLight)
	lightManager.ScaleCornerLight (m_fLightScale, bAll);
if (m_bSegmentLight)
	lightManager.SetSegmentLight (m_fSegmentLight, (int) bAll, m_bDynSegmentLights != 0);
if (m_bDeltaLight)
	lightManager.ComputeVariableLight (m_fDeltaLight, (int) bAll);
undoManager.End ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CLightTool::OnCancel ()
{
SetDefaults ();
UpdateData (FALSE);
}

//------------------------------------------------------------------------------

void CLightTool::OnShowLightSource ()
{
m_bShowLightSource = BtnCtrl (IDC_LIGHT_DELTA_SHOWSOURCE)->GetCheck ();
DLE.MineView ()->EnableDeltaShading (bEnableDeltaShading, m_deltaLightFrameRate, m_bShowLightSource);
}

//------------------------------------------------------------------------------

void CLightTool::OnShowDelta ()
{
CHECKMINE;

if (!::IsWindow(m_hWnd))
	return;
UpdateData (TRUE);
if (bEnableDeltaShading) {
	((CWnd *) GetDlgItem (IDC_LIGHT_SHOWDELTA))->SetWindowText ("animate");
	DLE.MineView ()->EnableDeltaShading (0, m_deltaLightFrameRate, m_bShowLightSource);
	}
else {
	((CWnd *) GetDlgItem (IDC_LIGHT_SHOWDELTA))->SetWindowText ("stop");
	DLE.MineView ()->EnableDeltaShading (1, m_deltaLightFrameRate, m_bShowLightSource);
	}
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CLightTool::OnSetVertexLight ()
{
CHECKMINE;

	int	nVertexLight;

if (!::IsWindow(m_hWnd))
	return;
UpdateData (TRUE);
nVertexLight = (int) (m_fVertexLight * f1_0 / 100.0);

	short			nSide, nVertex, i;
	CSegment*	segP = segmentManager.Segment (0);
	CSide*		sideP;
	bool			bChange = false;

undoManager.Begin (udSegments);
for (CSegmentIterator si; si; si++) {
	segP = &(*si);
	for (nSide = 0, sideP = segP->m_sides; nSide < 6; nSide++, sideP++) {
		for (i = 0; i < 4; i++) {
			nVertex = segP->m_info.verts [sideVertTable [nSide][i]];
			if (vertexManager.Status (nVertex) & MARKED_MASK) {
				sideP->m_info.uvls [i].l = nVertexLight;
				bChange = true;
				}
			}
		}
	}
undoManager.End ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CLightTool::OnDefaultLightAndColor ()
{
lightManager.LoadDefaults ();
}

//------------------------------------------------------------------------------

void CLightTool::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
	int	nPos = pScrollBar->GetScrollPos ();
	CRect	rc;

if (pScrollBar == (CScrollBar *) GetDlgItem (IDC_LIGHT_DELTA_FRAMERATE)) {
	switch (scrollCode) {
		case SB_LINEUP:
			nPos++;
			break;
		case SB_LINEDOWN:
			nPos--;
			break;
		case SB_PAGEUP:
			nPos += 10;
			break;
		case SB_PAGEDOWN:
			nPos -= 10;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	if (nPos < 10)
		nPos = 10;
	else if (nPos > 100)
		nPos = 100;
	m_deltaLightFrameRate = nPos;
	UpdateData (FALSE);
	}
}

//------------------------------------------------------------------------------


//eof lightdlg.cpp