#include "define.h"
#include "types.h"
#include "texturefilter.h"

//------------------------------------------------------------------------

tTexFilterInfo texFiltersD1 [TEXFILTER_SIZE_D1] = {
	{{0, 5}, TXT_GRAY_ROCK, 0},
	{{6, 6}, TXT_BLUE_ROCK, 0},
	{{7, 7}, TXT_YELLOW_ROCK, 0},
	{{8, 8}, TXT_GRAY_ROCK, 0},
	{{9, 10}, TXT_RED_ROCK, 0},
	{{11, 11}, TXT_BLUE_ROCK, 0},
	{{12, 12}, TXT_GRAY_ROCK, TXT_STONES},
	{{13, 13}, TXT_GRAY_ROCK, 0},
	{{14, 14}, TXT_TARMAC, 0},
	{{15, 15}, TXT_GRAY_ROCK, 0},
	{{16, 16}, TXT_BLUE_ROCK, 0},
	{{17, 17}, TXT_RED_ROCK, 0},
	{{18, 18}, TXT_BLUE_ROCK, TXT_GRAY_ROCK},
	{{19, 19}, TXT_RED_ROCK, 0},
	{{20, 21}, TXT_STEEL, 0},
	{{22, 30}, TXT_RED_ROCK, 0},
	{{31, 31}, TXT_BROWN_ROCK, 0},
	{{32, 32}, TXT_RED_ROCK, TXT_STONES},
	{{33, 38}, TXT_RED_ROCK, 0},
	{{39, 39}, TXT_BRICK, 0},
	{{40, 40}, TXT_GRAY_ROCK, 0},
	{{41, 41}, TXT_FLOOR, 0},
	{{42, 42}, TXT_GRAY_ROCK, 0},
	{{43, 43}, TXT_RED_ROCK, 0},
	{{44, 44}, TXT_BLUE_ROCK, 0},
	{{45, 45}, TXT_RED_ROCK, TXT_LAVA},
	{{46, 47}, TXT_RED_ROCK, 0},
	{{48, 49}, TXT_GRAY_ROCK, 0},
	{{50, 50}, TXT_RED_ROCK, 0},
	{{51, 53}, TXT_BROWN_ROCK, 0},
	{{54, 55}, TXT_RED_ROCK, 0},
	{{56, 56}, TXT_BROWN_ROCK, 0},
	{{57, 57}, TXT_RED_ROCK, 0},
	{{58, 58}, TXT_GRAY_ROCK, TXT_STONES},
	{{59, 59}, TXT_ICE, 0},
	{{60, 60}, TXT_BROWN_ROCK, TXT_STONES},
	{{61, 61}, TXT_GRAY_ROCK, 0},
	{{62, 66}, TXT_BROWN_ROCK, 0},
	{{67, 69}, TXT_GRAY_ROCK, 0},
	{{70, 73}, TXT_GREEN_ROCK, 0},
	{{74, 75}, TXT_RED_ROCK, 0},
	{{76, 77}, TXT_BLUE_ROCK, 0},
	{{78, 78}, TXT_BROWN_ROCK, 0},
	{{79, 79}, TXT_BLUE_ROCK, TXT_ICE},
	{{80, 81}, TXT_BROWN_ROCK, 0},
	{{82, 82}, TXT_GRAY_ROCK, 0},
	{{83, 83}, TXT_BLUE_ROCK, 0},
	{{84, 85}, TXT_GRAY_ROCK, 0},
	{{86, 87}, TXT_BLUE_ROCK, 0},
	{{88, 89}, TXT_BROWN_ROCK, 0},
	{{90, 90}, TXT_BLUE_ROCK, 0},
	{{91, 91}, TXT_BROWN_ROCK, 0},
	{{92, 96}, TXT_RED_ROCK, 0},
	{{97, 97}, TXT_BROWN_ROCK, 0},
	{{98, 98}, TXT_GREEN_ROCK, 0},
	{{99, 99}, TXT_BROWN_ROCK, 0},
	{{100, 100}, TXT_GRAY_ROCK, 0},
	{{101, 105}, TXT_BROWN_ROCK, 0},
	{{106, 109}, TXT_GREEN_ROCK, 0},
	{{110, 110}, TXT_BLUE_ROCK, TXT_ICE},
	{{111, 116}, TXT_GREEN_ROCK, TXT_GRASS},
	{{117, 118}, TXT_BROWN_ROCK, 0},
	{{119, 120}, TXT_CONCRETE, 0},
	{{121, 122}, TXT_BROWN_ROCK, 0},
	{{123, 123}, TXT_GREEN_ROCK, TXT_GRASS},
	{{124, 125}, TXT_BROWN_ROCK, 0},
	{{126, 126}, TXT_BROWN_ROCK, TXT_BLUE_ROCK},
	{{127, 127}, TXT_BLUE_ROCK, 0},
	{{128, 141}, TXT_RED_ROCK, 0},
	{{142, 143}, TXT_RED_ROCK, TXT_LAVA},
	{{144, 144}, TXT_BLUE_ROCK, TXT_ICE},
	{{145, 145}, TXT_GREEN_ROCK, 0},
	{{146, 146}, TXT_BLUE_ROCK, TXT_ICE},
	{{147, 149}, TXT_BROWN_ROCK, 0},
	{{150, 151}, TXT_RED_ROCK, 0},
	{{152, 153}, TXT_GREEN_ROCK, TXT_STONES},
	{{154, 154}, TXT_GRAY_ROCK, TXT_LAVA},
	{{155, 155}, TXT_BROWN_ROCK, TXT_LAVA},
	{{156, 157}, TXT_STEEL, 0},
	{{158, 158}, TXT_CONCRETE, TXT_WALL},
	{{159, 159}, TXT_TECH, TXT_WALL},
	{{160, 160}, TXT_CONCRETE, TXT_WALL},
	{{161, 161}, TXT_TECH, TXT_WALL},
	{{162, 163}, TXT_CONCRETE, TXT_WALL},
	{{164, 164}, TXT_CONCRETE, TXT_SIGN | TXT_WALL},
	{{165, 171}, TXT_CONCRETE, TXT_WALL},
	{{172, 173}, TXT_CONCRETE, TXT_SIGN | TXT_WALL},
	{{174, 185}, TXT_CONCRETE, TXT_WALL},
	{{186, 190}, TXT_STEEL, 0},
	{{191, 196}, TXT_CONCRETE, TXT_WALL},
	{{197, 197}, TXT_STEEL, 0},
	{{198, 199}, TXT_CONCRETE, TXT_WALL},
	{{200, 200}, TXT_CONCRETE, TXT_WALL | TXT_GRATE},
	{{201, 207}, TXT_CONCRETE, TXT_WALL},
	{{208, 208}, TXT_CONCRETE, TXT_WALL | TXT_SIGN},
	{{209, 211}, TXT_CONCRETE, TXT_WALL},
	{{212, 214}, TXT_CONCRETE, TXT_WALL | TXT_LIGHT},
	{{215, 217}, TXT_CONCRETE, TXT_WALL},
	{{218, 218}, TXT_CONCRETE, TXT_WALL | TXT_GRATE},
	{{219, 219}, TXT_CONCRETE, TXT_WALL},
	{{220, 220}, TXT_CONCRETE, TXT_WALL | TXT_GRATE},
	{{221, 222}, TXT_CONCRETE, TXT_WALL | TXT_LIGHT},
	{{223, 230}, TXT_TECH, 0},
	{{231, 234}, TXT_CONCRETE, TXT_WALL},
	{{235, 237}, TXT_CONCRETE, TXT_WALL | TXT_GRATE},
	{{238, 238}, TXT_CONCRETE, TXT_WALL},
	{{239, 239}, TXT_STEEL, TXT_GRATE},
	{{240, 243}, TXT_TECH, 0},
	{{244, 244}, TXT_CONCRETE, TXT_WALL},
	{{245, 245}, TXT_GRATE, TXT_STEEL},
	{{246, 246}, TXT_GRATE, 0},
	{{247, 249}, TXT_GRATE, TXT_STEEL},
	{{250, 253}, TXT_LIGHT, 0},
	{{254, 255}, TXT_GRATE, 0},
	{{256, 260}, TXT_CONCRETE, TXT_WALL},
	{{261, 262}, TXT_CEILING, TXT_STEEL},
	{{263, 263}, TXT_FLOOR, TXT_STEEL},
	{{264, 269}, TXT_LIGHT, 0},
	{{270, 277}, TXT_FLOOR, 0},
	{{278, 289}, TXT_LIGHT, 0},
	{{290, 291}, TXT_FLOOR, 0},
	{{292, 294}, TXT_LIGHT, 0},
	{{295, 296}, TXT_GRATE, TXT_TECH},
	{{297, 297}, TXT_GRATE, 0},
	{{298, 298}, TXT_SIGN, 0},
	{{299, 300}, TXT_TECH, TXT_SIGN},
	{{301, 301}, TXT_ENERGY, 0},
	{{302, 308}, TXT_SIGN, TXT_LABEL},
	{{309, 309}, TXT_GRATE, 0},
	{{310, 312}, TXT_SIGN, TXT_LABEL},
	{{313, 314}, TXT_SIGN, TXT_STRIPES},
	{{315, 315}, TXT_TECH, 0},
	{{316, 316}, TXT_RED_ROCK, TXT_TECH},
	{{317, 317}, TXT_SIGN, TXT_LABEL},
	{{318, 318}, TXT_SIGN, TXT_STRIPES},
	{{319, 321}, TXT_SIGN, TXT_LABEL},
	{{322, 322}, TXT_ENERGY, 0},
	{{323, 323}, TXT_SIGN, TXT_LABEL},
	{{324, 324}, TXT_GRATE, 0},
	{{325, 325}, TXT_FAN, 0},
	{{326, 326}, TXT_TECH, TXT_SIGN},
	{{327, 327}, TXT_SIGN, TXT_LIGHT | TXT_MOVE},
	{{328, 328}, TXT_ENERGY, TXT_LIGHT},
	{{329, 329}, TXT_FAN, 0},
	{{330, 331}, TXT_SIGN, TXT_LIGHT | TXT_MOVE},
	{{332, 332}, TXT_GREEN_ROCK, TXT_TECH},
	{{333, 333}, TXT_RED_ROCK, TXT_TECH},
	{{334, 337}, TXT_CONCRETE, TXT_TECH},
	{{338, 339}, TXT_TECH, 0},
	{{340, 340}, TXT_TARMAC, 0},
	{{341, 354}, TXT_SIGN, TXT_LIGHT | TXT_MONITOR},
	{{355, 356}, TXT_LAVA, TXT_LIGHT},
	{{357, 370}, TXT_SIGN, TXT_LIGHT | TXT_MONITOR},
	{{371, 577}, TXT_DOOR, 0}
};

