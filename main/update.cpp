#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#if defined(__unix__) || defined(__macosx__)
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#ifdef __macosx__
#	include "SDL/SDL_main.h"
#	include <SDL/SDL_thread.h>
#	include "SDL/SDL_keyboard.h"
#	include "FolderDetector.h"
#else
#	include "SDL_main.h"
#	include "SDL_thread.h"
#	include "SDL_keyboard.h"
#endif
#include "descent.h"
#include "text.h"
#include "menu.h"

// ----------------------------------------------------------------------------

#if defined(__unix__)

#	define FILEEXT		"rar"
#	define FILETYPE	"src"

#include <curl/curl.h>
#include <cstdio>

// link with libcurl (-lcurl)

int DownloadFile (const char* pszSrc, const char* pszDest)
{
CURL* hCurl;
if (!(hCurl = curl_easy_init ()))
	return 1;
if (curl_easy_setopt (hCurl, CURLOPT_URL, pszSrc)) {
	curl_easy_cleanup (hCurl);
	return 1;
	}
std::FILE* file;
if (!(file = std::fopen (pszDest, "w"))) {
	curl_easy_cleanup (hCurl);
	return 1;
	}
#if DBG
curl_easy_setopt (hCurl, CURLOPT_VERBOSE, 1);
#endif
if (curl_easy_setopt (hCurl, CURLOPT_WRITEDATA, file)) {
	curl_easy_cleanup (hCurl);
	return 1;
	}
if (curl_easy_perform (hCurl)) {
	curl_easy_cleanup (hCurl);
	std::fclose (file);
	unlink (pszDest);
	return 1;
	}
curl_easy_cleanup (hCurl);
std::fclose (file);
return 0;
}

// ----------------------------------------------------------------------------

#elif defined(_WIN32)

#	include "urlmon.h"
#	include <process.h>
#	include "errno.h"

//#	define DownloadFile(_src,_dest)	URLDownloadToFile (NULL, _src, _dest, NULL, NULL)

#	define FILEEXT		"exe"
#	define FILETYPE	"win"

class CDownload : public IBindStatusCallback {
	private:
		int			m_nResult;
		int			m_nStatus;
		int			m_nProgress;
		int			m_nProgressMax;
		SDL_Thread*	m_thread;
		const char* m_pszSrc;
		const char* m_pszDest;
		int			m_nOptPercentage;
		int			m_nOptProgress;
		CMenu	m_menu;

		static CDownload* m_handler;

	private:
		void Setup (const char* pszSrc, const char* pszDest) {
			m_nStatus = -1;
			m_nResult = 0;
			m_pszSrc = pszSrc;
			m_pszDest = pszDest;
			m_menu.AddText ("", 0);
			char szProgress [50];
			sprintf (szProgress, "0%c done", '%');
			m_nOptPercentage = m_menu.AddText (szProgress, 0);
			m_menu [m_nOptPercentage].m_x = (short) 0x8000;	//centered
			m_menu [m_nOptPercentage].m_bCentered = 1;
			m_nOptProgress = m_menu.AddGauge ("                    ", -1, 100);
			}

	public:
		static CDownload* Handler (void) {
			if (!m_handler)
				m_handler = new CDownload();
			return m_handler;
			}

		int Update (void) {
			Start ();
			if (!(m_nProgress && m_nProgressMax))
				return 1;
			int h = int (m_nProgress * 100 / m_nProgressMax);
			if (h == m_nProgress)
				return 1;
			if (h >= 100)
				return 0;
			m_nProgress = h;
			sprintf (m_menu [m_nOptPercentage].m_text, TXT_PROGRESS, m_nProgress, '%');
			m_menu [m_nOptPercentage].m_bRebuild = 1;
			h = m_nProgress;
			if (m_menu [m_nOptProgress].m_value != h) {
				m_menu [m_nOptProgress].m_value = h;
				m_menu [m_nOptProgress].m_bRebuild = 1;
				}
			return 1;
			}


