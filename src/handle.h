#ifndef __HANDLE_H__
#define __HANDLE_H__

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
				HasHandle(int id = -1) : handle(id)
				{
					HandleManager<T>::AssignHandle(gc_ptr_from_this<T>::GetWeak(), handle, independentHandle);
				}

				~HasHandle()
				{
					HandleManager<T>::RevokeHandle(handle);
				}

				int GetHandle()
				{
					return handle;
				}

				int GetIndependentHandle()
				{
					return independentHandle;
				}
		
				static gc_ptr<T> New()
				{
					return (new T)->GetRef();
				}

				static gc_ptr<T> New(int id)
				{
					return (new T(id))->GetRef();
				}
		};
	}
}

#endif