tTexFilter texFiltersD2 [TEXFILTER_SIZE_D2] = {
	{{0, 0}, TXT_GRAY_ROCK, TXT_CONCRETE},
	{{1, 5}, TXT_GRAY_ROCK, 0},
	{{6, 6}, TXT_BLUE_ROCK, 0},
	{{7, 7}, TXT_RED_ROCK, 0},
	{{8, 14}, TXT_GRAY_ROCK, 0},
	{{15, 15}, TXT_BROWN_ROCK, 0},
	{{16, 16}, TXT_RED_ROCK, TXT_STONES},
	{{17, 21}, TXT_RED_ROCK, 0},
	{{22, 23}, TXT_GRAY_ROCK, 0},
	{{24, 24}, TXT_RED_ROCK, 0},
	{{25, 25}, TXT_RED_ROCK, TXT_LAVA},
	{{26, 27}, TXT_RED_ROCK, 0},
	{{28, 28}, TXT_GRAY_ROCK, 0},
	{{29, 31}, TXT_BROWN_ROCK, 0},
	{{32, 32}, TXT_RED_ROCK, 0},
	{{33, 33}, TXT_BROWN_ROCK, 0},
	{{34, 34}, TXT_GRAY_ROCK, TXT_STONES},
	{{35, 35}, TXT_ICE, 0},
	{{36, 36}, TXT_BROWN_ROCK, TXT_STONES},
	{{37, 37}, TXT_GRAY_ROCK, 0},
	{{38, 42}, TXT_BROWN_ROCK, 0},
	{{43, 43}, TXT_GRAY_ROCK, 0},
	{{44, 44}, TXT_RED_ROCK, 0},
	{{45, 45}, TXT_TARMAC, 0},
	{{46, 49}, TXT_GREEN_ROCK, 0},
	{{50, 51}, TXT_RED_ROCK, 0},
	{{52, 53}, TXT_BLUE_ROCK, 0},
	{{54, 54}, TXT_BROWN_ROCK, 0},
	{{55, 55}, TXT_BLUE_ROCK, TXT_ICE},
	{{56, 58}, TXT_BROWN_ROCK, 0},
	{{59, 59}, TXT_BLUE_ROCK, 0},
	{{60, 61}, TXT_GRAY_ROCK, 0},
	{{62, 63}, TXT_BLUE_ROCK, 0},
	{{64, 64}, TXT_BROWN_ROCK, 0},
	{{65, 65}, TXT_BLUE_ROCK, 0},
	{{66, 66}, TXT_BROWN_ROCK, 0},
	{{67, 71}, TXT_RED_ROCK, 0},
	{{72, 72}, TXT_BROWN_ROCK, 0},
	{{73, 73}, TXT_GREEN_ROCK, 0},
	{{74, 74}, TXT_BROWN_ROCK, 0},
	{{75, 75}, TXT_GRAY_ROCK, 0},
	{{76, 80}, TXT_BROWN_ROCK, 0},
	{{81, 81}, TXT_GREEN_ROCK, TXT_ICE},
	{{82, 82}, TXT_GREEN_ROCK, 0},
	{{83, 83}, TXT_GREEN_ROCK, TXT_ICE},
	{{84, 84}, TXT_GREEN_ROCK, 0},
	{{85, 85}, TXT_GREEN_ROCK, TXT_ICE},
	{{85, 85}, TXT_GREEN_ROCK | TXT_BLUE_ROCK, TXT_ICE},
	{{86, 91}, TXT_GREEN_ROCK, TXT_GRASS},
	{{92, 93}, TXT_BROWN_ROCK, 0},
	{{94, 95}, TXT_CONCRETE, 0},
	{{96, 97}, TXT_BROWN_ROCK, 0},
	{{98, 98}, TXT_GREEN_ROCK, TXT_GRASS},
	{{99, 100}, TXT_BROWN_ROCK, TXT_RED_ROCK},
	{{101, 101}, TXT_BROWN_ROCK | TXT_BLUE_ROCK, TXT_ICE},
	{{102, 102}, TXT_BLUE_ROCK, 0},
	{{103, 103}, TXT_BROWN_ROCK, 0},
	{{104, 105}, TXT_RED_ROCK, TXT_LAVA},
	{{106, 106}, TXT_RED_ROCK, 0},
	{{107, 111}, TXT_BROWN_ROCK, 0},
	{{112, 114}, TXT_RED_ROCK, 0},
	{{115, 116}, TXT_RED_ROCK, TXT_LAVA},
	{{117, 117}, TXT_BLUE_ROCK, TXT_ICE},
	{{118, 118}, TXT_GREEN_ROCK, 0},
	{{119, 119}, TXT_BLUE_ROCK, 0},
	{{120, 121}, TXT_BROWN_ROCK, 0},
	{{122, 123}, TXT_BROWN_ROCK | TXT_RED_ROCK, 0},
	{{124, 125}, TXT_GREEN_ROCK, TXT_GRASS},
	{{126, 127}, TXT_RED_ROCK, 0},
	{{128, 138}, TXT_ICE, 0},
	{{139, 139}, TXT_ICE, 0},
	{{140, 142}, TXT_BROWN_ROCK, 0},
	{{143, 145}, TXT_SAND, 0},
	{{146, 147}, TXT_RED_ROCK, 0},
	{{148, 148}, TXT_BROWN_ROCK, 0},
	{{149, 151}, TXT_BROWN_ROCK, TXT_SAND},
	{{152, 152}, TXT_RED_ROCK, 0},
	{{153, 154}, TXT_BROWN_ROCK, TXT_SAND},
	{{155, 168}, TXT_RED_ROCK, 0},
	{{169, 171}, TXT_GREEN_ROCK, TXT_GRASS},
	{{172, 172}, TXT_BLUE_ROCK, TXT_ICE},
	{{173, 176}, TXT_ICE, 0},
	{{177, 177}, TXT_BROWN_ROCK},
	{{178, 183}, TXT_ICE},
	{{184, 184}, TXT_GREEN_ROCK | TXT_GRASS},
	{{185, 190}, TXT_ICE, 0},
	{{191, 191}, TXT_CONCRETE, 0},
	{{192, 193}, TXT_ICE, 0},
	{{194, 194}, TXT_GREEN_ROCK, TXT_ICE},
	{{195, 195}, TXT_ICE, 0},
	{{196, 196}, TXT_GREEN_ROCK, TXT_ICE},
	{{197, 197}, TXT_BROWN_ROCK, 0},
	{{198, 198}, TXT_GRAY_ROCK, 0},
	{{199, 199}, TXT_BROWN_ROCK, 0},
	{{200, 201}, TXT_STEEL, 0},
	{{202, 217}, TXT_CONCRETE, TXT_WALL},
	{{204, 204}, TXT_CONCRETE, TXT_SIGN | TXT_WALL},
	{{205, 206}, TXT_CONCRETE, TXT_WALL},
	{{207, 208}, TXT_CONCRETE, TXT_SIGN | TXT_WALL},
	{{209, 217}, TXT_CONCRETE, TXT_WALL},
	{{218, 222}, TXT_STEEL, 0},
	{{223, 225}, TXT_CONCRETE, TXT_WALL},
	{{226, 226}, TXT_STEEL, 0},
	{{227, 232}, TXT_CONCRETE, TXT_WALL},
	{{233, 233}, TXT_CONCRETE, TXT_SIGN | TXT_WALL},
	{{234, 234}, TXT_CONCRETE, TXT_WALL},
	{{235, 237}, TXT_CONCRETE, TXT_LIGHT | TXT_WALL},
	{{238, 240}, TXT_CONCRETE, TXT_WALL},
	{{241, 241}, TXT_CONCRETE, TXT_GRATE | TXT_WALL},
	{{242, 242}, TXT_CONCRETE, TXT_WALL},
	{{243, 244}, TXT_CONCRETE, TXT_LIGHT | TXT_WALL},
	{{245, 246}, TXT_TECH, 0},
	{{247, 248}, TXT_GRATE, TXT_STEEL},
	{{249, 252}, TXT_CONCRETE, TXT_WALL},
	{{253, 255}, TXT_CONCRETE, TXT_GRATE | TXT_WALL},
	{{256, 256}, TXT_CONCRETE, TXT_WALL},
	{{257, 257}, TXT_GRATE, TXT_STEEL},
	{{258, 258}, TXT_CONCRETE, TXT_WALL},
	{{259, 262}, TXT_GRATE, TXT_STEEL},
	{{263, 264}, TXT_GRATE, 0},
	{{265, 265}, TXT_CONCRETE, 0},
	{{266, 266}, TXT_FLOOR, 0},
	{{267, 269}, TXT_GRATE, 0},
	{{270, 272}, TXT_TECH, 0},
	{{273, 274}, TXT_STEEL, TXT_CEILING},
	{{275, 279}, TXT_LIGHT, 0},
	{{280, 287}, TXT_FLOOR, 0},
	{{288, 302}, TXT_LIGHT, 0},
	{{303, 304}, TXT_FLOOR, 0},
	{{305, 307}, TXT_LIGHT, 0},
	{{308, 310}, TXT_GRATE, 0},
	{{311, 312}, TXT_SIGN, TXT_TECH | TXT_STRIPES},
	{{313, 313}, TXT_ENERGY, 0},
	{{314, 320}, TXT_SIGN, TXT_LABEL},
	{{321, 321}, TXT_GRATE, 0},
	{{322, 324}, TXT_SIGN, TXT_LABEL},
	{{325, 326}, TXT_SIGN, TXT_STRIPES},
	{{327, 327}, TXT_RED_ROCK, TXT_TECH},
	{{328, 328}, TXT_SIGN, TXT_LABEL},
	{{329, 329}, TXT_SIGN, TXT_STRIPES},
	{{330, 332}, TXT_SIGN, TXT_LABEL},
	{{333, 333}, TXT_ENERGY, 0},
	{{334, 334}, TXT_SIGN, TXT_LABEL},
	{{335, 335}, TXT_GRATE, 0},
	{{336, 336}, TXT_FAN, 0},
	{{337, 337}, TXT_SIGN, TXT_TECH | TXT_STRIPES},
	{{338, 339}, TXT_SIGN, TXT_STRIPES},
	{{340, 347}, TXT_LIGHT, 0},
	{{348, 349}, TXT_LIGHT, TXT_STEEL | TXT_MOVE},
	{{350, 350}, TXT_GRATE, TXT_STEEL | TXT_MOVE},
	{{351, 351}, TXT_SIGN, TXT_LABEL},
	{{352, 352}, TXT_SIGN, TXT_LIGHT | TXT_MOVE},
	{{353, 353}, TXT_ENERGY, TXT_LIGHT},
	{{354, 354}, TXT_FAN, 0},
	{{355, 355}, TXT_GREEN_ROCK, TXT_TECH},
	{{356, 359}, TXT_CONCRETE , TXT_TECH | TXT_WALL | TXT_LIGHT},
	{{360, 361}, TXT_LIGHT, TXT_TECH},
	{{362, 362}, TXT_TARMAC, 0},
	{{363, 377}, TXT_SIGN, TXT_LIGHT | TXT_MONITOR},
	{{378, 378}, TXT_LAVA, 0},
	{{379, 398}, TXT_SIGN, TXT_LIGHT},
	{{399, 403}, TXT_WATER, 0},
	{{404, 409}, TXT_LAVA, 0},
	{{410, 412}, TXT_LIGHT, 0},
	{{413, 419}, TXT_SWITCH, 0},
	{{420, 420}, TXT_FORCEFIELD, TXT_LIGHT},
	{{423, 424}, TXT_SIGN, TXT_LIGHT | TXT_STRIPES},
	{{425, 425}, TXT_SIGN, TXT_LIGHT},
	{{426, 426}, TXT_LIGHT, TXT_TECH},
	{{427, 429}, TXT_LIGHT, 0},
	{{430, 431}, TXT_SIGN, TXT_LIGHT},
	{{432, 432}, TXT_FORCEFIELD, TXT_LIGHT},
	{{433, 434}, TXT_LIGHT, TXT_TECH},
	{{435, 901}, TXT_DOOR, 0}
	};

