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
#ifndef LOCKFREEQUEUE_H
#define LOCKFREEQUEUE_H

#include <iterator>

template <typename T>
class lockfreequeue
{
		struct entry
		{
			T a;
			entry* next;

			entry(T a = T()) : a(a), next(NULL)
			{
				
			}
		};

		entry* tail;
		entry* head;

	public:

		void produce(T a)
		{
			entry* new_entry = new entry(a);
			head->next = new_entry;
			head = new_entry;
		}

		T consume()
		{
			if (!tail->a)
			{
				if (tail->next)
				{
					entry* old = tail;
					tail = tail->next;
					delete old;
				}
				else
				{
					return T();
				}
			}

			T ret = tail->a;

			if (tail->next)
			{
				entry* old = tail;
				tail = tail->next;
				delete old;
			}
			else
			{
				tail->a = T();
			}
			return ret;
		}

		lockfreequeue()
		{
			head = tail = new entry();
		}

		class const_iterator : public std::iterator<std::forward_iterator_tag, T>
		{
			private:
				entry* t;
			public:
				const_iterator(entry* t) : t(t) {}
				~const_iterator() {}

				const_iterator& operator = (const const_iterator& other)
				{
					t = other.t;
					return *this;
				}

				bool operator == (const const_iterator& other)
				{
					return t == other.t;
				}

				bool operator != (const const_iterator& other)
				{
					return !(*this == other);
				}

				const_iterator& operator ++ ()
				{
					if (t != NULL)
					{
						t = t->next;
						if (t && !t->a)
						{
							t = NULL;
						}
					}
					return *this;
				}
				
				const_iterator operator ++ (int)
				{
					const_iterator tmp(*this);
					++(*this);
					return tmp;
				}

				const T& operator * ()
				{
					return t->a;
				}

				const T* operator -> ()
				{
					return &t->a;
				}
		};

		const_iterator begin() const
		{
			return const_iterator(tail->a ? tail :  tail->next);
		}
		
		const_iterator end() const
		{
			return const_iterator(NULL);
		}
};

#endif
