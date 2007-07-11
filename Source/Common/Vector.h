#ifndef _VECTOR_H_INCLUDED_
#define _VECTOR_H_INCLUDED_

#define VECTOR_BLOCK_SIZE 256

template <class T>
class Vector 
{
public:
	enum { NPOS = -1 };

	Vector()
	{
		m_iNumElements = 0;
		m_iArraySize = 0;
		m_pArray = NULL;
	}
	~Vector()
	{
		Free();
	}

	T& operator[](UINT i)
	{
		_ASSERTE(i >= 0);
		if(i >= m_iArraySize)
		{
			Expand(m_iArraySize + VECTOR_BLOCK_SIZE);
		}
		m_iNumElements = max(m_iNumElements, i + 1);
		return m_pArray[i];
	}

	const T& operator[](UINT i) const
	{
		_ASSERTE(i >= 0 && i < m_iNumElements);

		return m_pArray[i];
	}

	bool Expand(UINT size)
	{
		_ASSERTE(size > m_iArraySize);

		if(size > m_iArraySize)
		{
			T* pNewArray = new T[size];
			_ASSERTE(pNewArray != NULL);
			if(pNewArray)
			{
				if(m_pArray)
				{
					CopyMemory(pNewArray, m_pArray, m_iArraySize * sizeof(T));
					delete[] m_pArray;
				}

				m_pArray = pNewArray;
				m_iArraySize = size;
				return true;
			}
		}
		return false;
	}
	
	void Free()
	{
		delete[] m_pArray;		
		m_pArray = NULL;

		m_iNumElements = 0;
		m_iArraySize = 0;
	}

	T* Buffer()
	{
		return m_pArray;
	}

	UINT Size() const
	{
		return m_iNumElements;
	}

	bool Shrink(UINT size)
	{
		_ASSERTE(size >= 0);

		if(size >= 0 && size <= m_iNumElements)
		{
			m_iNumElements = size;
			return true;
		}
		return false;
	}

	void Clear()
	{
		Shrink(0);
	}

	bool Erase(UINT index, UINT nElements = 1)
	{
		_ASSERTE(m_iNumElements > 0);
		_ASSERTE(nElements > 0);

		if(m_iNumElements > 0 && nElements > 0 && index < m_iNumElements && index + nElements <= m_iNumElements)
		{
			MoveMemory(m_pArray + index, m_pArray + index + nElements, (m_iNumElements - index - nElements) * sizeof(T));
			m_iNumElements -= nElements;
			return true;
		}
		return false;
	}

	UINT Append(T t)
	{
		(*this)[m_iNumElements] = t;
		return m_iNumElements;
	}

	UINT Find(T t)
	{
		for(UINT i = 0; i < m_iNumElements; ++i)
		{
			if(m_pArray[i] == t)
				return i;
		}
		return NPOS;
	}

protected:
	UINT m_iNumElements;
	UINT m_iArraySize;
	T* m_pArray;	
};


#endif//_VECTOR_H_INCLUDED_