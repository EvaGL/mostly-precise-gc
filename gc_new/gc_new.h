#pragma once
#include <cstdio>
#include <pthread.h>
#include <vector>
#include <cstdlib>
#include "../object_repres/go.h"
#include "../object_repres/boxing2.h"
#include <typeinfo>
#include <unordered_map>
#include <string>

extern pthread_mutex_t mut;
extern std::vector <void *> offsets;
extern bool new_active;
extern std::vector <void *> ptr_in_heap;
extern std::unordered_map <std::string, void *> list_meta_obj;
extern size_t counter;

using std::make_pair;

class base_meta
{
	public:
	void *shell;
	size_t size;
	bool mbit;	
	virtual void del_ptr () = 0;
	virtual void* get_begin () = 0;
};

template <class T> class meta : public base_meta
{
	public:
	T* ptr;
	void del_ptr ()
	{
		if (size == 1)
		{
			((T*)ptr)->~T();
		}
		else
		{
			for (size_t i = 0; i < size; i++)
			  	((T*)ptr)[i].~T();
		}
	}
	void* get_begin ()
	{
		return reinterpret_cast <void*> (this);
	}
};
template <class T> T* gc_new (size_t count=1)
{
	while (pthread_mutex_trylock(&mut)!=0){};

	counter += sizeof(T);
	if (counter >= 50000000)
	{
		mark_and_sweep();
		counter = 0;
	}

	void *res;

	if (count <= 0)
		return 0;

	new_active = true;
	offsets.clear();

	if (count == 1)
	{

		res = malloc(sizeof(T) + sizeof(void*) + sizeof(meta<T>));
		new (res + sizeof(void*) + sizeof(meta<T>)) T;
		*((size_t*)(res + sizeof(meta<T>))) =  reinterpret_cast <size_t> (new (res) meta<T>);
		meta<T>* m_inf = reinterpret_cast <meta<T>* > (res);

		if (offsets.size() == 0)
		{
			if (list_meta_obj.count(typeid(T).name()))
				m_inf->shell = list_meta_obj[typeid(T).name()];
			else 
			{
				m_inf->shell = generic_box_simple ();
				list_meta_obj[typeid(T).name()] = m_inf->shell;
			}
		}
		else
		{
			std::list <size_t> offsets_ptr;
			for (size_t i = 0; i < offsets.size(); i++)
			{
				offsets_ptr.push_front(reinterpret_cast <size_t> (offsets[i]) - reinterpret_cast <size_t> (res + sizeof(void*) + sizeof(meta<T>)));
			}
			if (list_meta_obj.count(typeid(T).name()))
				m_inf->shell = list_meta_obj[typeid(T).name()];
			else 
			{
				m_inf->shell = generic_box_struct (offsets_ptr, sizeof(T), count);
				list_meta_obj[typeid(T).name()] = m_inf->shell;
			}
		}
		m_inf->size = count;
		m_inf->mbit = 0;
		m_inf->ptr = (T*)(res + sizeof(base_meta*) + sizeof(meta<T>));
	}
	else
	{

		res = malloc(sizeof(T) * count + sizeof(void *) + sizeof(meta<T>));
		new (res + sizeof(void*) + sizeof(meta<T>)) T[count];
		*((size_t*)(res + sizeof(meta<T>))) = reinterpret_cast <size_t> (new (res) meta<T>);
		meta<T>* m_inf = reinterpret_cast <meta<T>* > (res);		

		if (offsets.size() == 0)
		{
			if (list_meta_obj.count(typeid(T).name()))
				m_inf->shell = list_meta_obj[typeid(T).name()];
			else 
			{
				m_inf->shell = generic_box_unboxed_array (count);
				list_meta_obj[typeid(T).name()] = m_inf->shell;
			}
		}
		else
		{
			std::list <size_t> offsets_ptr;
			for (size_t i = 0; i < offsets.size() / count; i++)
				offsets_ptr.push_front(reinterpret_cast <size_t> (offsets[i]) - reinterpret_cast <size_t> (res + sizeof(void*) + sizeof(meta<T>)));
			if (list_meta_obj.count(typeid(T).name()))
				m_inf->shell = list_meta_obj[typeid(T).name()];
			else 
			{
				m_inf->shell = generic_box_struct (offsets_ptr, sizeof(T), count);
				list_meta_obj[typeid(T).name()] = m_inf->shell;
			}
		}
		m_inf->size = count;
		m_inf->mbit = 0;
		m_inf->ptr = (T*)(res + sizeof(base_meta*) + sizeof(meta<T>));
	}

	ptr_in_heap.push_back(res + sizeof(base_meta*) + sizeof(meta<T>));
	new_active = false;
	offsets.clear();
	pthread_mutex_unlock(&mut);
	
	return (T*)(res + sizeof(base_meta*) + sizeof(meta<T>));
}
