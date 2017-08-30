#pragma once
#include <time.h>
#include "Alloc.h"
#include "Construct.h"

template<class T, class Alloc = alloc>
class Vector
{
public:
	typedef T* Iterator;
	typedef SimpleAlloc<T, Alloc> DataAllocator;

	Vector()
		:_start(NULL)
		,_finish(NULL)
		,_endOfStorage(NULL)
	{}

	Iterator Begin()
	{
		return _start;
	}

	Iterator End()
	{
		return _finish;
	}

	~Vector()
	{
		Destroy(Begin(), End());
		DataAllocator::Deallocate(_start, Size());
	}

	void PushBack(const T& x)
	{
		CheckCapacity();

		//*_finish = x; // T string operator=
		Construct(_finish, x);
		++_finish;
	}

	size_t Size()
	{
		return _finish-_start;
	}

	size_t Capacity()
	{
		return _endOfStorage-_start;
	}

	void CheckCapacity()
	{
		if (_finish == _endOfStorage)
		{
			size_t size = Size();
			size_t newSize = size != 0 ? 2*size : 3; 
			T* tmp = DataAllocator::Allocate(newSize);

			if (_start)
			{
				// 
				//UninitializedCopy();
				for (size_t i = 0; i < size; ++i)
				{
					Construct(tmp+i, _start[i]);
				}

				Destroy(_start, _finish);
				DataAllocator::Deallocate(_start, size);
			}

			_start = tmp;
			_finish = _start +size;
			_endOfStorage = _start + newSize;

		}	
	}

protected:
	Iterator _start;
	Iterator _finish;
	Iterator _endOfStorage;
};

void TestVector()
{
	clock_t start;

	Vector<int> v1;

	start = clock();
	for (int i = 0; i < 100000000; i++)
	{
		v1.PushBack(i);
	}
	std::cout << "stack1 allocator Time: ";
	std::cout << (clock() - start)<< std::endl;
	
} 