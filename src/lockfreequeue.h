#ifndef __LOCKFREEQUEUE_H__
#define __LOCKFREEQUEUE_H__

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
