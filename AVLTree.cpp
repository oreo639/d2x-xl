
#include "AVLTree.h"

short avlDepth = 0;

#define  AVL_OVERFLOW   1
#define  AVL_BALANCED   0
#define  AVL_UNDERFLOW  3

// -----------------------------------------------------------------------------

_T* CAVLTree::Find (_K key)
{
   CAVL::CNode<_T>* p;

for (p = m_root; p; ) {
	if (
   {
   switch (a->cmpFunc (root->pData, key))
      {
      case 1:
         root = root->left;
         break;
      case -1:
         root = root->right;
         break;
      default:
         a->current = root;
         return root->pData;
      }
   }
return NULL;
} /*AVLFind*/

// -----------------------------------------------------------------------------

short CAVL::AllocNode (pCAVL:: a, pCAVLNode* root)
{
	void *	pData = NULL;
	INT4		h;

ENTER ("CAVL::AllocNode");
if ((h = a->dataSize) && 
    !(pData = a->bUseStack ? InsStack (&(a->dataStack), NULL) : AllocFunc (a->allocDataFunc, h)))
	LEAVE1 (0, a->avlRes = E_PTV_MEMORY);

a->current = (CAVLNode*) (a->bUseStack ? InsStack (&(a->avlStack), NULL) : 
													 AllocFunc (a->allocFunc, sizeof (tCAVL::Node)));
if (!a->current)
	{
	if (pData && a->dataSize)
		a->freeFunc (&(pData));
	LEAVE1 (0, a->avlRes = E_PTV_MEMORY);
	}
MemSet (a->current, 0, sizeof (tCAVL::Node));
a->current->pData = pData;
*root = a->current;
a->branchChanged = TRUE;
LEAVE1 (0, a->avlRes = E_PTV_OK);
}

// -----------------------------------------------------------------------------

static short CAVL::DoInsert
#ifdef _ANSIC
   (pCAVL:: a, pCAVLNode* root)
#else
   (a, root, pData)
   pCAVL::           a;
   pCAVLNode*      root;
#endif
{
   CAVLNode* p1, p2, r;
   INT1     b;

ENTER ("DoCAVL::Insert");
if (!(r = *root))
	{
	CAVL::AllocNode (a, root);
	LEAVE1 (0, a->avlRes);
	}

switch (a->cmpFunc (r->pData, a->key))
   {
   case 1:
      if (DoCAVL::Insert (a, &(r->left)))
			LEAVE1 (0, a->avlRes);
      if (a->branchChanged)
         {
         switch (r->balance)
            {
            case AVL_UNDERFLOW:
               p1 = r->left;
               if (p1->balance == AVL_UNDERFLOW)   /*einfache LL-Rotation*/
                  {
                  r->left = p1->right;
                  p1->right = r;
                  r->balance = AVL_BALANCED;
                  r = p1;
                  }
               else
                  { /*doppelte LR-Rotation*/
                  p2 = p1->right;
                  p1->right = p2->left;
                  p2->left = p1;
                  r->left = p2->right;
                  p2->right = r;
                  b = (INT1) p2->balance;
                  r->balance = (b == AVL_UNDERFLOW) ? AVL_OVERFLOW : AVL_BALANCED;
                  p1->balance = (b == AVL_OVERFLOW) ? AVL_UNDERFLOW : AVL_BALANCED;
                  r = p2;
                  }
               r->balance = AVL_BALANCED;
               a->branchChanged = FALSE;
               break;
   
            case AVL_BALANCED:
               r->balance = AVL_UNDERFLOW;
               break;
   
            case AVL_OVERFLOW:
               r->balance = AVL_BALANCED;
               a->branchChanged = FALSE;
               break;
            }
         }
      break;

   case 0:
      a->dupEntry = TRUE;
      a->current = r;
      break;

   case -1:
      if (DoCAVL::Insert (a, &(r->right)))
         LEAVE1 (0, a->avlRes);
      if (a->branchChanged)
         {
         switch (r->balance)
            {
            case AVL_UNDERFLOW:
               r->balance = AVL_BALANCED;
               a->branchChanged = FALSE;
               break;
   
            case AVL_BALANCED:
               r->balance = AVL_OVERFLOW;
               break;
   
            case AVL_OVERFLOW:
               p1 = r->right;
               if (p1->balance == AVL_OVERFLOW) /*einfache RR-Rotation*/
                  {
                  r->right = p1->left;
                  p1->left = r;
                  r->balance = AVL_BALANCED;
                  r = p1;
                  }
               else
                  { /*doppelte RL-Rotation*/
                  p2 = p1->left;
                  p1->left = p2->right;
                  p2->right = p1;
                  r->right = p2->left;
                  p2->left = r;
                  b = (INT1) p2->balance;
                  r->balance = (b == AVL_OVERFLOW) ? AVL_UNDERFLOW : AVL_BALANCED;
                  p1->balance = (b == AVL_UNDERFLOW) ? AVL_OVERFLOW : AVL_BALANCED;
                  r = p2;
                  }
               r->balance = AVL_BALANCED;
               a->branchChanged = FALSE;
            }
         }
      break;
   }
*root = r;
LEAVE1 (0, a->avlRes = E_PTV_OK);
} /*DoCAVL::Insert*/

// -----------------------------------------------------------------------------

short LIBEXP CAVL::Insert
#ifdef _ANSIC
   (pCAVL:: a, PPTR ppData, INT4 dataSize, void * key)
#else
   (a, pData, dataSize, key, cmpFunc)
   pCAVL::        a;
   void * *   pData;
   INT4        dataSize;
   void *     key;
   pCAVL::CmpFunc cmpFunc;
#endif
{
ENTER ("CAVL::Insert");
if (!a->cmpFunc)
	LEAVE1 (0, a->avlRes = E_PTV_INVARG);
OMPLOCK (a->sem, 1);
a->key = (LPSTR) key;
a->branchChanged = 
a->dupEntry = FALSE;
a->dataSize = dataSize;
if (DoCAVL::Insert (a, &(a->root)))
	{
	OMPUNLOCK;
   LEAVE1 (0, a->avlRes);
	}
if (a->dupEntry)
   {
   *ppData = a->current->pData;
	OMPUNLOCK;
   LEAVE1 (0, a->avlRes = E_PTV_DUPLICATE);
   }
if (dataSize)
   *ppData = a->current->pData;
else
   {
   a->current->pData = *ppData;
   a->current->userData = 1;
   }
OMPUNLOCK;
LEAVE1 (0, a->avlRes = E_PTV_OK);
} /*CAVL::Insert*/

// -----------------------------------------------------------------------------

static void CAVL::BalanceLShrink
#ifdef _ANSIC
   (pCAVLNode* root, pBOOLEAN branchShrinked)
#else
   (root, branchShrinked)
   pCAVLNode*   root;
   pBOOLEAN    branchShrinked;
#endif
{
   CAVLNode*    p1, p2, r;
   INT1        b;

r = *root;
switch (r->balance)
   {
   case AVL_UNDERFLOW:
      r->balance = AVL_BALANCED;
      break;

   case AVL_BALANCED:
      r->balance = AVL_OVERFLOW;
      *branchShrinked = FALSE;
      break;

   case AVL_OVERFLOW:
      p1 = r->right;
      if ((b = (INT1) p1->balance) != AVL_UNDERFLOW) /*einfache RR-Rotation*/
         {
         r->right = p1->left;
         p1->left = r;
         if (b)
            {
            r->balance =
            p1->balance = AVL_BALANCED;
            }
         else
            {
            r->balance = AVL_OVERFLOW;
            p1->balance = AVL_UNDERFLOW;
            *branchShrinked = FALSE;
            }
         r = p1;
         }
      else     /*doppelte RL-Rotation*/
         {
         p2 = p1->left;
         b = (INT1) p2->balance;
         p1->left = p2->right;
         p2->right = p1;
         r->right = p2->left;
         p2->left = r;
         r->balance = (b == AVL_OVERFLOW) ? AVL_UNDERFLOW : AVL_BALANCED;
         p1->balance = (b == AVL_UNDERFLOW) ? AVL_OVERFLOW : AVL_BALANCED;
         r = p2;
         r->balance = AVL_BALANCED;
         }
   }
*root = r;
} /*CAVL::BalanceLShrink*/

// -----------------------------------------------------------------------------

static void CAVL::BalanceRShrink
#ifdef _ANSIC
   (pCAVLNode* root, pBOOLEAN branchShrinked)
#else
   (root, branchShrinked)
   pCAVLNode*   root;
   pBOOLEAN    branchShrinked;
#endif
{
   CAVLNode*    p1, p2, r;
   INT1        b;

r = *root;
switch (r->balance)
   {
   case AVL_OVERFLOW:
      r->balance = AVL_BALANCED;
      break;

   case AVL_BALANCED:
      r->balance = AVL_UNDERFLOW;
      *branchShrinked = FALSE;
      break;

   case AVL_UNDERFLOW:
      p1 = r->left;
      if ((b = (INT1) p1->balance) != AVL_OVERFLOW) /*einfache LL-Rotation*/
         {
         r->left = p1->right;
         p1->right = r;
         if (b)
            {
            r->balance = 
            p1->balance = AVL_BALANCED;
            }
         else
            {
            r->balance = AVL_UNDERFLOW;
            p1->balance = AVL_OVERFLOW;
            *branchShrinked = FALSE;
            }
         r = p1;
         }
      else
         {    /*doppelte LR-Rotation*/
         p2 = p1->right;
         b = (INT1) p2->balance;
         p1->right = p2->left;
         p2->left = p1;
         r->left = p2->right;
         p2->right = r;
         r->balance = (b == AVL_UNDERFLOW) ? AVL_OVERFLOW : AVL_BALANCED;
         p1->balance = (b == AVL_OVERFLOW) ? AVL_UNDERFLOW : AVL_BALANCED;
         r = p2;
         r->balance = AVL_BALANCED;
         }
   }
*root = r;
} /*CAVL::BalanceRShrink*/

// -----------------------------------------------------------------------------
// CAVL::Free sucht das groesste Element im linken Unterbaum des auszufuegenden  
// elements. Es nimmt die Stelle des auszufuegenden Elements im Baum ein. Dazu
// werden die Datenpointer der beiden Baumknoten *root und *p vertauscht.     
// Danach kann in CAVL::Delete *root geloescht werden. Seine Stelle im CAVL::-Baum  
// wird vom Knoten root->left eingenommen. Welcher Knoten zu loeschen ist,    
// wird von CAVL::Free in p vermerkt.                                            

static void DoCAVL::Free (pCAVLNode* root, pCAVLNode* delNode, pBOOLEAN branchShrinked)
{
   CAVLNode* r, d;
   void *  h;

r = *root;
if (r->right)
   {
   DoCAVL::Free (&(r->right), delNode, branchShrinked);
   if (*branchShrinked)
      CAVL::BalanceRShrink (&(r), branchShrinked);
   }
else
   {
   d = *delNode;
   h = r->pData;
   r->pData = d->pData;
   d->pData = h;
   d = r;
   r = r->left;
   *branchShrinked = TRUE;
   *delNode = d;
   }
*root = r;     
} 

// -----------------------------------------------------------------------------

static short DoCAVL::Delete (pCAVL:: a, pCAVLNode* root, PPTR ppData)
{
   CAVLNode* r, d;
	short		avlRes = E_PTV_NOTFOUND;

ENTER ("DoCAVL::Delete");
if (!(r = *root))
   a->branchChanged = FALSE;
else
   {
   switch (a->cmpFunc (r->pData, a->key))
      {
      case 1:
         if (avlRes = DoCAVL::Delete (a, &(r->left), ppData))
				LEAVE1 (0, avlRes);
         if (a->branchChanged)
              CAVL::BalanceLShrink (&(r), &(a->branchChanged));
         break;

      case -1:
         if (avlRes = DoCAVL::Delete (a, &(r->right), ppData))
				LEAVE1 (0, avlRes);
         if (a->branchChanged)
            CAVL::BalanceRShrink (&(r), &(a->branchChanged));
         break;

      case 0:
			avlRes = E_PTV_OK;
         d = r;
         if (!r->right)
            {
            r = r->left;
            a->branchChanged = TRUE;
            }
         else if (!r->left)
            {
            r = r->right;
            a->branchChanged = TRUE;
            }
         else 
            {
            DoCAVL::Free (&(d->left), &(d), &(a->branchChanged));
            if (a->branchChanged)
               CAVL::BalanceLShrink (&(r), &(a->branchChanged));
            }
			if (ppData)
				*ppData = &(d->pData);
			else if (!d->userData)
#if FORTIFY
            if (!a->freeDataFunc)
               MFree (d->pData);
            else
#endif
				if (a->bUseStack)
					DelStack (&(a->dataStack), (LPSTR) d->pData, NULL);
				else
					a->freeDataFunc (&d->pData);
#if FORTIFY
         if (!a->freeFunc)
            MFree (d);
         else
#endif
			if (a->bUseStack)
				DelStack (&(a->avlStack), (LPSTR) d, NULL);
			else
				a->freeFunc ((void **) &(d));
         break;
      }
   }
*root = r;
LEAVE1 (0, a->avlRes = avlRes);
} /*DoCAVL::Delete*/

// -----------------------------------------------------------------------------

short LIBEXP CAVL::Delete (pCAVL:: a, void * key, PPTR ppData)
{
ENTER ("CAVL::Delete");
if (!a->cmpFunc)
	LEAVE1 (0, a->avlRes = E_PTV_INVARG);
a->branchChanged = FALSE;
a->key = (LPSTR) key;
a->avlRes = (a->root ? DoCAVL::Delete (a, &(a->root), ppData) : E_PTV_NOTFOUND);
LEAVE1 (0, a->avlRes);
} /*CAVL::Delete*/

// -----------------------------------------------------------------------------

static void DoCAVL::FreeAll
#ifdef _ANSIC
   (pCAVL:: a, CAVLNode* root)
#else
   (a, root)
   pCAVL::     a;
   CAVLNode* root;
#endif
{
if (root)
   {
   DoCAVL::FreeAll (a, root->left);
   DoCAVL::FreeAll (a, root->right);
   if (!root->userData)
#if FORTIFY
      if (!a->freeDataFunc)
         MFree (root->pData);
      else
#endif
      a->freeDataFunc (&root->pData);
#if FORTIFY
   if (!(a->freeFunc || a->bUseStack))
      MFree (root);
   else
#endif
	if (!a->bUseStack)
	   a->freeFunc ((void **) &(root));
   }
} /*DoCAVL::FreeAll*/

// -----------------------------------------------------------------------------

void CAVL::FreeAll (pCAVL:: a)
{
OMPLOCK (a->sem, 1);
if (a->bUseStack)
	{
	FreeStack (&(a->avlStack));
	FreeStack (&(a->dataStack));
	}
else if (a->root)
	DoCAVL::FreeAll (a, a->root);
OMPUNLOCK;
a->root = (CAVLNode*) NULL;
} /*CAVL::FreeAll*/

// -----------------------------------------------------------------------------

static void DoCAVL::FreeData
#ifdef _ANSIC
   (pCAVL:: a, CAVLNode* root)
#else
   (a, root)
   pCAVL::     a;
   CAVLNode* root;
#endif
{
if (root)
   {
   DoCAVL::FreeData (a, root->left);
   DoCAVL::FreeData (a, root->right);
   if (!root->userData)
      {
      a->freeDataFunc (&root->pData);
      root->pData = NULL;
      }
   }
} /*DoCAVL::FreeAll*/

// -----------------------------------------------------------------------------

void CAVL::FreeData (pCAVL:: a)
{
if (a->root)
   {
	OMPLOCK (a->sem, 2);
   SetMemFunc (a->freeDataFunc, PtvFree);
   DoCAVL::FreeData (a, a->root);
	OMPUNLOCK;
   }
} /*CAVL::FreeAll*/

// -----------------------------------------------------------------------------

static BOOLEAN DoCAVL::Walk
#ifdef _ANSIC
   (CAVLNode* root, pCAVL::WalkFunc walkFunc, void * userData)
#else
   (root, walkFunc, userData)
   CAVLNode*       root;
   pCAVL::WalkFunc   walkFunc;
   void *        userData;
#endif
{
if (root)
   {
   ++avlDepth;
   if (!DoCAVL::Walk (root->left, walkFunc, userData))
      return FALSE;
   if (!walkFunc (root->pData, userData))
      return FALSE;
   if (!DoCAVL::Walk (root->right, walkFunc, userData))
      return FALSE;
   --avlDepth;
   }
return TRUE;
} /*DoCAVL::Walk*/

// -----------------------------------------------------------------------------

BOOLEAN LIBEXP CAVL::Walk
#ifdef _ANSIC
   (pCAVL:: a, pCAVL::WalkFunc walkFunc, void * userData)
#else
   (a, walkFunc, userData)
   pCAVL::				a;
	pCAVL::WalkFunc	walkFunc;
   void *			userData;
#endif
{
if (!(walkFunc || (walkFunc = a->walkFunc)))
	return FALSE;
return DoCAVL::Walk (a->root, walkFunc, userData);
} /*CAVL::Walk*/

#undef SetMemFunc

// -----------------------------------------------------------------------------

void * LIBEXP CAVL::Min
#ifdef _ANSIC
   (pCAVL:: a)
#else
   (a, key)
   pCAVL::        a;
#endif
{
   CAVLNode* pa;

for (pa = a->root; pa && pa->left; pa = pa->left)
	;
return pa;
} /*AVLMin*/

// -----------------------------------------------------------------------------

static void DoCAVL::ExtractMin
#ifdef _ANSIC
   (pCAVL:: a, pCAVLNode* root, PPTR ppData)
#else
   (a, root)
   pCAVL::        a;
   pCAVLNode*   root;
#endif
{
   CAVLNode* r, d;

if (!(r = *root))
   a->branchChanged = FALSE;
else if (r->left)
   {
   DoCAVL::ExtractMin (a, &(r->left), ppData);
   if (a->branchChanged)
		CAVL::BalanceLShrink (&(r), &(a->branchChanged));
	}
else
	{
   d = r;
   r = (CAVLNode*) NULL;
   a->branchChanged = TRUE;
	if (ppData)
		*ppData = &(d->pData);
	else if (!d->userData)
      a->freeDataFunc ((void **) &d->pData);
   a->freeFunc ((void **) &(d));
   }
*root = r;
} /*DoCAVL::ExtractMin*/

// -----------------------------------------------------------------------------

void * LIBEXP CAVL::ExtractMin (pCAVL:: a)
{
	void *		pData;

if (!a->root)
	{
	a->avlRes = E_PTV_NOTFOUND;
	return NULL;
	}
OMPLOCK (a->sem, 1);
DoCAVL::ExtractMin (a, &(a->root), &(pData));
OMPUNLOCK;
a->avlRes = E_PTV_OK;
return pData;
} /*AVLExtractMin*/

// -----------------------------------------------------------------------------

short LIBEXP CAVL::DecKey (pCAVL:: a, void * pOldKey, void * pNewKey)
{
	void *	pData;

if (CAVL::Delete (a, pOldKey, &(pData)))
	return a->avlRes;
if (CAVL::Insert (a, &(pData), 0, pNewKey))
	return a->avlRes;
a->current->userData = 0;
return a->avlRes;
}

// -----------------------------------------------------------------------------
// eof