		virtual HRESULT STDMETHODCALLTYPE OnProgress (ULONG ulProgress, ULONG ulProgressMax, ULONG ulResultCode, LPCWSTR szResultText) {
			m_nProgress = int (ulProgress);
			m_nProgressMax = int (ulProgressMax);
			return S_OK;
			}

		virtual ULONG STDMETHODCALLTYPE AddRef (void) { return E_NOTIMPL; } 

		virtual ULONG STDMETHODCALLTYPE Release (void) { return E_NOTIMPL; } 

		virtual HRESULT STDMETHODCALLTYPE QueryInterface (const IID &,void **) { return E_NOTIMPL; } 

		virtual HRESULT STDMETHODCALLTYPE OnStartBinding (DWORD dwReserved, __RPC__in_opt IBinding *pib) { return E_NOTIMPL; } 

		virtual HRESULT STDMETHODCALLTYPE GetPriority (__RPC__out LONG *pnPriority) { return E_NOTIMPL; } 

		virtual HRESULT STDMETHODCALLTYPE OnLowResource (DWORD reserved) { return E_NOTIMPL; } 

		virtual HRESULT STDMETHODCALLTYPE OnStopBinding (HRESULT hresult,__RPC__in_opt LPCWSTR szError) { return E_NOTIMPL; } 

		virtual HRESULT STDMETHODCALLTYPE GetBindInfo (DWORD *grfBINDF, BINDINFO *pbindinfo) { return E_NOTIMPL; } 

		virtual HRESULT STDMETHODCALLTYPE OnDataAvailable (DWORD grfBSCF, DWORD dwSize, FORMATETC *pformatetc, STGMEDIUM *pstgmed) { return E_NOTIMPL; } 

		virtual HRESULT STDMETHODCALLTYPE OnObjectAvailable (__RPC__in REFIID riid, __RPC__in_opt IUnknown *punk) { return E_NOTIMPL; } 

	protected:
		CDownload () : m_nStatus (-1), m_nResult (0), m_nProgress (0), m_nProgressMax (0), m_thread (NULL) {}
		CDownload (CDownload const&) {}
		CDownload& operator= (CDownload const&) {}

	public:
		static int MenuPoll (CMenu& menu, int& key, int nCurItem, int nState) {
			if (!nState) {
				CDownload::Handler ()->Update ();
				key = (CDownload::Handler ()->Status () == 1) ? -2 : 0;
				}
			return nCurItem;
			}

		static int _CDECL_ Download (void* downloadHandler) {
			return CDownload::Handler ()->Fetch ();
			}

		int Execute (const char* pszSrc, const char* pszDest) {
			Setup (pszSrc, pszDest);
			for (; m_menu.Menu (NULL, "Downloading...", &CDownload::MenuPoll) >= 0; )
				;
			m_thread = NULL;
			Destroy ();
			return Result ();
			}

		void Start (void) {
			if (m_nStatus < 0) {
				m_nStatus = 0;
				m_thread = SDL_CreateThread (&CDownload::Download, NULL);
				}
			}

		int Fetch (void) {
			m_nResult = URLDownloadToFile (NULL, m_pszSrc, m_pszDest, NULL, Handler ());
			m_nStatus = 1;
			return m_nResult;
			}

		inline int Status (void) { return m_nStatus; }

		inline int Result (void) { return m_nResult; }
	};


CDownload* CDownload::m_handler = NULL;


int DownloadFile (const char* pszSrc, const char* pszDest)
{
return CDownload::Handler ()->Execute (pszSrc, pszDest);
}

#endif

#if defined(_WIN32) || defined(__unix__)