//------------------------------------------------------------------------

short CTextureFilter::FilterIndex (short nTexture)
{
	short	m, l = 0, r = TEX_FILTER_SIZE - 1;

do {
	m = (l + r) / 2;
	if (nTexture < TEXTURE_FILTERS [m].m_range.nMin)
		r = m - 1;
	else if (nTexture > TEXTURE_FILTERS [m].m_range.nMax)
		l = m + 1;
	else
		return m;
	}
while (l <= r);
return -1;
}

//------------------------------------------------------------------------

uint CTextureFilter::Filter (short nTexture)
{
	short	m = TexFilterIndex (nTexture);

return (m < 0) ? 0 : TEXTURE_FILTERS [m].nFilter1;
}

//------------------------------------------------------------------------

int CTextureFilter::CompareFilters (int nTexture, int mTexture, uint mf, uint mf2)
{
	short	n = TexFilterIndex (nTexture);
	uint	nf = TEXTURE_FILTERS [n].nFilter1,
			nf2 = TEXTURE_FILTERS [n].nFilter2;

//CBRK (((nf == TXT_DOOR) && (mf == TXT_SAND)) || ((mf == TXT_DOOR) && (nf == TXT_SAND)));
if (nf < mf)
	return -1;
else if (nf > mf)
	return 1;
else if (nf2 < mf2)
	return -1;
else if (nf2 > mf2)
	return 1;
else
	return (nTexture < mTexture) ? -1 : (nTexture > mTexture) ? 1 : 0;
}

