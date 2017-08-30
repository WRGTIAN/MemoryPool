#pragma once

//
// 迭代器的型别
//
struct InputIteratorTag {};
struct OutputIteratorTag {};
struct ForwardIteratorTag : public InputIteratorTag {};
struct BidirectionalIteratorTag : public ForwardIteratorTag {};
struct RandomAccessIteratorTag : public BidirectionalIteratorTag {};

// 迭代器的萃取是为了弥补T*不符合迭代器的标准--（内嵌定义5种类型）

template <class Iterator>
struct IteratorTraits 
{
	typedef typename Iterator::iterator_category iterator_category;
	typedef typename Iterator::value_type        value_type;
	typedef typename Iterator::difference_type   difference_type;
	typedef typename Iterator::pointer           pointer;
	typedef typename Iterator::reference         reference;
};

template <class T>
struct IteratorTraits<T*> 
{
	typedef RandomAccessIteratorTag iterator_category;
	typedef T                          value_type;
	typedef	size_t					   difference_type;
	typedef T*                         pointer;
	typedef T&                         reference;
};

template <class T>
struct IteratorTraits<const T*> 
{
	typedef RandomAccessIteratorTag iterator_category;
	typedef T                          value_type;
	typedef size_t							difference_type;
	typedef const T*                         pointer;
	typedef const T&                         reference;
};


// O(1)
template <class InputIterator>
inline size_t __Distance(InputIterator first, InputIterator last,
						 RandomAccessIteratorTag)
{
	cout<<"__Distance->O(1)"<<endl;
	return last-first;
}

// O(N)
template <class InputIterator>
inline size_t __Distance(InputIterator first, InputIterator last,
						 InputIteratorTag)
{
	cout<<"__Distance->O(N)"<<endl;
	size_t n = 0;
	while (first != last)
	{
		++n;
		++first;
	}

	return n;
	
}

template <class InputIterator>
inline size_t Distance(InputIterator first, InputIterator last)
{
	return __Distance(first, last, IteratorTraits<InputIterator>::iterator_category());
}