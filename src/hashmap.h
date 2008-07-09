
int sizes[] = {3, 7, 17, 37};

template <typename _Key, typename _Tp >
class hash
{
	private:

	struct entry
	{
		_Key key;
		_Tp value;
		entry *next;

		entry(_Key key, _Tp value)
		{
			this->key = key;
			this->value = value;
			next = NULL;
		}
	};

	int num_entries;
	entry **entries;
	int load;

	public:

	hash(int i)
	{
		num_entries = i;
		load = 0;
		entries = new entry*[i];
		memset(entries, 0, sizeof(entry*) * i);
	}

	hash()
	{
		num_entries = sizes[0];
		load = 0;
		entries = new entry*[num_entries];
		memset(entries, 0, sizeof(entry*) * num_entries);
	}

	void empty()
	{
		for (int i = 0; i < num_entries; i++)
		{
			entry *cur_entry = entries[i];
			while (cur_entry)
			{
				entry *next = cur_entry->next;
				delete cur_entry;
				cur_entry = next;
			}
			entries[i] = NULL;
		}
	}

	~hash()
	{
		empty();
		delete[] entries;
	}

	int hash_func(_Key key)
	{
		return (unsigned long) key % num_entries;
	}

	void set(_Key key, _Tp value)
	{
		int hashval = hash_func(key);
		entry *cur_entry = entries[hashval], *last_entry = NULL;
		while (cur_entry)
		{
			if (cur_entry->key == key)
			{
				cur_entry->value = value;
				return;
			}
			last_entry = cur_entry;
			cur_entry = cur_entry->next;
		}
		entry *new_entry = new entry(key, value);
		load++;
		if (last_entry)
		{
			last_entry->next = new_entry;
		}
		else
		{
			entries[hashval] = new_entry;
		}
	}

	_Tp get(_Key key)
	{
		int hashval = hash_func(key);
		entry *cur_entry = entries[hashval], *last_entry = NULL;
		while (cur_entry)
		{
			if (cur_entry->key == key)
			{
				return cur_entry->value;
			}
			last_entry = cur_entry;
			cur_entry = cur_entry->next;
		}
		return (_Tp) 0;
	}
	
	void remove(_Key key)
	{
		int hashval = hash_func(key);
		entry *cur_entry = entries[hashval], *last_entry = NULL;
		while (cur_entry)
		{
			if (cur_entry->key == key)
			{
				if (last_entry)
				{
					last_entry->next = cur_entry->next;
				}
				else
				{
					entries[hashval] = cur_entry->next;
				}
				load--;
				delete cur_entry;
				return;
			}
			last_entry = cur_entry;
			cur_entry = cur_entry->next;
		}
	}

	int get_load()
	{
		return load;
	}

	int get_numentries()
	{
		return num_entries;
	}

	void repopulate(hash* new_hash)
	{
		for (int i = 0; i < num_entries; i++)
		{
			entry *cur_entry = entries[i];
			while (cur_entry)
			{
				new_hash->set(cur_entry->key, cur_entry->value);
				cur_entry = cur_entry->next;
			}
		}
	}
};

template <typename _Key, typename _Tp >
class hashmap
{
	private:
	
	hash<_Key, _Tp> *cur_hash;

	public:

	hashmap()
	{
		cur_hash = new hash<_Key, _Tp>();
	}

	void set(_Key key, _Tp value)
	{
		cur_hash->set(key, value);
		if (cur_hash->get_load() > cur_hash->get_numentries() * 0.75)
		{
			hash<_Key, _Tp> *new_hash = new hash<_Key, _Tp>(cur_hash->get_numentries() * 2);
			cur_hash->repopulate(new_hash);
			delete cur_hash;
			cur_hash = new_hash;
		}
	}
	
	_Tp get(_Key key)
	{
		return cur_hash->get(key);
	}
	
	void remove(_Key key)
	{
		cur_hash->remove(key);
	}

};