//------------------------------------------------------------------------

void CTextureFilter::QSortTexMap (short left, short right)
{
	short		mTexture = m_mapViewToTex [(left + right) / 2];
	short		m = TexFilterIndex (mTexture);
	uint		mf, mf2;
	short		h, l = left, r = right;

mf = TEXTURE_FILTERS [m].nFilter;
mf2 = TEXTURE_FILTERS [m].n2ndFilter;
do {
	while (QCmpTexFilters (m_mapViewToTex [l], mTexture, mf, mf2) < 0)
		l++;
	while (QCmpTexFilters (m_mapViewToTex [r], mTexture, mf, mf2) > 0)
		r--;
	if (l <= r) {
		if (l < r) {
			h = m_mapViewToTex [l];
			m_mapViewToTex [l] = m_mapViewToTex [r];
			m_mapViewToTex [r] = h;
			}
		l++;
		r--;
		}
	}
while (l < r);
if (l < right)
	QSortTexMap (l, right);
if (left < r)
	QSortTexMap (left, r);
}

//------------------------------------------------------------------------

void CTextureFilter::CreateTexMap (void)
{
QSortTexMap (0, m_nTextures [1] - 1);
for (int i = 0; i < m_nTextures [1]; i++)
	m_mapTexToView [m_mapViewToTex [i]] = i;
}

