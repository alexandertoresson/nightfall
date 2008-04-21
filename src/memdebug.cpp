#include <cstdlib> // for malloc() and free()
#include <map>
#include <iostream>
#include <fstream>
#include "memdebug.h"

#define RECORD_ALL

#define HASH_SIZE 3331337
#define HASH_FUNC(x) ((unsigned long)x % HASH_SIZE)

#define HASH_SIZE_BY_POS 1337

struct hash_entry_by_pos
{
	std::string file;
	unsigned line;
	unsigned num;
	size_t totalSize;
	hash_entry_by_pos *next;
	
	hash_entry_by_pos(std::string file, unsigned line, size_t totalSize)
	{
		this->file = file;
		this->line = line;
		this->totalSize = totalSize;
		this->num = 1;
		next = false;
	}
};

struct hash_entry
{
	void* pnt;
	size_t size;

#ifdef RECORD_ALL
	bool freed;
	std::string file;
	unsigned line;
#endif

	bool array_alloc;

	hash_entry* next;
	hash_entry_by_pos* pos_entry;

	hash_entry(void *p, size_t size, bool array_alloc)
	{
		this->pnt = p;
		this->size = size;
		this->array_alloc = array_alloc;
		freed = false;
	}
};

static hash_entry **hash_table;
/*static hash_entry_by_pos **hash_table_by_pos;

static void *alloc(size_t size, std::string file, unsigned line, bool array_alloc) throw(std::bad_alloc)
{
	void *p=malloc(size); 
	if (p==0) // did malloc succeed?
    		throw std::bad_alloc(); // ANSI/ISO compliant behavior

	if (!hash_table)
	{
		hash_table = (hash_entry**) calloc(HASH_SIZE, sizeof(hash_entry*));
	}

	if (!hash_table_by_pos)
	{
		hash_table_by_pos = (hash_entry_by_pos**) calloc(HASH_SIZE_BY_POS, sizeof(hash_entry_by_pos*));
	}

	hash_entry *hentry = NULL;

	{

		int h = HASH_FUNC(p);
		hentry = hash_table[h];

		hash_entry *last_entry = NULL;
		while (hentry)
		{
			if (hentry->pnt == p)
			{
				hentry->pnt = p;
				hentry->size = size;
				hentry->array_alloc = array_alloc;
				hentry->freed = false;
				break;
			}
			last_entry = hentry;
			hentry = hentry->next;
		}
		if (!hentry)
		{
			hentry = new (malloc(sizeof(hash_entry))) hash_entry(p, size, array_alloc);
			if (last_entry)
			{
				last_entry->next = hentry;
			}
			else
			{
				hash_table[h] = hentry;
			}
		}
	}

	{
		int hpos = line % HASH_SIZE_BY_POS;

		hash_entry_by_pos* pos_entry = hash_table_by_pos[hpos];
		hash_entry_by_pos* last_entry = NULL;
		bool found = false;

		while (pos_entry)
		{
			if (pos_entry->file == file && pos_entry->line == line)
			{
				pos_entry->num++;
				pos_entry->totalSize += size;
				hentry->pos_entry = pos_entry;
				found = true;
				break;
			}
			last_entry = pos_entry;
			pos_entry = pos_entry->next;
		}

		if (!found)
		{
			pos_entry = new (malloc(sizeof(hash_entry_by_pos))) hash_entry_by_pos(file, line, size);
			if (last_entry)
			{
				last_entry->next = pos_entry;
			}
			else
			{
				hash_table_by_pos[hpos] = pos_entry;
			}
			hentry->pos_entry = pos_entry;
		}
	}

 	return p;
}

static void release(void* p, std::string file, unsigned line, bool array_release) throw ()
{
	int h = HASH_FUNC(p);	
	hash_entry *curentry = hash_table[h];
	hash_entry *lastentry = NULL;
	bool found = false;

	while (curentry)
	{
		if (curentry->pnt == p)
		{
			found = true;
			if (curentry->array_alloc != array_release)
			{
		 		std::cout << "Error: Mismatched delete / delete[] of " << curentry->size << " bytes at " << file << ":" << line << std::endl;
		 		std::cout << "Allocated at  " << curentry->pos_entry->file << ":" << curentry->pos_entry->line << std::endl;
			}
			curentry->pos_entry->num--;
			curentry->pos_entry->totalSize -= curentry->size;
#ifdef RECORD_ALL
			if (curentry->freed)
			{
		 		std::cout << "Error: Attempt to free already freed memory " << p << " of " << curentry->size << " bytes at " << file << ":" << line << std::endl;
		 		std::cout << "Already freed at " << curentry->file << ":" << curentry->line << std::endl;
		 		return;
			}
			curentry->freed = true;
			curentry->file = file;
			curentry->line = line;
#else
			if (lastentry)
			{
				lastentry->next = curentry->next;
			}
			else
			{
				hash_table[h] = NULL;
			}
			free(curentry);
#endif
			break;
		}
		lastentry = curentry;
		curentry = curentry->next;
	}

	if (found)
	{
 		free(p); 
 	}
 	else
 	{
 		std::cout << "Error: Attempt to free not allocated memory " << p << " at " << file << ":" << line << std::endl;
 	}
}

void* operator new (size_t size, std::string file, unsigned line) throw(std::bad_alloc)
{
	return alloc(size, file, line, false);
}

void operator delete (void *p, std::string file, unsigned line) throw ()
{
	release(p, file, line, false);
}

void* operator new[] (size_t size, std::string file, unsigned line) throw(std::bad_alloc)
{
	return alloc(size, file, line, true);
}

void operator delete[] (void *p, std::string file, unsigned line) throw ()
{
	release(p, file, line, true);
}
*/
void WriteFragmentationReport(std::string filename)
{
	std::ofstream ofile(filename.c_str());
	for (int i = 0; i < HASH_SIZE; i++)
	{
		hash_entry *curentry = hash_table[i];
		while (curentry)
		{
			ofile << curentry->pnt << " " << curentry->size << std::endl;
			curentry = curentry->next;
		}
	}
	ofile.close();
}

void WriteLeakReport(std::string filename)
{
	std::ofstream ofile(filename.c_str());
}

