#ifndef _STRINGT_H_INCLUDED_
#define _STRINGT_H_INCLUDED_

#include <string.h>
#include <stdio.h>
#include <tchar.h>

template <class T>
class StringT
{
public:
	StringT()
	{
		m_psz = 0;
		m_nCapacity = 0;
	}

	StringT(const StringT& from, unsigned nMax = -1)
	{
		m_psz = 0;
		m_nCapacity = 0;
		Assign(from, nMax);
	}

	StringT(const T* from, unsigned nMax = -1)
	{
		m_psz = 0;
		m_nCapacity = 0;
		Assign(from, nMax);
	}

	~StringT()
	{
		Clear();
	}

	StringT& operator=(const StringT& from)
	{
		Assign(from);
		return *this;
	}

	StringT& operator=(const T* from)
	{
		Assign(from);
		return *this;
	}

	// operator== is always case sensitive
	// use Compare for case insensitive

	bool operator==(const StringT& from) const
	{
		return Compare(from);
	}

	bool operator==(const T* from) const
	{
		return Compare(from);
	}

	bool operator!=(const StringT& from) const
	{
		return !Compare(from);
	}

	bool operator!=(const T* from) const
	{
		return !Compare(from);
	}

	T& operator[](UINT i)
	{
		_ASSERTE(i < m_nCapacity);
		return m_psz[i];
	}

	const T& operator[](UINT i) const
	{
		_ASSERTE(i < m_nCapacity);
		return m_psz[i];
	}

	StringT& operator+=(const StringT& right)
	{
		Append(right);
		return *this;
	}

	StringT& operator+=(const T* right)
	{
		Append(right);
		return *this;
	}

	StringT& operator+=(const T right)
	{
		Append(right);
		return *this;
	}

	// nMax is the maximum number of characters to copy
	// NOT including the NULL terminator

	// Return value is number of characters copied, 
	// NOT including NULL terminator

	// eg.  str.Assign("Hi There", 2) will result in the
	// new string containing the value "Hi" and will return 2

	// If nMax is specified to a value less than or equal to 
	// the size of the source buffer then the source does not 
	// need to be NULL terminated.

	unsigned Assign(const StringT& from, unsigned nMax = -1)
	{
		return Assign(from.Str(), nMax);
	}

	unsigned Assign(const T* from, unsigned nMax = -1)
	{
		Clear();

		if(from)
		{
			unsigned nCopy = 0;
			while(nCopy < nMax && from[nCopy] != '\0') ++nCopy;

			Reserve(nCopy + 1);

			_tcsncpy(m_psz, from, nCopy);
			m_psz[nCopy] = '\0';

			return nCopy;
		}
		return 0;
	}

	unsigned Append(const StringT& right, unsigned nMax = -1)
	{
		return Append(right.Str(), nMax);
	}

	unsigned Append(const T right)
	{
		return Append(&right, 1);
	}

	unsigned Append(const T* right, unsigned nMax = -1)
	{
		if(right)
		{
			unsigned nCopy = 0;
			while(nCopy < nMax && right[nCopy] != '\0') ++nCopy;

			if(nCopy)
			{
				unsigned nCurrent = Size();

				if(nCurrent + nCopy + 1 > m_nCapacity)
				{
					T* tmp = m_psz;
					if(tmp)
					{
						m_psz = 0;
						m_nCapacity = NULL;
					}

					Reserve(nCurrent + nCopy + 1);

					if(tmp)
					{
						_tcsncpy(m_psz, tmp, nCurrent);
						delete[] tmp;
					}
				}

				_tcsncpy(m_psz + nCurrent, right, nCopy);
				m_psz[nCurrent + nCopy] = '\0';
			}
			return nCopy;
		}
		return 0;
	}

	StringT SubStr(unsigned nPos = 0, unsigned nMax = -1) const
	{
		if(nPos < Size())
		{
			return StringT(Str() + nPos, nMax);
		}
		else
		{
			return StringT();
		}
	}

	StringT GetWord(unsigned nWord = 0, bool bIncludeRest = false) const
	{
		UINT nSize = Size();
		if(nSize > 0)
		{
			UINT nCharStart = 0;
			UINT nCharEnd = 0;
			UINT nWordsUsed = 0;

			while(nWordsUsed <= nWord && nCharStart < nSize)
			{
				// Skip whitespace
				while(nCharStart < nSize && m_psz[nCharStart] == ' ')
					nCharStart++;

				nCharEnd = nCharStart;

				// Find word length
				while(nCharEnd < nSize && !(m_psz[nCharEnd] == ' '))
					nCharEnd++;

				if(nWordsUsed == nWord && nCharEnd > nCharStart)
				{
					return SubStr(nCharStart, bIncludeRest ? -1 : (nCharEnd - nCharStart));
				}

				nWordsUsed++;
				nCharStart = nCharEnd;
			}

		}
		return StringT();
	}

	bool Compare(const StringT& from, bool bCaseSensitive = true) const
	{
		return Compare(from.Str(), bCaseSensitive);
	}

	bool Compare(const T* from, bool bCaseSensitive = true) const
	{
		_ASSERTE(from != NULL);

		if(bCaseSensitive)
		{
			return (_tcscmp(Str(), from) == 0);
		}
		else
		{
			return (_tcsicmp(Str(), from) == 0);
		}	
	}

	// Writing to the return value of Str() when Capacity() == 0
	// would be a BadThing(tm);

	// This pointer may be invalidated by any future Assign, Append, etc...

	T* Str()
	{
		return m_psz ? m_psz : _T("");
	}
	const T* Str() const
	{
		return m_psz ? m_psz : _T("");
	}

	// Calling Clear() multiple times will not cause problems

	void Clear()
	{
		delete[] m_psz;
		m_psz = 0;
		m_nCapacity = 0;
	}

	// Reserve will only reallocate memory if the newly requested
	// size is larger than the currently allocated block

	// nCapacity must include room for the NULL terminator

	void Reserve(unsigned nCapacity)
	{
#if DEBUG && DEBUG_STRING_ALLOCS
		if(nCapacity < 1000)
		{
			++allocs[nCapacity];
		}
		else
		{
			++greater;
		}
#endif
		if(nCapacity > Capacity())
		{
			Clear();

			m_psz = new T[nCapacity];
			m_nCapacity = nCapacity;
			m_psz[0] = '\0';
		}
	}

	// Returns the amount of currently allocated memory

	unsigned Capacity() const
	{
		return m_nCapacity;
	}

	// Returns number of characters NOT including NULL terminator

	unsigned Size() const
	{
		if(m_psz)
			return _tcslen(m_psz);
		return 0;
	}
#if DEBUG && DEBUG_STRING_ALLOCS
	static int allocs[1000];
	static int greater;
#endif
protected:
	T* m_psz;
	unsigned m_nCapacity;

};

typedef StringT<char> StringA;
typedef StringT<wchar_t> StringW;

#ifndef _UNICODE
typedef StringA String;
#else
typedef StringW String;
#endif

#endif//_STRINGT_H_INCLUDED_