//------------------------------------------------------------------------
// CTextureFilter::EvVScroll()
//
// Action - Receives scroll movements messages and updates scroll position
//          Also calls refreshdata() to update the bitmap.
//------------------------------------------------------------------------

void CTextureFilter::OnVScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
UINT nPos = GetScrollPos (SB_VERT);
CRect rect;
GetClientRect(rect);

m_bDelayRefresh = true;
switch (scrollCode) {
	case SB_LINEUP:
		nPos--;
		break;
	case SB_LINEDOWN:
		nPos++;
		break;
	case SB_PAGEUP:
		nPos -= m_viewSpace.cy;
		break;
	case SB_PAGEDOWN:
		nPos += m_viewSpace.cy;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nPos = thumbPos;
		break;
	case SB_ENDSCROLL:
		m_bDelayRefresh = false;
		Refresh (false);
		return;
}
SetScrollPos (SB_VERT, nPos, TRUE);
Refresh ();
}

//------------------------------------------------------------------------
// CTextureFilter::EvLButtonDown()
//
// Action - Sets texture 1 of current side to texture under mouse pointer
//
// Notes - If shift is held down and object is poly type, then object's
//         texture is changed instead of cube's texture.
//------------------------------------------------------------------------

void CTextureFilter::OnLButtonDown(UINT nFlags, CPoint point)
{
if (!theMine) return;

	short nBaseTex;

if (PickTexture (point, nBaseTex))
	return;
if (nFlags & MK_SHIFT) {
	CGameObject *objP = theMine->Objects (theMine->Current ()->nObject);
   if (objP->m_info.renderType != RT_POLYOBJ) 
		return;
	objP->rType.polyModelInfo.tmap_override = nBaseTex;
  } 
else if (nFlags & MK_CONTROL) {
	DLE.ToolView ()->TriggerTool ()->SetTexture (nBaseTex, -1);
	DLE.ToolView ()->TriggerTool ()->Refresh ();
	}
else {
	theMine->SetTexture (-1, -1, nBaseTex, -1);
	Refresh ();
	}
DLE.SetModified (TRUE);
}

