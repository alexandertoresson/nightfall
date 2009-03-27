/*
 * Nightfall - Real-time strategy game
 *
 * Copyright (c) 2008 Marcus Klang, Alexander Toresson and Leonard Wickmark
 * 
 * This file is part of Nightfall.
 * 
 * Nightfall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nightfall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nightfall.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GC_ARRAY_H
#define GC_ARRAY_H

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

template <typename T, bool U = is_arithmetic<T>::value >
struct default_array_shader
{
	static void shade(T* arr, unsigned length)
	{
	}
};

template <typename T>
struct default_array_shader<gc_ptr<T>, false>
{
	static void shade(T* arr, unsigned length)
	{
		for (unsigned i = 0; i < length; i++)
		{
			if (arr[i])
			{
				arr[i].shade();
			}
		}
	}
};

template <typename T>
struct default_array_shader<T, true>
{
	static void shade(T* arr, unsigned length)
	{
	}
};

template <typename T, int i, typename _Counter = gc_default_counter, typename _Shader = default_array_shader<T> >
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

template <typename T, typename _Counter, typename _Shader>
class gc_array<T, 1, _Counter, _Shader>
{
	private:
		typedef T* PtrType;
		typedef gc_array<T, 1, _Counter> ThisType;
		PtrType arr;
		unsigned length;

	public:
		gc_array() : arr(NULL), length(0)
		{
		}

		gc_array(unsigned j) : arr(new T[j]), length(j)
		{
		}

		template <typename U>
		gc_array(std::vector<unsigned> dims, U ai = default_array_initializer<T>(), std::vector<unsigned> updims = std::vector<unsigned>())
		{
			if (dims.size())
			{
				unsigned dim = dims[0];

				arr = new T[dim];
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
			_Shader::shade(arr, length);
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
