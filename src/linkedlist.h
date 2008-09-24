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
#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <cstdlib> // is needed to get NULL defined on linux

namespace Utilities
{
	template <typename T>
	struct Node
	{
		T value;
		Node<T>* pNext;
		Node<T>* pPrevious;
	};

	template <typename T>
	class LinkedList
	{
		private:
			bool dealloc;
			int count;
			void (*fptrOnDealloc)(Node<T>*);

		public:
			Node<T>* pHead;
			Node<T>* pTail;

			LinkedList(bool _deallocNodesAtDeconstruction = true, void (*fptr)(Node<T>*) = NULL)
			{
				dealloc = _deallocNodesAtDeconstruction;
				SetDeallocFunc(fptr);

				pHead = pTail = NULL;
				count = 0;
			}

			~LinkedList()
			{
				if (count == 0)
					return;

				if (dealloc)
				{
					if (pHead != NULL)
					{
						if (fptrOnDealloc != NULL)
							fptrOnDealloc(&*pHead);

						if (pHead == NULL)
							return;
						
						Node<T>* pNode = pHead;
						Node<T>* pDelete = NULL;
						do 
						{
							pDelete = pNode;
							pNode = pNode->pNext;
							delete pDelete;
							pDelete = NULL;
						} while (pNode != NULL);

						pHead = NULL;
						pTail = NULL;
					}
				}
			}

			void SetDeallocFunc(void (*fptr)(Node<T>*))
			{
				fptrOnDealloc = fptr;
			}


			Node<T>* Add(T val)
			{
				Node<T>* pItem = new Node<T>;
				pItem->value = val;
				pItem->pNext = NULL;
				pItem->pPrevious = NULL;

				if (pHead == NULL)
				{
					pHead = pTail = pItem;
				}
				else
				{
					pTail->pNext = pItem;
					pItem->pPrevious = pTail;

					pTail = pItem;
				}
				count++;

				return pItem;
			}

			void Remove(Node<T>* pItem, bool _dealloc)
			{
				if (pHead == NULL)
					return;
					
				if (pItem == NULL)
					return;
					
				if (pHead == pItem)
				{
					if (pTail == pItem)
					{
						pHead = NULL;
						pTail = NULL;
					}
					else
					{
						pHead = pItem->pNext;
						pHead->pPrevious = NULL;
					}
				}
				else if (pTail == pItem)
				{
					pTail = pItem->pPrevious;
					pTail->pNext = NULL;
				}
				else
				{
					pItem->pPrevious->pNext = pItem->pNext;
					pItem->pNext->pPrevious = pItem->pPrevious;
				}

				if (_dealloc)
				{
					delete pItem;
					pItem = NULL;
				}

				count--;
			}
			
			Node<T>* At(int idx)
			{
				if (idx > count)
					return NULL;

				Node<T>* pNode = pHead;
				for (int i = 0; i < idx; i++)
					pNode = pNode->pNext;

				return pNode;
			}

			unsigned int Length()
			{
				return count;
			}
	};

	template<typename T>
	class LinkedListIterator
	{
		private:
			LinkedList<T>* pSource;
			Node<T>* pCurrent;

		public:
			LinkedListIterator(LinkedList<T>* source)
			{
				pSource = source;
				pCurrent = source->pHead;
			}

			T Step()
			{
				if (pCurrent == NULL)
					return NULL;

				T value = pCurrent->value;
				pCurrent = pCurrent->pNext;

				return value;
			}
	};
}

#endif

