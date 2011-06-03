#ifdef _CQUICKSORT_H

//-----------------------------------------------------------------------------

template <typename _T>
inline void CQuickSort< _T >::Swap (_T* left, _T* right)
{
_T h = *left;
*left = *right;
*right = h;
}

//-----------------------------------------------------------------------------

template <typename _T>
void CQuickSort< _T >::SortAscending (_T* buffer, int left, int right) 
{
	int	l = left,
			r = right;
	_T		median = buffer [(l + r) / 2];

do {
	while (buffer [l] < median)
		l++;
	while (buffer [r] > median)
		r--;
	if (l <= r) {
		if (l < r)
			Swap (buffer + l, buffer + r);
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	SortAscending (buffer, l, right);
if (left < r)
	SortAscending (buffer, left, r);
median.info.visibleVertices = NULL;
};

//-----------------------------------------------------------------------------

template <typename _T>
void CQuickSort< _T >::SortDescending (_T* buffer, int left, int right) 
{
	int	l = left,
			r = right;
	_T		median = buffer [(l + r) / 2];

do {
	while (buffer [l] > median)
		l++;
	while (buffer [r] < median)
		r--;
	if (l <= r) {
		if (l < r)
			Swap (buffer + l, buffer + r);
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	SortDescending (buffer, l, right);
if (left < r)
	SortDescending (buffer, left, r);
};

//-----------------------------------------------------------------------------

template <typename _T>
void CQuickSort< _T >::SortAscending (_T* buffer, int left, int right, comparator compare) 
{
	int	l = left,
			r = right;
	_T		median = buffer [(l + r) / 2];

do {
	while (compare (buffer + l, &median) < 0)
		l++;
	while (compare (buffer + r, &median) > 0)
		r--;
	if (l <= r) {
		if (l < r)
			Swap (buffer + l, buffer + r);
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	SortAscending (buffer, l, right, compare);
if (left < r)
	SortAscending (buffer, left, r, compare);
};

//-----------------------------------------------------------------------------

template <typename _T>
void CQuickSort< _T >::SortDescending (_T* buffer, int left, int right, comparator compare) 
{
	int	l = left,
			r = right;
	_T		m = buffer [(l + r) / 2];

do {
	while (compare (buffer + l, &m) > 0)
		l++;
	while (compare (buffer + r, &m) < 0)
		r--;
	if (l <= r) {
		if (l < r)
			Swap (buffer + l, buffer + r);
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	SortDescending (buffer, l, right, compare);
if (left < r)
	SortDescending (buffer, left, r, compare);
}

// ----------------------------------------------------------------------------

template <typename _T>
int CQuickSort< _T >::BinSearch (_T* buffer, int l, int r, _T key)
{
	int	m;

do {
	m = (l + r) / 2;
	if (key < buffer [m])
		r = m - 1;
	else if (key > buffer [m])
		l = m + 1;
	else {
		// find first record with equal key
		for (; m > 0; m--)
			if (key > buffer [m - 1])
				break;
		return m;
		}
	} while (l <= r);
return -1;
}

//-----------------------------------------------------------------------------

#endif //_CQUICKSORT_H
