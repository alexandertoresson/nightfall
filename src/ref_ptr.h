#ifndef __REF_PTR_H__
#define __REF_PTR_H__

template <typename T>
class ref_ptr
{
	private:
		T* ref;
		int* numrefs;

		void decrefs()
		{
			(*numrefs)--;
			if (*numrefs == 0)
			{
				if (ref)
					delete ref;
				delete numrefs;
				ref = NULL;
				numrefs = NULL;
			}
		}

		void increfs() const
		{
			(*numrefs)++;
		}

	public:
		ref_ptr() : ref(NULL), numrefs(new int(1))
		{
		}

		ref_ptr(const ref_ptr& a) : ref(a.ref), numrefs(a.numrefs)
		{
			increfs();
		}

		ref_ptr(T* a) : ref(a), numrefs(new int(1))
		{
		}

		~ref_ptr()
		{
			decrefs();
		}

		ref_ptr& operator = (const ref_ptr& a)
		{
			a.increfs();
			
			decrefs();

			ref = a.ref;
			numrefs = a.numrefs;
			return *this;
		}

		ref_ptr& operator = (T* a)
		{
			decrefs();

			ref = a;
			numrefs = new int(1);
			return *this;
		}

		bool operator == (const ref_ptr& a) const
		{
			return ref == a.ref;
		}

		bool operator != (const ref_ptr& a) const
		{
			return ref != a.ref;
		}

		T operator * () const throw()
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

};

template <typename T>
class ref_ptr_array
{
	private:
		T* ref;
		int* numrefs;

		void decrefs()
		{
			(*numrefs)--;
			if (*numrefs == 0)
			{
				if (ref)
					delete[] ref;
				delete numrefs;
				ref = NULL;
				numrefs = NULL;
			}
		}

		void increfs() const
		{
			(*numrefs)++;
		}

	public:
		ref_ptr_array() : ref(NULL), numrefs(new int(1))
		{
		}

		ref_ptr_array(const ref_ptr_array& a) : ref(a.ref), numrefs(a.numrefs)
		{
			increfs();
		}

		ref_ptr_array(T* a) : ref(a), numrefs(new int(1))
		{
		}

		~ref_ptr_array()
		{
			decrefs();
		}

		ref_ptr_array& operator = (const ref_ptr_array& a)
		{
			a.increfs();
			
			decrefs();

			ref = a.ref;
			numrefs = a.numrefs;
			return *this;
		}

		ref_ptr_array& operator = (T* a)
		{
			decrefs();

			ref = a;
			numrefs = new int(1);
			return *this;
		}

		bool operator == (const ref_ptr_array& a) const
		{
			return ref == a.ref;
		}

		bool operator != (const ref_ptr_array& a) const
		{
			return ref != a.ref;
		}

		T operator * () const throw()
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

};

#endif
