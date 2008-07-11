
#ifndef __GC_PTR_H__
#define __GC_PTR_H__

#include "sdlheader.h"
#include <map>
#include <set>
#include <typeinfo>
#include <iostream>
#include <cassert>
#include <string>

#include "boost/type_traits/is_fundamental.hpp"

template <typename T, typename U = boost::is_fundamental<T> >
struct gc_default_shader
{
	static void shade(T* ref)
	{
		ref->shade();
	}
};

template <typename T>
struct gc_default_shader<T, boost::true_type>
{
	static void shade(T* ref)
	{
	}
};

template <typename T>
struct gc_null_shader
{
	static void shade(T* ref)
	{
	}
};

class gc_marker_base
{
	public:
		enum Mark
		{
			MARK_WHITE = 0,
			MARK_GRAY,
			MARK_NUM
		};

	private:
		Mark mark;

		typedef std::set<gc_marker_base*> MarkerSet;
		static MarkerSet marked[MARK_NUM];
		static MarkerSet marked_temp[MARK_NUM];
		static SDL_mutex *mutex;
		static bool startedCollecting;
		static enum CollectStep {
			COLLECTSTEP_NOTSTARTED,
			COLLECTSTEP_SHADING,
			COLLECTSTEP_DEALLOCATING
		} collectStep;

	protected:
		
		Mark tempMark;
		int refs;

		void insert();

		void shade();

	public:

		gc_marker_base(Mark mark, int refs);
		
		virtual void dispose() = 0;
		virtual void blacken() = 0;
		virtual int size() = 0;
		virtual std::string name() = 0;

		void increfs();

		void decrefs();

		static void sweep();

	template <typename T, typename _Shader>
	friend class gc_marker;
	template <typename T, typename _Shader, typename _Counter>
	friend class gc_ptr;

};

template <typename T, typename _Shader = gc_default_shader<T> >
class gc_marker : public gc_marker_base
{
	private:
		T* ref;
		void(*func)(T*);

		void dispose()
		{
//			std::cout << "dispose " << typeid(T).name() << " @ " << ref << std::endl;
			if (ref)
			{
				if (func)
					func(ref);
				else
					delete ref;
			}
		}
			
		void blacken()
		{
			if (ref)
			{
				_Shader::shade(ref);
			}
		}

		int size()
		{
			return ref ? sizeof(T) : 0;
		}

		std::string name()
		{
			return ref ? std::string(typeid(T).name()) : "NULL";
		}

	public:
		gc_marker(T* ref = NULL, void(*func)(T*) = NULL, Mark mark = MARK_WHITE, int refs = 0) : gc_marker_base(mark, refs), ref(ref), func(func)
		{
		}
		
};

struct gc_default_counter
{
	static const gc_marker_base::Mark defaultMark = gc_marker_base::MARK_WHITE;
	static const int defaultRefs = 0;

	static void increfs(gc_marker_base* m)
	{
		
	}
	
	static void decrefs(gc_marker_base* m)
	{
		
	}
};

struct gc_root_counter
{
	static const gc_marker_base::Mark defaultMark = gc_marker_base::MARK_GRAY;
	static const int defaultRefs = 1;

	static void increfs(gc_marker_base* m)
	{
		if (m)
			m->increfs();
	}
	
	static void decrefs(gc_marker_base* m)
	{
		if (m)
			m->increfs();
	}
};

template <typename T, typename _Counter = gc_default_counter, typename _Shader = gc_default_shader<T> >
class gc_ptr
{
	protected:
		T* ref;
		gc_marker_base* m;

	public:
		gc_ptr() : ref(NULL), m(NULL)
		{
		}

		template <typename T2, typename _Counter2, typename _Shader2>
		gc_ptr(const gc_ptr<T2, _Counter2, _Shader2>& a) : ref(a.ref), m(a.m)
		{
			_Counter::increfs(m);
		}

		gc_ptr(const gc_ptr& a) : ref(a.ref), m(a.m)
		{
			_Counter::increfs(m);
		}

		template <typename T2, typename _Shader2>
		gc_ptr(T2* a, void(*func)(T2*) = NULL) : ref(a), m(a ? new gc_marker<T2, _Shader2>(a, func, _Counter::defaultMark, _Counter::defaultRefs) : NULL)
		{
		}

		gc_ptr(T* a, void(*func)(T*) = NULL) : ref(a), m(a ? new gc_marker<T, _Shader>(a, func, _Counter::defaultMark, _Counter::defaultRefs) : NULL)
		{
		}

		~gc_ptr()
		{
			_Counter::decrefs(m);
		}

		gc_ptr& operator = (const gc_ptr& a)
		{
			_Counter::increfs(a.m);
			_Counter::decrefs(m);

			ref = a.ref;
			m = a.m;
			return *this;
		}

		bool operator < (const gc_ptr& a) const
		{
			return ref < a.ref;
		}

		bool operator == (const gc_ptr& a) const
		{
			return ref == a.ref;
		}

		bool operator != (const gc_ptr& a) const
		{
			return ref != a.ref;
		}

		T& operator * () const throw()
		{
			return *ref;
		}

		T* operator -> () const throw()
		{
			return ref;
		}

		T& operator [] (unsigned i) const throw()
		{
			return ref[i];
		}

		operator bool() const
		{
			return ref != 0;
		}

		void reset()
		{
			*this = gc_ptr();
		}

		void shade() const
		{
			if (m)
				m->shade();
		}

	
	template <typename T2, typename _Counter2, typename _Shader2>
	friend class gc_ptr;
};

template <typename T, typename _Shader = gc_default_shader<T> >
struct gc_root_ptr
{
	typedef gc_ptr<T, gc_root_counter, _Shader> type;
};

template <typename T>
void array_deleter(T* a)
{
	delete[] a;
}

template <typename T>
void null_deleter(T* a)
{
}

template <typename T>
class gc_ptr_from_this
{
	private:
		gc_ptr<T> self;
	public:
		gc_ptr_from_this()
		{
			self = gc_ptr<T>(static_cast<T*>(this));
		}

		gc_ptr<T> GetRef()
		{
			return self;
		}
		
		gc_ptr<T> GetWeak()
		{
			return self;
		}
		
		static gc_ptr<T> New()
		{
			return (new T)->GetRef();
		}

};

template <typename T2>
void gc_shade_container(const T2& c)
{
	for (typename T2::const_iterator it = c.begin(); it != c.end(); it++)
		(*it).shade();
}

template <typename T2>
void gc_shade_map(const T2& c)
{
	for (typename T2::const_iterator it = c.begin(); it != c.end(); it++)
		(*it).second.shade();
}

template <typename T2>
void gc_shade_map_key(const T2& c)
{
	for (typename T2::const_iterator it = c.begin(); it != c.end(); it++)
		(*it).first.shade();
}

template <typename T2>
void gc_shade_map_key_value(const T2& c)
{
	for (typename T2::const_iterator it = c.begin(); it != c.end(); it++)
	{
		(*it).first.shade();
		(*it).second.shade();
	}
}

#include "gc_array.h"

#endif
