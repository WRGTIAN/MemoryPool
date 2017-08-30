#pragma once

////////////////////////////////////////////////////////////////
#include <iostream>
using namespace std;
typedef void(*HANDLE_FUNC)();

template <int inst>     //预留参数
class __MallocAllocTemplate
{
public:
	static void* OOM_Malloc(size_t size)
	{
		while(1)
		{
			if (_handle == NULL)
			{
				throw bad_alloc();
			}

			_handle();

			void* ret = malloc(size);
			if(ret)
				return ret;
		}
	}

	static void* Allocate(size_t size)
	{
		void* ret = malloc(size);
		if (ret == 0)
		{
			ret = OOM_Malloc(size);
		}

		return ret;
	}

	static void Deallocate(void* p, size_t n)
	{

		free(p);
	}

	static HANDLE_FUNC SetMallocHandler(HANDLE_FUNC f)
	{
		HANDLE_FUNC old = f;
		_handle = f;
		return old;
	}

private:
	static HANDLE_FUNC _handle;
};

template<int inst>
HANDLE_FUNC __MallocAllocTemplate<inst>::_handle = 0;

/////////////////////////////////////////////////////////
//二级空间配置器
template <bool threads, int inst>
class __DefaultAllocTemplate
{
public:
	static  size_t FREELIST_INDEX(size_t bytes)
	{
		return (((bytes) + __ALIGN-1)/__ALIGN - 1);
	}

	static size_t ROUND_UP(size_t bytes)
	{
		return (bytes+__ALIGN-1)&(~(__ALIGN-1));
	}

	static char* ChunkAlloc(size_t n, size_t& nobjs)
	{

		size_t totalBytes = n*nobjs;
		size_t leftBytes = _endFree - _startFree;
		// 1.有足够nobjs个对象
		// 2.至少够一个对象
	    // 3.一个对象都没有
		if (leftBytes >= totalBytes)
		{
			char* ret = _startFree;
			_startFree += totalBytes;
			return ret;
		}
		else if (leftBytes > n)
		{
			nobjs = leftBytes/n;
			totalBytes = nobjs*n; 
			char* ret = _startFree;
			_startFree += totalBytes;
			return ret;
		}
		else
		{
			if (leftBytes > 0)
			{
				size_t index = FREELIST_INDEX(leftBytes);
				((Obj*)_startFree)->_freeListLink = _freeList[index];
				_freeList[index] = (Obj*)_startFree;
			}


			size_t bytesToGet = totalBytes*2+ROUND_UP(_heapSize>>4);
			_startFree = (char*)malloc(bytesToGet);
			if (_startFree == NULL)
			{
				size_t i = FREELIST_INDEX(n);
				for (size_t i = 0; i < __NFREELISTS; ++i)
				{
					if (_freeList[i])
					{
						Obj* ret = _freeList[i];
						_freeList[i] = ret->_freeListLink;
						_startFree = (char*)ret;
						_endFree = _startFree + (i+1)*__ALIGN;
						return ChunkAlloc(n, nobjs);
					}
				}

				_startFree = (char*)__MallocAllocTemplate<0>::Allocate(bytesToGet);
			}

			_heapSize += bytesToGet;
			_endFree = _startFree + bytesToGet;
			return ChunkAlloc(n, nobjs);
		}
	}

	static char* Refill(size_t n)
	{
		size_t nobjs = 20;
		char* chunk = ChunkAlloc(n, nobjs);
		if (nobjs == 1)
			return chunk;
		
		// 将剩余的nobjs-1块内存挂到自由链表的下面
		size_t index = FREELIST_INDEX(n);
		_freeList[index] = (Obj*)(chunk+n);
		Obj* cur = _freeList[index];
		for (size_t i = 2; i < nobjs; ++i)
		{
			//Obj* next = (Obj*)(chunk+i*n);
			Obj* next = (Obj*)((char*)cur+n);
			cur->_freeListLink = next;
			cur = next;
		}
		cur->_freeListLink = NULL;

		return chunk;
	}

	static void* Allocate(size_t n)
	{
		if (n > __MAX_BYTES)
		{
			// 一级
			return __MallocAllocTemplate<0>::Allocate(n);
		}
		size_t index = FREELIST_INDEX(n);     //申请的n个字节对应内存池的下标
		if (_freeList[index])                   //
		{
			Obj* ret = _freeList[index];
			_freeList[index] = ret->_freeListLink;  //从头取一块
			return ret;           //申请成功
		}
		else
		{
			return Refill(ROUND_UP(n));     //对应下标无内存----》去申请
		}
	}

	static void Deallocate(void* ptr, size_t n)
	{
		if (n > __MAX_BYTES)
		{
			__MallocAllocTemplate<0>::Deallocate(ptr, n);
		}
		else
		{
			size_t index = FREELIST_INDEX(n);

			((Obj*)ptr)->_freeListLink = _freeList[index];
			_freeList[index] = (Obj*)ptr;
		}
	}

private:
	enum {__ALIGN = 8};
	enum {__MAX_BYTES = 128};
	enum {__NFREELISTS = __MAX_BYTES/__ALIGN};

	// 自由链表 ->辅助管理分配出去的内存块
	union Obj
	{
		union Obj* _freeListLink;   //下一块内存地址
		char client_data[1];           //测试用
	};

	static Obj* _freeList[__NFREELISTS];   // 狭义内存池
	static char* _startFree;     //内存池(链表)开始
	static char* _endFree;      //内存池(链表)结束
	static size_t _heapSize;     //从系统堆分配的内存的总大小
};

////////////////////////////////////////////////////////////////


template <bool threads, int inst>
char* __DefaultAllocTemplate<threads, inst>::_startFree = 0;

template <bool threads, int inst>
char* __DefaultAllocTemplate<threads, inst>::_endFree = 0;

template <bool threads, int inst>
size_t __DefaultAllocTemplate<threads, inst>::_heapSize = 0;

template <bool threads, int inst>
typename __DefaultAllocTemplate<threads, inst>::Obj*
__DefaultAllocTemplate<threads, inst>::_freeList[__NFREELISTS] = { 0 };

#ifdef __USE_MALLOC             //使用malloc
	typedef __MallocAllocTemplate<0> alloc;
#else
	typedef __DefaultAllocTemplate<true, 0> alloc;
#endif // __USE_MALLOC


template<class T, class Alloc>
class SimpleAlloc 
{
public:
	static T* Allocate(size_t n)
	{ 
		return 0 == n? 0 : (T*) Alloc::Allocate(n * sizeof (T)); 
	}

	static T* Allocate(void)
	{ 
		return (T*) Alloc::Allocate(sizeof (T));
	}

	static void Deallocate(T *p, size_t n)
	{ 
		if (0 != n)
		{
			Alloc::Deallocate(p, n * sizeof (T));
		}
			
	}
	static void Deallocate(T *p)
	{
		Alloc::Deallocate(p, sizeof (T));
	}
};


