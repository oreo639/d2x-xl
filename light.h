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
	void set_illumination(short nSegment, short nSide, uint brightness);
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
  uint test;
  int current_highlight;

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
  void handle_light_button(int);
  void add_msg();
  void delete_msg();
  void RefreshData();
  void PatternMsg();
  LRESULT User( WPARAM, LPARAM );
//DECLARE_RESPONSE_TABLE( CBlinkDialog );
};

void delete_flickering_light(ushort nSegment, ushort nSide);
void add_flickering_light(ushort nSegment, ushort nSide, uint mask,int time);
short get_flickering_light(ushort nSegment, ushort nSide);

void set_segment_child_num(short nSegment,short recursion_level);
byte light_weight(short nBaseTex);
void calculate_CLightDeltaValue_data(double factor, int force); // light.cpp
void update_CLightDeltaValues(); // light.cpp
int is_light(int value);
int is_exploding_light(int value);
void CreateLightMap (void);
int ReadLightMap (CFileManager& fLightMap, uint nSize);
int WriteLightMap (CFileManager& fLightMap);
int WriteColorMap (CFileManager& fColorMap);
BOOL HasCustomLightMap (void);

#endif