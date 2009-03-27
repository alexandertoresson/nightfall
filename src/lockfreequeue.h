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

#include "gc_ptr.h"

template <typename T>
class lockfreequeue
{
		struct entry
		{
			gc_ptr<T> a;
			gc_ptr<entry> next;

			entry(gc_ptr<T> a) : a(a)
			{
				
			}

			void shade()
			{
				a.shade();
				next.shade();
			}
		};

		gc_ptr<entry> tail, head;

	public:

		void produce(gc_ptr<T> a)
		{
			gc_ptr<entry> new_entry = new entry(a);
			head->next = new_entry;
			head = new_entry;
		}

		gc_ptr<T> consume()
		{
			if (!tail->a)
			{
				if (tail->next)
				{
					tail = tail->next;
				}
				else
				{
					return NULL;
				}
			}

			gc_ptr<T> ret = tail->a;

			if (tail->next)
			{
				tail = tail->next;
			}
			else
			{
				tail->a = NULL;
			}
			return ret;
		}

		lockfreequeue()
		{
			head = tail = new entry(NULL);
		}

		void shade()
		{
			tail.shade();
			head.shade();
		}
};

#endif
