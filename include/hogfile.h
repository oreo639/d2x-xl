/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifndef _HOGFILE_H
#define _HOGFILE_H

#include <stdio.h>
#include <sys/types.h>

#include "maths.h"
#include "vecmat.h"

// the maximum length of a filename
#define FILENAME_LEN			255
#define SHORT_FILENAME_LEN 13
#define MAX_HOGFILES			300

typedef struct tHogFile {
	char		name [256];
	int32_t		offset;
	int32_t		length;
} tHogFile;

typedef struct tHogFileList {
	tHogFile		files [MAX_HOGFILES];
	char			szName [FILENAME_LEN];
	char			szFolder [FILENAME_LEN];
	int32_t			nFiles;
	int32_t			bInitialized;
} tHogFileList;

typedef struct tGameHogFiles {
	tHogFileList D1HogFiles;
	tHogFileList D2HogFiles;
	tHogFileList D2XHogFiles;
	tHogFileList XLHogFiles;
	tHogFileList ExtraHogFiles;
	tHogFileList MsnHogFiles;
	char szAltHogFile [FILENAME_LEN];
	int32_t bAltHogFileInited;
} tGameHogFiles;

class CHogFile {
	public:
		tGameHogFiles	m_files;

	public:
		CHogFile () {};
		~CHogFile () {};
		int32_t Init (const char *pszHogName, const char *pszFolder);

		int32_t UseXL (const char *name);
		int32_t UseD2X (const char *name);
		int32_t UseExtra (const char *name);
		int32_t UseMission (const char *name);
		int32_t ReloadMission (const char * name = NULL);
		int32_t UseD1 (const char *name);
		void UseAltDir (const char *path);
		FILE* Find (const char *name, int32_t *length, int32_t bUseD1Hog);
		FILE *Find (tHogFileList *hogP, const char *folder, const char *name, int32_t *length);

		inline tHogFileList& D1Files (void) { return m_files.D1HogFiles; }
		inline tHogFileList& D2Files (void) { return m_files.D2HogFiles; }
		inline tHogFileList& D2XFiles (void) { return m_files.D2XHogFiles; }
		inline tHogFileList& XLFiles (void) { return m_files.XLHogFiles; }
		inline tHogFileList& ExtraFiles (void) { return m_files.ExtraHogFiles; }
		inline tHogFileList& AltFiles (void) { return m_files.MsnHogFiles; }
		char *AltHogFile (void) { return m_files.szAltHogFile; }
		inline char *MissionName (void) { return m_files.MsnHogFiles.szName; }

	private:
		void QuickSort (tHogFile *hogFiles, int32_t left, int32_t right);
		tHogFile *BinSearch (tHogFile *hogFiles, int32_t nFiles, const char *pszFile);
		int32_t Use (tHogFileList *hogP, const char *name, const char *folder);
		int32_t Reload (tHogFileList *hogP);
		int32_t Setup (const char *pszFile, const char *folder, tHogFile *hogFiles, int32_t *nFiles);
};

extern CHogFile hogFileManager;

#endif
