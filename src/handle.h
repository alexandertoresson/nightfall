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
#ifndef HANDLE_H
#define HANDLE_H

#include "research-pre.h"
#include "dimension-pre.h"
#include "networking.h"
#include <iostream>

namespace Game
{
	namespace Dimension
	{

		template <typename T>
		struct HandleTraits { };

		template <>
		struct HandleTraits<Unit>
		{
			enum { base = 0, num = 65536 };
		};

		// NOTE: The networking code assumes that all unit type and research handles subtracted by HandleTraits<UnitType>::base
		//       fits in a 16-bit uint.
		template <>
		struct HandleTraits<UnitType>
		{
			enum { base = 65536, num = 16384 };
		};
		
		template <>
		struct HandleTraits<Research>
		{
			enum { base = 81920, num = 16384 };
		};
		
		template <>
		struct HandleTraits<Player>
		{
			enum { base = 98304, num = 1024 };
		};

		template <typename T>
		class HandleManager
		{
			private:
				static gc_ptr<T> handles[HandleTraits<T>::num];
				static unsigned nextIndex;

			public:
				static void AssignHandle(gc_ptr<T> pnt, int& handle, int& independentHandle)
				{
					if (handle != -1)
					{
						int index = handle - HandleTraits<T>::base;
						if (index < 0 || index >= HandleTraits<T>::num)
						{
							std::cout << "Tried to assign invalid handle " << index << " of type " << typeid(T).name() << "!" << std::endl;
							handle = -1;
							independentHandle = -1;
							return;
						}
						handles[index] = pnt;
						independentHandle = index;
					}
					else
					{
						unsigned first = nextIndex;
						while (handles[nextIndex])
						{
							nextIndex++;
							if (nextIndex == HandleTraits<T>::num)
							{
								nextIndex = 0;
							}
							if (nextIndex == first)
							{
								std::cout << "Out of handles of type " << typeid(T).name() << "!" << std::endl;
								handle = -1;
								independentHandle = -1;
								return;
							}
						}
						handles[nextIndex] = pnt;
						independentHandle = nextIndex;
						handle = nextIndex + HandleTraits<T>::base;
					}
#ifdef CHECKSUM_DEBUG_HIGH
					Networking::checksum_output << "Assigned handle: " << handle << "\n";
#endif
				}

				static void RevokeHandle(int handle)
				{
					int index = handle - HandleTraits<T>::base;
					if (index < 0 || index >= HandleTraits<T>::num)
					{
						std::cout << "Tried to revoke invalid handle " << index << " of type " << typeid(T).name() << "!" << std::endl;
						return;
					}
					handles[index] = gc_ptr<T>();
#ifdef CHECKSUM_DEBUG_HIGH
					Networking::checksum_output << "Revoked handle: " << handle << "\n";
#endif
				}

				static bool IsCorrectHandle(int handle)
				{
					int index = handle - HandleTraits<T>::base;
					return index >= 0 && index < HandleTraits<T>::num;
				}

				static gc_ptr<T> InterpretHandle(int handle)
				{
					int index = handle - HandleTraits<T>::base;
					if (index < 0 || index >= HandleTraits<T>::num)
					{
						std::cout << "Tried to interpret invalid handle " << index << " of type " << typeid(T).name() << "!" << std::endl;
						return gc_ptr<T>();
					}
					return handles[index];
				}
				
				static gc_ptr<T> InterpretIndependentHandle(int independentHandle)
				{
					return InterpretHandle(independentHandle + HandleTraits<T>::base);
				}
		};
		
		template <typename T>
		unsigned HandleManager<T>::nextIndex = 0;

		template <typename T>
		gc_ptr<T> HandleManager<T>::handles[HandleTraits<T>::num];

		template <typename T>
		class HasHandle : public gc_ptr_from_this<T>
		{
			private:
				int handle;
				int independentHandle;

			public:
				HasHandle() : handle(-1), independentHandle(-1)
				{
				}

				~HasHandle()
				{
					RevokeHandle();
				}

				void AssignHandle(int id = -1)
				{
					handle = id;
					HandleManager<T>::AssignHandle(gc_ptr_from_this<T>::GetRef(), handle, independentHandle);
				}

				int GetHandle()
				{
					if (handle == -1)
					{
						std::cout << "Uninitialized handle for " << typeid(T).name() << std::endl;
					}
					return handle;
				}

				int GetIndependentHandle()
				{
					if (independentHandle == -1)
					{
						std::cout << "Uninitialized independent handle for " << typeid(T).name() << std::endl;
					}
					return independentHandle;
				}

				void RevokeHandle()
				{
					if (handle != -1)
					{
						HandleManager<T>::RevokeHandle(handle);
						handle = -1;
						independentHandle = -1;
					}
				}		
		};
	}
}

#endif
