// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <malloc.h>
#include <stdlib.h>
#undef abs
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dle-xp.h"
#include "mine.h"
#include "global.h"
#include "cfile.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

short CTriggerTarget::Add (CSideKey key) 
{
if (m_count < MAX_TRIGGER_TARGETS)
	m_targets [m_count] = key;
return m_count++;
}

//------------------------------------------------------------------------------

short CTriggerTarget::Delete (short i = -1) 
{
if (i < 0)
	i = m_count - 1;
if ((m_count > 0) && (i < --m_count)) {
	int l = m_count - i;
	if (l)
		memcpy (m_targets + i, m_targets + i + 1, l * sizeof (m_targets [0]));
	m_targets [m_count] = CSideKey (-1,-1);
	}
return m_count;
}	

//------------------------------------------------------------------------------

int CTriggerTarget::Delete (CSideKey key) 
{ 
short i = Find (key);
if (i >= 0)
	Delete (i);
}

//------------------------------------------------------------------------------

int CTriggerTarget::Find (CSideKey key) 
{ 
for (int i = 0; i < m_count; i++)
	if (m_targets [i] == key)
		return i;
return -1;
}

//------------------------------------------------------------------------------

void Clear (void) 
{ 
m_count = 0;
for (int i = 0; i < MAX_TRIGGER_TARGETS; i++)
	m_targets [i].Clear ();
}

//------------------------------------------------------------------------------

void Read (CFileManager& fp) 
{
	int i;

for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	m_targets [i].m_nSegment = fp.ReadInt16 ();
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	m_targets [i].m_nSide = fp.ReadInt16 ();
return 1;
}

//------------------------------------------------------------------------------

void Write (CFileManager& fp) 
{
	int i;

for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	fp.Write (m_targets [i].m_nSegment);
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	fp.Write (m_targets [i].m_nSide);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CTrigger::Setup (short type, short flags)
{
m_info.type = (char) type;
m_info.flags = (char) flags;
if (type == TT_SPEEDBOOST)
	m_info.value = 10;
else if ((type == TT_CHANGE_TEXTURE) || (type == TT_MASTER))
	m_info.value = 0;
else if ((type == TT_MESSAGE) || (type == TT_SOUND))
	m_info.value = 1;
else 	
	m_info.value = 5 * F1_0; // 5% shield or energy damage
m_info.time = -1;
m_count = 0;
for (int i = 0; i < MAX_TRIGGER_TARGETS; i++)
	m_targets [i].Clear ();
}

//------------------------------------------------------------------------------

int CTrigger::Read (CFileManager& fp, int version, bool bObjTrigger)
{
if (DLE.IsD2File ()) {
	m_info.type = fp.ReadByte ();
	m_info.flags = bObjTrigger ? fp.ReadInt16 () : (ushort) fp.ReadByte ();
	m_count = fp.ReadByte ();
	fp.ReadByte ();
	m_info.value = fp.ReadInt32 ();
	if ((DLE.LevelVersion () < 21) && (m_info.type == TT_EXIT))
		m_info.value = 0;
	if ((version < 39) && (m_info.type == TT_MASTER))
		m_info.value = 0;
	m_info.time = fp.ReadInt32 ();
	}
else {
	m_info.type = fp.ReadByte ();
	m_info.flags = fp.ReadInt16 ();
	m_info.value = fp.ReadInt32 ();
	m_info.time = fp.ReadInt32 ();
	fp.ReadByte (); //skip 8 bit value "link_num"
	m_count = char (fp.ReadInt16 ());
	if (m_count < 0)
		m_count = 0;
	else if (m_count > MAX_TRIGGER_TARGETS)
		m_count = MAX_TRIGGER_TARGETS;
	}
this->CTriggerTargets::Read (fp);
return 1;
}

//------------------------------------------------------------------------------

void CTrigger::Write (CFileManager& fp, int version, bool bObjTrigger)
{
if (DLE.IsD2File ()) {
	fp.Write (m_info.type);
	if (bObjTrigger)
		fp.Write (m_info.flags);
	else
		fp.WriteSByte ((sbyte) m_info.flags);
	fp.WriteSByte ((sbyte) m_count);
	fp.WriteByte (0);
	fp.Write (m_info.value);
	fp.Write (m_info.time);
	}
else {
	fp.Write (m_info.type);
	fp.Write (m_info.flags);
	fp.Write (m_info.value);
	fp.Write (m_info.time);
	fp.WriteSByte ((sbyte) m_count);
	fp.Write (m_count);
	}
this->CTriggerTargets::Write (fp);
}

//------------------------------------------------------------------------------

bool CTrigger::IsExit (void)
{
return DLE.IsD1File () 
		 ? m_info.flags & (TRIGGER_EXIT | TRIGGER_SECRET_EXIT) 
		 : m_info.type == TT_EXIT || trigP->m_info.type == TT_SECRET_EXIT);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

int CReactorTrigger::Read (CFileManager& fp, int version, bool bFlag)
{
	int	i;

m_count = char (fp.ReadInt16 ());
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	m_targets [i].m_nSegment = fp.ReadInt16 ();
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	m_targets [i].m_nSide = fp.ReadInt16 ();
return 1;
}

//------------------------------------------------------------------------------

void CReactorTrigger::Write (CFileManager& fp, int version, bool bFlag)
{
	int	i;

fp.Write (m_count);
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	fp.Write (m_targets [i].m_nSegment);
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	fp.Write (m_targets [i].m_nSide);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------
//eof trigger.cpp