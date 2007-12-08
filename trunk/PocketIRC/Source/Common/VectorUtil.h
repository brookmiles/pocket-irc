#ifndef _VECTORUTIL_H_INCLUDED_
#define _VECTORUTIL_H_INCLUDED_

template <class T> 
void Erase(std::vector<T> v, T t)
{
	std::vector<T>::iterator i = std::find(v.begin(), v.end(), t);
	if(i != v.end())
	{
		v.erase(i);
	}
}

#endif//_VECTORUTIL_H_INCLUDED_