//------------------------------------------------------------------------

void CTextureFilter::OnRButtonDown(UINT nFlags, CPoint point)
{
if (!theMine) return;

	CSide *sideP = theMine->CurrSide ();
	short	nBaseTex;

if (PickTexture (point, nBaseTex))
	return;
if (nFlags & MK_CONTROL) {
	DLE.ToolView ()->TriggerTool ()->SetTexture (-1, nBaseTex);
	DLE.ToolView ()->TriggerTool ()->Refresh ();
	}
else {
	theMine->SetTexture (-1, -1, -1, nBaseTex);
	Refresh ();
	}
DLE.SetModified (TRUE);
}

//------------------------------------------------------------------------
// CTextureFilter::PickTexture()
//
// Action - calculates position of textures and matches it up with mouse.
// If a match is found, nBaseTex is defined and 0 is returned
//------------------------------------------------------------------------

int CTextureFilter::PickTexture(CPoint &point,short &nBaseTex) 
{
if (!theMine) 
	return 1;

//if (!m_pTextures)
//	return 0;

  nBaseTex = 0;  // set default in case value is used when function fails

  CRect rect;
  GetClientRect(rect);

//if (m_nRows [m_bShowAll] <= m_viewSpace.cy)
//	return 1;

UINT nPos = GetScrollPos (SB_VERT);
int nOffset = nPos * m_viewSpace.cx;
byte pFilter [1 + MAX_D2_TEXTURES / 8];

FilterTextures (pFilter, m_bShowAll);
int x = 0;
int y = 0;

x = point.x / m_iconSpace.cx;
y = point.y / m_iconSpace.cy;
int h = nOffset + y * m_viewSpace.cx + x + 1;
int i;
for (i = 0; i < m_nTextures [1]; i++) {
	if (BITSET (pFilter, i)) //pFilter [i / 8] & (1 << (i & 7)))
		if (!--h) {
			nBaseTex = m_mapViewToTex [i]; //m_pTextures [i];
			return 0; // return success
			}
	}
return 1; // return failure
}

//------------------------------------------------------------------------
// TextureIndex()
//
// looks up texture index number for nBaseTex
// returns 0 to m_nTextures-1 on success
// returns -1 on failure
//------------------------------------------------------------------------

int CTextureFilter::TextureIndex(short nBaseTex) 
{
return ((nBaseTex < 0) || (nBaseTex >= sizeof (m_mapTexToView) / sizeof (m_mapTexToView [0]))) ? 0 : m_mapTexToView [nBaseTex];
}

//------------------------------------------------------------------------
// FilterTexture()
//
// Determines which textures to display based on which have been used
//------------------------------------------------------------------------

