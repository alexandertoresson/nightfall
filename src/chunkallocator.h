#include <stack>

namespace Utilities
{

	template <typename T> class ChunkAllocator
	{
		private:
		std::stack<T*> chunks;
		T* curChunk;
		unsigned itemsPerChunk;
		unsigned itemsHandedOut;

		void NewChunk()
		{
			curChunk = new T[itemsPerChunk];
			chunks.push(curChunk);
			itemsHandedOut = 0;
		}

		public:

		ChunkAllocator(unsigned itemsPerChunk)
		{
			this->itemsPerChunk = itemsPerChunk;
			itemsHandedOut = itemsPerChunk;
			curChunk = NULL;
		}

		T* New()
		{
			if (itemsHandedOut == itemsPerChunk)
			{
				NewChunk();
			}
			return &curChunk[itemsHandedOut++];
		}

		void PutBack()
		{
			if (itemsHandedOut > 0)
			{
				itemsHandedOut--;
			}
		}

		void DeallocChunks()
		{
			while (chunks.size())
			{
				delete[] chunks.top();
				chunks.pop();
			}
			curChunk = NULL;
			itemsHandedOut = itemsPerChunk;
		}

		~ChunkAllocator()
		{
			DeallocChunks();
		}
	};
}

