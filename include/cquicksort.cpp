#ifdef _CQUICKSORT_H

//-----------------------------------------------------------------------------

template <typename DATA_T>
inline void CQuickSort< DATA_T >::Swap (DATA_T* left, DATA_T* right)
{
DATA_T h = *left;
*left = *right;
*right = h;
}

//-----------------------------------------------------------------------------

template <typename DATA_T>
void CQuickSort< DATA_T >::SortAscending (DATA_T* buffer, int32_t left, int32_t right) 
{
	int32_t	l = left,
				r = right;
	DATA_T			median = buffer [(l + r) / 2];

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
};

//-----------------------------------------------------------------------------

template <typename DATA_T>
void CQuickSort< DATA_T >::SortDescending (DATA_T* buffer, int32_t left, int32_t right) 
{
	int32_t	l = left,
				r = right;
	DATA_T		median = buffer [(l + r) / 2];

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

template <typename DATA_T>
void CQuickSort< DATA_T >::SortAscending (DATA_T* buffer, int32_t left, int32_t right, comparator compare) 
{
	int32_t	l = left,
				r = right;
	DATA_T			median = buffer [(l + r) / 2];

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

template <typename DATA_T>
void CQuickSort< DATA_T >::SortDescending (DATA_T* buffer, int32_t left, int32_t right, comparator compare) 
{
	int32_t	l = left,
				r = right;
	DATA_T			m = buffer [(l + r) / 2];

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

template <typename DATA_T>
int32_t CQuickSort< DATA_T >::BinSearch (DATA_T* buffer, int32_t l, int32_t r, DATA_T key)
{
	int32_t	m;

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