int CheckForUpdate (void)
{
	char		szSrc [FILENAME_LEN], szDest [FILENAME_LEN];
	CFile		cf;
	int		nVersion [3], nLocation;
	char		szMsg [1000];

	static const char* pszSource [2] = {
		"http://www.descent2.de/files", 
#if defined(_WIN32)
		"http://sourceforge.net/projects/d2x-xl/files/Windows"
#else
		"http://sourceforge.net/projects/d2x-xl/files/Linux"
#endif
	};

sprintf (szDest, "%s/d2x-xl-version.txt", gameFolders.szDownloadDir);
if (!DownloadFile ("http://www.descent2.de/files/d2x-xl-version.txt", szDest))
	nLocation = 0;
else if (!DownloadFile ("http://sourceforge.net/projects/d2x-xl/files/d2x-xl-version.txt/download", szDest))
	nLocation = 1;
else {
	MsgBox (TXT_ERROR, NULL, 1, TXT_CLOSE, "Download failed.");
	return -1;
	}
if (!cf.Open ("d2x-xl-version.txt", gameFolders.szDownloadDir, "rb", -1)) {
	MsgBox (TXT_ERROR, NULL, 1, TXT_CLOSE, "Download failed.");
	return -1;
	}
if (3 != fscanf (cf.File (), "%d.%d.%d", &nVersion [0], &nVersion [1], &nVersion [2])) {
	MsgBox (TXT_ERROR, NULL, 1, TXT_CLOSE, "Download failed.");
	return -1;
	}

#if !DBG
if (D2X_IVER >= nVersion [0] * 100000 + nVersion [1] * 1000 + nVersion [2]) {
	MsgBox (NULL, NULL, 1, TXT_CLOSE, "No updates were found.");
	return 0;
	}
#endif

if (MsgBox (NULL, NULL, 2, TXT_YES, TXT_NO, "An update has been found.\nDownload it?"))
	return 0;
sprintf (szDest, "%s/d2x-xl-%s-%d.%d.%d.%s", gameFolders.szDownloadDir,
			FILETYPE, nVersion [0], nVersion [1], nVersion [2], FILEEXT);
#if 1
messageBox.Show ("Downloading...");
sprintf (szSrc, "%s/d2x-xl-%s-%d.%d.%d.%s", pszSource [nLocation], FILETYPE, nVersion [0], nVersion [1], nVersion [2], FILEEXT);
if (DownloadFile (szSrc, szDest)) {
	messageBox.Clear ();
	MsgBox (TXT_ERROR, NULL, 1, TXT_CLOSE, "Download failed.");
	return -1;
	}
messageBox.Clear ();
if (!cf.Exist (szDest, "", 0)) {
	MsgBox (TXT_ERROR, NULL, 1, TXT_CLOSE, "Download failed.");
	return -1;
	}
#endif
#if defined(__unix__)
sprintf (szMsg, "\nThe file\n\n%s\n\nwas sucessfully downloaded.", szDest);
MsgBox (NULL, NULL, 1, TXT_CLOSE, szMsg);
#else
#	if 1
#	include "shellapi.h"
#	include "objbase.h"
#if !defined(_M_IA64) && !defined(_M_AMD64) && !defined(_OPENMP) && !defined(__INTEL_COMPILER)
CoInitializeEx (NULL, COINIT_MULTITHREADED);
#	else
CoInitialize (NULL);
#	endif
if (HINSTANCE (32) < ShellExecute (NULL, NULL, szDest, NULL, NULL, SW_SHOW))
	exit (1);
#else
char*	args [2] = {szDest, NULL};
if (0 <= _execv (szDest, args))
	exit (1);
#endif
sprintf (szMsg, "\nThe file\n\n%s\n\nwas sucessfully downloaded, but couldn't be excuted.\nPlease leave D2X-XL and start the installer manually.", szDest);
//Warning (szMsg);
MsgBox (TXT_ERROR, NULL, 1, TXT_CLOSE, szMsg);
#endif
return -1;
}

#endif

// ----------------------------------------------------------------------------
