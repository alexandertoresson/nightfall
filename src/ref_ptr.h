#ifndef __REF_PTR_H__
#define __REF_PTR_H__

//#define REF_PTR_DEBUG

#include <cstdlib>
#include <iostream>
#include <typeinfo>

#ifdef REF_PTR_DEBUG
#include <map>
#endif

template <typename T>
class enc_ptr;

template <typename T>
class ref_ptr
{
	private:
#ifdef REF_PTR_DEBUG
		static std::map<T*, unsigned*> refs;
		static std::map<T*, unsigned*> test;
#endif
		T* ref;
		unsigned* numrefs;
		unsigned* weakrefs;
		void(*func)(T*);

		void decrefs()
		{
			(*numrefs)--;
			if (*numrefs == 0)
			{
				if (ref)
				{
					if (func)
						func(ref);
					else
						delete ref;
				}
				if (*weakrefs == 0)
				{
#ifdef REF_PTR_DEBUG
					remcheckref();
#endif
					delete numrefs;
					delete weakrefs;
				}
				ref = NULL;
				numrefs = NULL;
				weakrefs = NULL;
				func = NULL;
			}
		}

		void increfs() const
		{
			(*numrefs)++;
		}

#ifdef REF_PTR_DEBUG
		void checkref() const
		{
/*			if (ref && refs[ref] && refs[ref] != numrefs)
				delete test[ref];
			if (ref)
			{
				refs[ref] = numrefs;
				test[ref] = new unsigned;
				delete test[ref];
			}*/
		}

		void remcheckref()
		{
//			refs.erase(ref);
		}
#endif

	public:
		ref_ptr() : ref(NULL), numrefs(new unsigned(1)), weakrefs(new unsigned(0)), func(NULL)
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
		}

		template <typename T2>
		ref_ptr(const ref_ptr<T2>& a) : ref(a.ref), numrefs(a.numrefs), weakrefs(a.weakrefs), func((void(*)(T*))a.func)
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			increfs();
		}

		ref_ptr(const ref_ptr<T>& a) : ref(a.ref), numrefs(a.numrefs), weakrefs(a.weakrefs), func(a.func)
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			increfs();
		}

		template <typename T2>
		ref_ptr(T2* a, void(*func)(T*) = NULL, unsigned refs = 1) : ref(a), numrefs(new unsigned(refs)), weakrefs(new unsigned(0)), func((void(*)(T*))func)
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
		}

		ref_ptr(T* a, void(*func)(T*) = NULL, unsigned refs = 1) : ref(a), numrefs(new unsigned(refs)), weakrefs(new unsigned(0)), func(func)
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
		}

		~ref_ptr()
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			decrefs();
		}

		ref_ptr& operator = (const ref_ptr& a)
		{
			a.increfs();
			
#ifdef REF_PTR_DEBUG
			a.checkref();
			checkref();
#endif
			decrefs();

			ref = a.ref;
			numrefs = a.numrefs;
			weakrefs = a.weakrefs;
			func = a.func;
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return *this;
		}

		template <typename T2>
		ref_ptr& operator = (const ref_ptr<T2>& a)
		{
			a.increfs();
			
#ifdef REF_PTR_DEBUG
			a.checkref();
			checkref();
#endif
			decrefs();

			ref = a.ref;
			numrefs = a.numrefs;
			weakrefs = a.weakrefs;
			func = a.func;
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return *this;
		}

		ref_ptr& operator = (const T*& a)
		{
			if (ref == a)
				return *this;

			decrefs();

			ref = a;
			numrefs = new unsigned(1);
			weakrefs = new unsigned(0);
			func = NULL;
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return *this;
		}

		ref_ptr& operator = (T* a)
		{
			if (ref == a)
				return *this;

			decrefs();

			ref = a;
			numrefs = new unsigned(1);
			weakrefs = new unsigned(0);
			func = NULL;
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return *this;
		}

		template <typename T2>
		ref_ptr& operator = (const T2*& a)
		{
			if (ref == a)
				return *this;

			decrefs();

			ref = a;
			numrefs = new unsigned(1);
			weakrefs = new unsigned(0);
			func = NULL;
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return *this;
		}

		template <typename T2>
		ref_ptr& operator = (T2* a)
		{
			if (ref == a)
				return *this;

			decrefs();

			ref = a;
			numrefs = new unsigned(1);
			weakrefs = new unsigned(0);
			func = NULL;
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return *this;
		}

		bool operator == (const ref_ptr<T>& a) const
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return ref == a.ref;
		}

		bool operator != (const ref_ptr<T>& a) const
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return ref != a.ref;
		}

		bool operator == (const T*& a) const
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return ref == a;
		}

		bool operator != (const T*& a) const
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return ref != a;
		}

		template <typename T2>
		bool operator == (const ref_ptr<T2>& a) const
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return ref == a.ref;
		}

		template <typename T2>
		bool operator != (const ref_ptr<T2>& a) const
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return ref != a.ref;
		}

		template <typename T2>
		bool operator == (const T2*& a) const
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return ref == a;
		}

		template <typename T2>
		bool operator != (const T2*& a) const
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return ref != a;
		}

		T& operator * () const throw()
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return *ref;
		}

		T* operator -> () const throw()
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return ref;
		}

		T& operator [] (unsigned i) const throw()
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return ref[i];
		}

		operator bool() const
		{
#ifdef REF_PTR_DEBUG
			checkref();
#endif
			return ref != 0;
		}

		template <typename T2>
		friend class ref_ptr;
		template <typename T2>
		friend class weak_ptr;

};

#ifdef REF_PTR_DEBUG
template <typename T>
std::map<T*, unsigned*> ref_ptr<T>::refs;
template <typename T>
std::map<T*, unsigned*> ref_ptr<T>::test;
#endif

template <typename T>
class enc_ptr
{
	private:
		T* ref;
		unsigned* numrefs;
		unsigned* weakrefs;
	public:
		
		enc_ptr() : ref(NULL), numrefs(NULL), weakrefs(NULL)
		{
		}

		template <typename T2>
		enc_ptr(const ref_ptr<T2>& a) : ref(a.ref), numrefs(a.numrefs), weakrefs(a.weakrefs)
		{
			(*weakrefs)++;
		}
		
		~enc_ptr()
		{
			(*weakrefs)--;
			if (*weakrefs == 0)
			{
				delete weakrefs;
				delete numrefs;
			}
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

		T operator * () const throw()
		{
			return *numrefs ? *ref : NULL;
		}

		T* operator -> () const throw()
		{
			return *numrefs ? ref : NULL;
		}

		T& operator [] (unsigned i) const throw()
		{
			return *numrefs ? ref[i] : T();
		}

		operator bool() const
		{
			return *numrefs ? ref != 0 : false;
		}
		
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

#endif
