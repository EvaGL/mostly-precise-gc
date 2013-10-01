#pragma once
#include "collect.h"
#include <vector>

extern std::vector<void *> offsets;
extern bool new_active;

template <class T> 
class gc_ptr
{
public:

	T* ptr;

	T& operator*() const
	{
		return *ptr; 
	}

	T* operator->() const
	{
		return ptr; 
	}

	T* get() const
	{
		return ptr; 
	}

	T& operator[](size_t index) const
	{
		return ptr[index]; 
	}

	T& operator[](size_t index)
	{
		return ptr[index]; 
	}

	gc_ptr& operator = (const gc_ptr <T> &a) 
	{
		ptr = a.ptr;

		return *this;
	}

	bool operator == (const gc_ptr <T> &a) 
	{
		return (a.ptr == ptr);
	}

	bool operator != (const gc_ptr <T> &a)
	{
		return (a.ptr != ptr);
	}

	bool operator != (const T* a)
	{
		return (a != ptr);
	}

	bool operator == (const T* a)
	{
		return (a == ptr);
	}

	bool operator == (const int a)
	{
		return (a == reinterpret_cast<size_t> (ptr));
	}

	bool operator != (const int a)
	{
		return (a != reinterpret_cast<size_t> (ptr));
	}

	gc_ptr& operator = (T *a)
	{
		ptr = a;
		return *this;
	}

	operator bool() const {
		return (ptr!=NULL);
	}


	gc_ptr()
	{
		if (!new_active)
			me = inc(this), stack_ptr = true;
		else
		{
			stack_ptr = 0;
			offsets.push_back(this);
			me = 0;
		}
		ptr = 0;
	}

	gc_ptr(T* p)
	{
		if (!new_active)
			me = inc(this), stack_ptr = true;
		else
		{
			stack_ptr = 0;
			offsets.push_back(this);
			me = 0;
		}
		ptr = p;
	}

	gc_ptr(const gc_ptr <T> &p)
	{
		if (!new_active)
			me = inc(this), stack_ptr = true;
		else
		{
			stack_ptr = 0;
			offsets.push_back(this);
			me = 0;
		}
		ptr = p.ptr;
	}



	~gc_ptr()
	{
		if (stack_ptr)
		{
			dec(me);
		}
	}
	ptr_list *me;
	bool stack_ptr;
};

