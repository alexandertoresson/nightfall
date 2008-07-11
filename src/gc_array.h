#ifndef __GC_ARRAY_H__
#define __GC_ARRAY_H__

#include "gc_ptr.h"
#include <vector>

#define GC_ARRAY_DEBUG

template <typename T, int i, typename _Counter = gc_default_counter>
class gc_array
{
	private:
		typedef gc_array<T, i-1, _Counter> LowerType;
		gc_ptr<LowerType, _Counter> arr;
		unsigned length;

	public:
		gc_array() : arr(NULL), length(0)
		{
		}

		gc_array(unsigned j) : arr(new LowerType[j], array_deleter), length(j)
		{
		}

		gc_array(std::vector<unsigned> dims)
		{
			if (dims.size())
			{
				unsigned dim = dims[0];

				arr = gc_ptr<LowerType>(new LowerType[dim], array_deleter);
				length = dim;

				dims.erase(dims.begin());

				for (unsigned j = 0; j < dim; j++)
				{
					arr[j] = LowerType(dims);
				}
			}
		}

		gc_array(const gc_array& a) : arr(a.arr), length(a.length)
		{
		}

		LowerType& operator [] (unsigned j) const throw()
		{
			return arr[j];
		}

		gc_array& operator = (const gc_array& a)
		{
			arr = a.arr;
			length = a.length;
		}

		void shade()
		{
			arr.shade();
		}

};

template <typename T, typename _Counter>
class gc_array<T, 1, _Counter>
{
	private:
		gc_ptr<T, _Counter> arr;
		unsigned length;

	public:
		gc_array() : arr(NULL), length(0)
		{
		}

		gc_array(unsigned j) : arr(new T[j], array_deleter), length(j)
		{
		}

		gc_array(std::vector<unsigned> dims)
		{
			if (dims.size())
			{
				unsigned dim = dims[0];

				arr = gc_ptr<T>(new T[dim], array_deleter);
				length = dim;
			}
		}

		gc_array(const gc_array& a) : arr(a.arr), length(a.length)
		{
		}

		T& operator [] (unsigned j) const throw()
		{
			return arr[j];
		}

		gc_array& operator = (const gc_array& a)
		{
			arr = a.arr;
			length = a.length;
		}
	

		void shade()
		{
			arr.shade();
		}
};

template <typename T, typename _Counter>
class gc_array<T, 0, _Counter>
{
	// Perhaps this could error out in some more obvious way?
};

template <typename T, int i>
class gc_root_array
{
	typedef gc_array<T, i, gc_root_counter> type;
};

#endif
