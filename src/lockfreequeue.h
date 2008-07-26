
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
	};

	gc_ptr<entry> tail, head;

	void produce(gc_ptr<T> a)
	{
		gc_ptr<entry> new_entry = new entry(a);
		head->next = new_entry;
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

		gc_ptr<entry> ret = tail->a;

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
};

