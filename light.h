#ifndef __light_h
#define __light_h

#include "stdafx.h"
#include "windows.h"
// Copyright (C) 1997 Bryan Aamot
//==========================================================================
// CLASS -- CLightDialog
//==========================================================================
class CLightDialog : public CDialog {
//	CCheckBox *AutoCheck,*SetAllCheck,*AverageCheck,*ScaleCheck,*DeltaCheck;
public:
	CEdit *SetAllEdit,*ScaleEdit,*LightFactorEdit,*DeltaEdit;
	CButton *OkButton,*ApplyButton, *LightsOffButton;
	double LightingFactor;

	CLightDialog(CWnd * AParent, LPSTR name);
	virtual void SetupWindow();
	void Ok();
	virtual bool CanClose();
	void auto_adjust_light();
	void average_corner_light();
	void scale_corner_light(double fvalue);
	void Apply();
	void ShowDelta();
	void set_illumination(INT16 nSegment, INT16 nSide, UINT32 brightness);
//	DECLARE_RESPONSE_TABLE( CLightDialog );
};

//==========================================================================
// CLASS -- CBlinkDialog
//==========================================================================
class CBlinkDialog : public CDialog {
public:
//  CRadioButton *LightButton[32];
  CScrollBar *CycleTimeBar;
  CStatic    *CycleTimeText;
//  CRadioButton *OptionRadio[4];
//  CGroupBox *LightGroup;
  CStatic *BlackBox;
  CEdit *PatternEdit;
  UINT32 test;
  INT32 current_highlight;

  CBlinkDialog(CWnd * AParent, LPSTR name);
  virtual void SetupWindow();
  afx_msg void OnPaint();
  afx_msg void OnTimer( UINT_PTR );
  void cycle_time_bar_msg(UINT);
  void blink_radio_button_msg();
  void light_button_msg1();
  void light_button_msg2();
  void light_button_msg3();
  void light_button_msg4();
  void light_button_msg5();
  void light_button_msg6();
  void light_button_msg7();
  void light_button_msg8();
  void light_button_msg9();
  void light_button_msg10();
  void light_button_msg11();
  void light_button_msg12();
  void light_button_msg13();
  void light_button_msg14();
  void light_button_msg15();
  void light_button_msg16();
  void light_button_msg17();
  void light_button_msg18();
  void light_button_msg19();
  void light_button_msg20();
  void light_button_msg21();
  void light_button_msg22();
  void light_button_msg23();
  void light_button_msg24();
  void light_button_msg25();
  void light_button_msg26();
  void light_button_msg27();
  void light_button_msg28();
  void light_button_msg29();
  void light_button_msg30();
  void light_button_msg31();
  void light_button_msg32();
  void handle_light_button(INT32);
  void add_msg();
  void delete_msg();
  void RefreshData();
  void PatternMsg();
  LRESULT User( WPARAM, LPARAM );
//DECLARE_RESPONSE_TABLE( CBlinkDialog );
};

void delete_flickering_light(UINT16 nSegment, UINT16 nSide);
void add_flickering_light(UINT16 nSegment, UINT16 nSide, UINT32 mask,FIX time);
INT16 get_flickering_light(UINT16 nSegment, UINT16 nSide);

void set_segment_child_num(INT16 nSegment,INT16 recursion_level);
UINT8 light_weight(INT16 nBaseTex);
void calculate_CLightDeltaValue_data(double factor, INT32 force); // light.cpp
void update_CLightDeltaValues(); // light.cpp
INT32 is_light(INT32 value);
INT32 is_exploding_light(INT32 value);
void CreateLightMap (void);
INT32 ReadLightMap (FILE *fLightMap, UINT32 nSize);
INT32 WriteLightMap (FILE *fLightMap);
INT32 WriteColorMap (FILE *fColorMap);
BOOL HasCustomLightMap (void);

#endif