void CTextureFilter::FilterTextures (byte *pFilter, BOOL bShowAll) 
{
if (bShowAll) {
	if (m_nTxtFilter == 0xFFFFFFFF)
		memset (pFilter, 0xFF, (MAX_D2_TEXTURES + 7) / 8);
	else {
		memset (pFilter, 0, (MAX_D2_TEXTURES + 7) / 8);
		m_nTextures [0] = 0;
		int i, f = m_nTxtFilter & ~TXT_MOVE;
		for (i = 0; i < m_nTextures [1]; i++) {
			int t = m_mapViewToTex [i];
			int j = TexFilterIndex (t);
			if ((TEXTURE_FILTERS [j].nFilter | TEXTURE_FILTERS [j].n2ndFilter) & f) {
				SETBIT (pFilter, i);
				m_nTextures [0]++;
				}
			}
		}
	}
else {
	ushort nSegment,nSide;
	CSegment *segP;

	memset (pFilter, 0, (MAX_D2_TEXTURES + 7) / 8);
	m_nTextures [0] = 0;
	for (nSegment = 0, segP = theMine->Segments (0); nSegment < theMine->SegCount (); nSegment++, segP++)
      for (nSide = 0;nSide < 6; nSide++) {
			ushort nWall = segP->m_sides[nSide].m_info.nWall;
			if ((segP->Child (nSide) == -1) ||
				 (nWall < theMine->GameInfo ().walls.count && 
				  theMine->Walls (nWall)->m_info.type != WALL_OPEN)) {
				int t = segP->m_sides [nSide].m_info.nBaseTex;
				int i = TextureIndex (t);
				int j = TexFilterIndex (t);
				if ((i >= 0) && !BITSET (pFilter, i) && 
					 ((TEXTURE_FILTERS [j].nFilter | TEXTURE_FILTERS [j].n2ndFilter) & m_nTxtFilter)) {
					SETBIT (pFilter, i);
					m_nTextures [0]++;
					}
//					pFilter[t/8] |= (1<<(t&7));
				t = segP->m_sides [nSide].m_info.nOvlTex & 0x3fff;
				i = TextureIndex (t);
				j = TexFilterIndex (t);
				if ((t > 0) && !BITSET (pFilter, i)) {
					SETBIT (pFilter, i);
//					pFilter[t/8] |= (1<<(t&7));
					m_nTextures [0]++;
					}
				}
			}
	}
}

								/*---------------------------*/

void CTextureFilter::RecalcLayout () 
{
CHECKMINE;

	CRect rect;
	GetClientRect(rect);

m_viewSpace.cx = rect.Width () / m_iconSpace.cx;
m_viewSpace.cy = rect.Height () / m_iconSpace.cy;

int nOffset = 0;
m_bShowAll = ((m_viewFlags & eViewMineUsedTextures) == 0);

if (!(m_viewSpace.cx && m_viewSpace.cy))
	return;

byte pFilter [(MAX_D2_TEXTURES + 7) / 8];
FilterTextures (pFilter, m_bShowAll);
m_nRows [ShowAll ()] = (m_nTextures [ShowAll ()] + m_viewSpace.cx - 1) / m_viewSpace.cx;

// read scroll position to get offset for bitmap tiles
SetScrollRange (SB_VERT, 0, m_nRows [m_bShowAll] - m_viewSpace.cy, TRUE);
// if there is enough lines to show all textures, then hide the scroll bar
if (m_nRows [ShowAll ()] <= m_viewSpace.cy) {
	ShowScrollBar (SB_VERT,FALSE);
	nOffset = 0;
	}
else {
// otherwise, calculate the number of rows to skip
	ShowScrollBar (SB_VERT,TRUE);
	UINT nPos = GetScrollPos (SB_VERT);
	int i, j = nPos * m_viewSpace.cx; 
	for (i = 0, nOffset = 0; (i < m_nTextures [1]); i++)
		if (ShowAll () || BITSET (pFilter, i)) {
			nOffset++;
			if (--j < 0)
				break;
			}
	}
SetScrollPos (SB_VERT, nOffset / m_viewSpace.cx, TRUE);
// figure out position of current texture
int nBaseTex = theMine->CurrSide ()->m_info.nBaseTex;
int nOvlTex = theMine->CurrSide ()->m_info.nOvlTex & 0x3fff; // strip rotation info
CTexture tex (textureManager.m_bmBuf);

CDC *pDC = GetDC();
if (!pDC) {
	ErrorMsg ("No device context for texture picker available");
	return;
	}
BITMAPINFO* bmi = paletteManager.BMI ();

   // realize pallette for 256 color displays
CPalette *oldPalette = pDC->SelectPalette (paletteManager.Render (), FALSE);
pDC->RealizePalette();
pDC->SetStretchBltMode(STRETCH_DELETESCANS);
int x = 0;
int y = 0;
for (int i = 0; i < m_nTextures [1]; i++) {
	if (!ShowAll ()) {
		if (!BITSET (pFilter, i))
			continue;
			}
	if (nOffset &&	--nOffset)
		continue;
	if (!textureManager.Define (m_mapViewToTex [i], 0, &tex, 0, 0)) {
		bmi->bmiHeader.biWidth = tex.m_info.width;
		bmi->bmiHeader.biHeight = tex.m_info.width;
		StretchDIBits (*pDC, 3 + x * m_iconSpace.cx, 3 + y * m_iconSpace.cy, 
							m_iconSize.cx, m_iconSize.cy, 0, 0, tex.m_info.width, tex.m_info.width, 
							(void *)tex.m_info.bmData, bmi, DIB_RGB_COLORS, SRCCOPY);
		}
// pick color for box drawn around texture
	if (m_mapViewToTex [i] == nBaseTex)
		pDC->SelectObject (GetStockObject (WHITE_PEN));
	else if (i && (m_mapViewToTex [i] == nOvlTex)) // note: 0 means no texture
		pDC->SelectObject (m_penCyan);
	else
		pDC->SelectObject (GetStockObject (BLACK_PEN));
	pDC->MoveTo (1 + x * m_iconSpace.cx, 1 + y * m_iconSpace.cy);
	pDC->LineTo (4 + x * m_iconSpace.cx+m_iconSize.cx,1 + y * m_iconSpace.cy);
	pDC->LineTo (4 + x * m_iconSpace.cx+m_iconSize.cx,4 + y * m_iconSpace.cy+m_iconSize.cy);
	pDC->LineTo (1 + x * m_iconSpace.cx, 4 + y * m_iconSpace.cy+m_iconSize.cy);
	pDC->LineTo (1 + x * m_iconSpace.cx, 1 + y * m_iconSpace.cy);

// draw black boxes around and inside this box
//	  SelectObject(hdc, GetStockObject(BLACK_PEN));
	pDC->MoveTo (0 + x * m_iconSpace.cx, 0 + y * m_iconSpace.cy);
	pDC->LineTo (5 + x * m_iconSpace.cx+m_iconSize.cx,0 + y * m_iconSpace.cy);
	pDC->LineTo (5 + x * m_iconSpace.cx+m_iconSize.cx,5 + y * m_iconSpace.cy+m_iconSize.cy);
	pDC->LineTo (0 + x * m_iconSpace.cx, 5 + y * m_iconSpace.cy+m_iconSize.cy);
	pDC->LineTo (0 + x * m_iconSpace.cx, 0 + y * m_iconSpace.cy);
	pDC->MoveTo (2 + x * m_iconSpace.cx, 2 + y * m_iconSpace.cy);
	pDC->LineTo (3 + x * m_iconSpace.cx+m_iconSize.cx,2 + y * m_iconSpace.cy);
	pDC->LineTo (3 + x * m_iconSpace.cx+m_iconSize.cx,3 + y * m_iconSpace.cy+m_iconSize.cy);
	pDC->LineTo (2 + x * m_iconSpace.cx, 3 + y * m_iconSpace.cy+m_iconSize.cy);
	pDC->LineTo (2 + x * m_iconSpace.cx, 2 + y * m_iconSpace.cy);
	if (++x >= m_viewSpace.cx) {
		x = 0;
		if (++y >= m_viewSpace.cy)
			break;
			}
		} 
pDC->SelectPalette(oldPalette, FALSE);
ReleaseDC(pDC);
}

								/*---------------------------*/

