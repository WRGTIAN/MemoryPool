#pragma once
#include "Alloc.h"
#include "Construct.h"
#include "Iterator.h"
#include <time.h>

template<class T>
struct __ListNode
{
	__ListNode<T>* _next;
	__ListNode<T>* _prev;
	T _data;
};

template<class T, class Ref, class Ptr>
struct __ListIterator
{
	typedef __ListNode<T> Node;
	typedef __ListIterator<T, Ref, Ptr> Self;
	Node* _node;

	typedef BidirectionalIteratorTag iterator_category;
	typedef T value_type;
	typedef Ptr pointer;
	typedef Ref reference;
	typedef size_t difference_type;

	__ListIterator(Node* node)
		:_node(node)
	{}

	Ref operator*()
	{
		return _node->_data;
	}

	Ptr operator->()
	{
		return &(operator*());
	}

	Self& operator++()
	{
		_node = _node->_next;
		return *this;
	}

	Self& operator--()
	{
		_node = _node->_prev;
		return *this;
	}

	bool operator!=(const Self& s) const
	{
		return _node != s._node;
	}
};

template<class T, class Alloc = alloc>
class List
{
	typedef __ListNode<T> Node;
	typedef SimpleAlloc<Node, Alloc> ListNodeAllocator;
public:
	typedef __ListIterator<T, T&, T*> Iterator;
	typedef __ListIterator<T, const T&, const T*> ConstIterator;

	Iterator Begin()
	{
		return _head->_next;
	}

	Iterator End()
	{
		return _head;
	}

	Node* BuyNode(const T& x)
	{
		Node* node = ListNodeAllocator::Allocate();
		Construct(&node->_data, x);
		return node;
	}

	void DestoryNode(Node* node)
	{
		Destroy(&node->_data);
		ListNodeAllocator::Deallocate(node);
	}

	List()
	{
		_head = BuyNode(T());
		_head->_next = _head;
		_head->_prev = _head;
	}

	~List()
	{
		Clear();
		DestoryNode(_head);
		_head = NULL;
	}

	void Clear()
	{
		Destroy(Begin(), End());
		Node* cur = _head->_next;
		while (cur != _head)
		{
			Node* next = cur->_next;
			ListNodeAllocator::Deallocate(cur);
			cur = next;
		}

		_head->_next = _head;
		_head->_prev = _head;
	}

	void PushBack(const T& x)
	{
		Node* tail = _head->_prev;
		Node* tmp = BuyNode(x);

		tail->_next = tmp;
		tmp->_prev = tail;
		tmp->_next = _head;
		_head->_prev = tmp;
	}

protected:
	Node* _head;
};

#include <string>

void TestList()
{
	clock_t start;
	List<int> l1;
	start = clock();
	for (int i = 0; i < 10000000; i++)
	{
		l1.PushBack(i);
	}
	std::cout << "stack1 allocator Time: ";
	std::cout << (clock() - start) << std::endl;
	
}