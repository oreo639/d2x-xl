#ifndef __AVLTree_H
#define __AVLTree_H

#if _MSC
#  pragma pack(push,1)
#endif


typedef tAvlWalkFunc *  pAvlWalkFunc;
typedef tAvlCmpFunc *   pAvlCmpFunc;

#define PAVLNODE        struct tAvlNode *

typedef PACKED struct tAvlNode
   BEGIN
   PAVLNODE    left;
   PAVLNODE    right;
   void *     pData;
   tBitField   balance  :2;
   tBitField   userData :1;
   END tAvlNode;

typedef tAvlNode *   pAvlNode;
typedef pAvlNode *   ppAvlNode;

typedef PACKED struct tAvl
   BEGIN
   pAvlNode       root;
   pAvlNode       current;
   pAvlCmpFunc    cmpFunc;
   pAvlWalkFunc   walkFunc;
   pAllocFunc     allocFunc;
   pFreeFunc      freeFunc;
   pAllocFunc     allocDataFunc;
   pFreeFunc      freeDataFunc;
	tLStack			avlStack;
	tLStack			dataStack;
   LPSTR          key;
   INT4           dataSize;
   BOOLEAN        dupEntry;
   BOOLEAN        branchChanged;
	BOOLEAN			bUseStack;
	INT2				avlRes;
	tSemaphore		sem;
   END tAvl;

typedef tAvl *       pAvl;

#if _MSC
#  pragma pack(pop)
#endif

template class< _T > class CAVLTree (_T) {
	};

INT2 WINEXP AvlInit (pAvl a, 
							pAllocFunc allocFunc, pFreeFunc freeFunc, 
							pAllocFunc allocDataFunc, pFreeFunc freeDataFunc,
							pAvlCmpFunc cmpFunc, pAvlWalkFunc walkFunc,
							INT4 nDataSize, BOOLEAN bUseStack);

INT2 LIBEXP AvlInsert (pAvl a, void * * data, INT4 dataSize, void * key);
  /* Knoten in AVL-Baum einfuegen */

INT2 LIBEXP AvlDelete (pAvl a, void * key, PPTR ppData);
  /* Knoten aus AVL-Baum loeschen */

void LIBEXP AvlFreeAll (pAvl a);
  /* gesamten AVL-Baum loeschen */

void LIBEXP AvlFreeData (pAvl a);
  /* Datenbloecke des gesamten AVL-Baums loeschen */

void * LIBEXP AvlFind (pAvl a, void * key);
  /* Eintrag in AVL-Baum suchen */

BOOLEAN LIBEXP AvlWalk (pAvl a, pAvlWalkFunc walkFunc, void * pUserData);
  /* rekursiv durch AVL-Baum gehen; bei jedem Knoten wird walkHandler */
  /* aufgerufen */

#ifdef __cplusplus
END

class CAvl {
	private:
		tAvl	m_a;
	public:
		CAvl ()
			{ memset (&m_a, 0, sizeof (m_a)); }
		~CAvl () 
			{ Destroy (); }
		inline INT2 Create (pAvlCmpFunc cmpFunc, INT4 nDataSize) 
			{ return ::AvlInit (&m_a, NULL, NULL, NULL, NULL, cmpFunc, NULL, nDataSize, TRUE); }
		INT2 Insert (void *pKey, void *pData, INT4 nDataSize = -1) {
			void	*pAvlData = NULL;
			INT2	nAvlRes = ::AvlInsert (&m_a, &pAvlData, nDataSize, pKey);
			if (0 < nAvlRes && pData && (nDataSize > 0))
				memcpy (pAvlData, pData, nDataSize);
			return nAvlRes;
			}
		inline INT2 Delete (void *pKey, void **ppData)
			{ return ::AvlDelete (&m_a, pKey, ppData); }
		inline void *Find (void *pKey)
			{ return ::AvlFind (&m_a, pKey); }
		inline BOOL Walk (pAvlWalkFunc walkFunc, void *pUserData)
			{ return ::AvlWalk (&m_a, walkFunc, pUserData); }
		inline void FreeData (void)
			{ ::AvlFreeData (&m_a); }
		inline void Destroy (void) {
			::AvlFreeAll (&m_a); 
			memset (&m_a, 0, sizeof (m_a));
			}
	};
#endif

#else

extern INT2 AvlAlloc ();
extern void AvlFree ();
extern void AvlFreeAll ();
extern void AvlFreeData ();
extern void * AvlFind ();
extern BOOLEAN AvlWalk ();

#endif

extern INT2       avlDepth;
extern pAllocFunc avlAlloc;
extern pFreeFunc  avlFree;

#endif /*__AVL_H*/

/*eof*/
