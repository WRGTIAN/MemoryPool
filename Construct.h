#pragma once

#include "TypeTraits.h"

template <class T>
inline void Destroy(T* pointer)
{
	pointer->~T();
}

//new(p)string("11111")
template <class T1, class T2>
inline void Construct(T1* p, const T2& value)
{
	new (p) T1(value);
}

template <class ForwardIterator>
inline void __DestroyAux(ForwardIterator first, ForwardIterator last,
					  __TrueType)
{
	// no-op
}

template <class ForwardIterator>
inline void __DestroyAux(ForwardIterator first, ForwardIterator last,
					  __FalseType)
{
	while(first != last)
	{
		Destroy(&(*first));
		++first;
	}
}

template <class ForwardIterator, class T>
inline void __Destroy(ForwardIterator first, ForwardIterator last, T*)
{
	__DestroyAux(first, last, TypeTraits<T>::has_trivial_destructor());
}

template <class ForwardIterator>
inline void Destroy(ForwardIterator first, ForwardIterator last)
{
	__Destroy(first, last, &(*first)); //T*
}