void CTextureFilter::Setup (void) 
{
  int nTextures, nFrames = 0;

  nTextures = textureManager.MaxTextures ();

  // calculate total number of textures
m_nTextures [1] = 0;
for (int i = 0; i < nTextures; i++) {
	if (textureManager.Texture (i)->m_info.bFrame)
		++nFrames;
	else
		m_mapViewToTex [m_nTextures [1]++] = i;
	}

// allocate memory for texture list
CreateTexMap ();
}

								/*---------------------------*/

afx_msg BOOL CTextureFilter::OnEraseBkgnd (CDC* pDC)
{
if (!theMine) 
	return FALSE;

   CRect    rc;
   CBrush   bkGnd, * pOldBrush;
   CPoint   pt (0,0);

ClientToScreen (&pt);
bkGnd.CreateStockObject (BLACK_BRUSH);
bkGnd.UnrealizeObject ();
pDC->SetBrushOrg (pt);
pOldBrush = pDC->SelectObject (&bkGnd);
GetClientRect (rc);
pDC->FillRect (&rc, &bkGnd);
pDC->SelectObject (pOldBrush);
bkGnd.DeleteObject ();
return 1;
}

								/*---------------------------*/

BOOL CTextureFilter::OnMouseWheel (UINT nFlags, short zDelta, CPoint pt)
{
	CRect	rc;

GetWindowRect (rc);
if ((pt.x < rc.left) || (pt.x >= rc.right) || (pt.y < rc.top) || (pt.y >= rc.bottom))
	return 1;

int nPos = GetScrollPos (SB_VERT) - zDelta / WHEEL_DELTA;

if (nPos >= m_nRows [m_bShowAll])
	nPos = m_nRows [m_bShowAll] - m_viewSpace.cy;
if (nPos < 0)
	nPos = 0;
SetScrollPos (SB_VERT, nPos, TRUE);
Refresh ();
return 0;
}

								/*---------------------------*/

afx_msg void CTextureFilter::OnPaint ()
{
	CRect	rc;
	CDC	*pDC;
	PAINTSTRUCT	ps;

if (!GetUpdateRect (rc))
	return;
pDC = BeginPaint (&ps);
RecalcLayout ();
EndPaint (&ps);
}
								/*---------------------------*/
