#ifndef __REF_PTR_H__
#define __REF_PTR_H__

#include <cstdlib>
#include <iostream>
#include <typeinfo>

template <typename T>
class enc_ptr;

class ref_counter_base
{
	private:
		unsigned numrefs;
		unsigned weakrefs;

		virtual void dispose() = 0;

	public:
		void decrefs()
		{
			numrefs--;
			if (numrefs == 0)
			{
				dispose();
				if (weakrefs == 0)
				{
					delete this;
				}
			}
		}

		void increfs()
		{
			numrefs++;
		}

		void decweakrefs()
		{
			weakrefs--;
			if (weakrefs == 0 && numrefs == 0)
			{
				delete this;
			}
		}

		void incweakrefs()
		{
			weakrefs++;
		}

		ref_counter_base() : numrefs(1), weakrefs(0)
		{
			
		}
		
		ref_counter_base(int numrefs, int weakrefs) : numrefs(numrefs), weakrefs(weakrefs)
		{
			
		}
		
		template <typename T2>
		friend class enc_ptr;
};

template <typename T>
class ref_counter : public ref_counter_base
{
	private:
		T* ref;
		void(*func)(T*);

		void dispose()
		{
			if (ref)
			{
				if (func)
					func(ref);
				else
					delete ref;
			}
			ref = NULL;
			func = NULL;
		}

	public:
		ref_counter(T* ref) : ref(ref), func(NULL)
		{
		}
		
		ref_counter(T* ref, int numrefs, int weakrefs = 0, void(*func)(T*) = NULL) : ref_counter_base(numrefs, weakrefs), ref(ref), func(func)
		{
		}
		
		ref_counter() : ref(NULL), func(NULL)
		{
		}
		
};

template <typename T>
class ref_ptr
{
	private:
		T* ref;
		ref_counter_base* c;

	public:
		ref_ptr() : ref(NULL), c(new ref_counter<T>)
		{
		}

		template <typename T2>
		ref_ptr(const ref_ptr<T2>& a) : ref(a.ref), c(a.c)
		{
			c->increfs();
		}

		ref_ptr(const ref_ptr<T>& a) : ref(a.ref), c(a.c)
		{
			c->increfs();
		}

		template <typename T2>
		ref_ptr(const enc_ptr<T2>& a) : ref(a.ref), c(a.c)
		{
			c->increfs();
		}

		ref_ptr(const enc_ptr<T>& a) : ref(a.ref), c(a.c)
		{
			c->increfs();
		}

		template <typename T2>
		ref_ptr(T2* a, void(*func)(T2*) = NULL, int numrefs = 1) : ref(a), c(new ref_counter<T2>(a, numrefs, 0, func))
		{
		}

		ref_ptr(T* a, void(*func)(T*) = NULL, int numrefs = 1) : ref(a), c(new ref_counter<T>(a, numrefs, 0, func))
		{
		}

		~ref_ptr()
		{
			c->decrefs();
		}

		ref_ptr& operator = (const ref_ptr& a)
		{
			a.c->increfs();
			
			c->decrefs();

			ref = a.ref;
			c = a.c;
			return *this;
		}

		bool operator == (const ref_ptr<T>& a) const
		{
			return ref == a.ref;
		}

		bool operator != (const ref_ptr<T>& a) const
		{
			return ref != a.ref;
		}

		bool operator == (const T*& a) const
		{
			return ref == a;
		}

		bool operator != (const T*& a) const
		{
			return ref != a;
		}

		template <typename T2>
		bool operator == (const ref_ptr<T2>& a) const
		{
			return ref == a.ref;
		}

		template <typename T2>
		bool operator != (const ref_ptr<T2>& a) const
		{
			return ref != a.ref;
		}

		template <typename T2>
		bool operator == (const T2*& a) const
		{
			return ref == a;
		}

		template <typename T2>
		bool operator != (const T2*& a) const
		{
			return ref != a;
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
			c->decrefs();
			*this = ref_ptr();
		}

		template <typename T2>
		friend class ref_ptr;
		template <typename T2>
		friend class enc_ptr;

};

template <typename T>
class enc_ptr
{
	private:
		T* ref;
		ref_counter_base* c;
	public:
		
		enc_ptr() : ref(NULL), c(new ref_counter<T>)
		{
			c->incweakrefs();
			c->decrefs();
		}

		enc_ptr(const ref_ptr<T>& a) : ref(a.ref), c(a.c)
		{
			c->incweakrefs();
		}
		
		enc_ptr(const enc_ptr<T>& a) : ref(a.ref), c(a.c)
		{
			c->incweakrefs();
		}
		
		enc_ptr(T* a) : ref(a), c(new ref_counter<T>(a, 0, 1))
		{
			c->incweakrefs();
		}
		
		~enc_ptr()
		{
			c->decweakrefs();
		}

		void reset()
		{
			*this = enc_ptr();
		}

		ref_ptr<T> lock()
		{
			return ref_ptr<T>(*this);
		}

		enc_ptr& operator = (const enc_ptr& a)
		{
			a.c->incweakrefs();
			
			c->decweakrefs();

			ref = a.ref;
			c = a.c;
			return *this;
		}

		template <typename T2>
		bool operator == (const enc_ptr<T2>& a) const
		{
			return ref == a.ref;
		}

		template <typename T2>
		bool operator != (const enc_ptr<T2>& a) const
		{
			return ref != a.ref;
		}

		template <typename T2>
		bool operator == (const ref_ptr<T2>& a) const
		{
			return ref == a.ref;
		}

		template <typename T2>
		bool operator != (const ref_ptr<T2>& a) const
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

		template <typename T2>
		friend class ref_ptr;
		template <typename T2>
		friend class enc_ptr;
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
class ref_ptr_from_this
{
	private:
		enc_ptr<T> self;
	public:
		ref_ptr_from_this()
		{
			self = enc_ptr<T>((T*)this);
		}

		ref_ptr<T> GetRef()
		{
			return self;
		}
};

#endif
