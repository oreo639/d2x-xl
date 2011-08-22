#ifndef __blockman_h
#define __blockman_h

//------------------------------------------------------------------------------

class CBlockManager {
	private:
		char m_filename [256];
		CSegment* m_oldSegments;
		CSegment* m_newSegments;
		short	m_xlatSegNum [SEGMENT_LIMIT];

	public:
		void Cut (void);
		
		void Copy (char *filename = null, bool bDelete = false);
		
		void Paste (void);
		
		int Read (char *filename);
		
		void QuickPaste (void);
		
		void Delete (void);

	private:
		void SetupTransformation (CDoubleMatrix& m, CDoubleVector& o);
		
		short Read (CFileManager& fp);
		
		void Write (CFileManager& fp);

		bool CheckTunnelMaker (void);
};

extern CBlockManager blockManager;

#endif //__blockman_h

//------------------------------------------------------------------------------
//eof blockmanager.h