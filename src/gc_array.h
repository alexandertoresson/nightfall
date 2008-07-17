#ifndef __GC_ARRAY_H__
#define __GC_ARRAY_H__

#include "gc_ptr.h"
#include "type_traits.h"
#include <vector>

//#define GC_ARRAY_DEBUG

template <typename T, bool U = is_arithmetic<T>::value >
struct default_array_initializer
{
	T generate(std::vector<unsigned> updims)
	{
		return T();
	}
};

template <typename T>
struct default_array_initializer<T, true>
{
	T generate(std::vector<unsigned> updims)
	{
		return 0;
	}
};

template <typename T, int i, typename _Counter = gc_default_counter>
class gc_array
{
	private:
		typedef gc_array<T, i-1, gc_default_counter> LowerType;
		typedef gc_ptr<LowerType> PtrType;
		typedef gc_array<T, i, _Counter> ThisType;
		PtrType arr;
		unsigned length;

	public:
		gc_array() : arr(NULL), length(0)
		{
		}

		gc_array(unsigned j) : arr(new LowerType[j], array_deleter), length(j)
		{
		}

		template <typename U>
		gc_array(std::vector<unsigned> dims, U ai = default_array_initializer<T>(), std::vector<unsigned> updims = std::vector<unsigned>())
		{
			if (dims.size())
			{
				unsigned dim = dims[0];

				arr = gc_ptr<LowerType>(new LowerType[dim], array_deleter);
				length = dim;

				dims.erase(dims.begin());

				for (unsigned j = 0; j < dim; j++)
				{
					updims.push_back(j);
					arr[j] = LowerType(dims, ai, updims);
					updims.pop_back();
				}
			}
		}

		gc_array(std::vector<unsigned> dims)
		{
			default_array_initializer<T> ai;
			std::vector<unsigned> updims;
			if (dims.size())
			{
				unsigned dim = dims[0];

				arr = gc_ptr<LowerType>(new LowerType[dim], array_deleter);
				length = dim;

				dims.erase(dims.begin());

				for (unsigned j = 0; j < dim; j++)
				{
					updims.push_back(j);
					arr[j] = LowerType(dims, ai, updims);
					updims.pop_back();
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
			return *this;
		}

		gc_array& operator = (unsigned j)
		{
			*this = ThisType(j);
			return *this;
		}

		gc_array& operator = (std::vector<unsigned> dims)
		{
			*this = ThisType(dims);
			return *this;
		}

		unsigned size()
		{
			return length;
		}

		void shade()
		{
			arr.shade();
			for (unsigned j = 0; j < length; j++)
			{
				arr[j].shade();
			}
		}

};

template <typename T, typename _Counter>
class gc_array<T, 1, _Counter>
{
	private:
		typedef gc_ptr<T, _Counter> PtrType;
		typedef gc_array<T, 1, _Counter> ThisType;
		PtrType arr;
		unsigned length;

	public:
		gc_array() : arr(NULL), length(0)
		{
		}

		gc_array(unsigned j) : arr(new T[j], array_deleter), length(j)
		{
		}

		template <typename U>
		gc_array(std::vector<unsigned> dims, U ai = default_array_initializer<T>(), std::vector<unsigned> updims = std::vector<unsigned>())
		{
			if (dims.size())
			{
				unsigned dim = dims[0];

				arr = gc_ptr<T>(new T[dim], array_deleter);
				length = dim;

				for (unsigned j = 0; j < dim; j++)
				{
					updims.push_back(j);
					arr[j] = ai.generate(updims);
					updims.pop_back();
				}
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
			return *this;
		}
	
		gc_array& operator = (unsigned j)
		{
			*this = ThisType(j);
			return *this;
		}

		unsigned size()
		{
			return length;
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