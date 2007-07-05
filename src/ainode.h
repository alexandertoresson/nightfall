#ifndef __AINODE_H__
#define __AINODE_H__
namespace Game
{
	namespace AI
	{
		class Node
		{
			public:
				Node* pParent;
				Node* pChild;

				float g;
				float h;
				float f;

				int x;
				int y;

				Node() : pParent(NULL), pChild(NULL), g(0), h(0), f(0), x(0), y(0)
				{}
				
				~Node()
				{}

				bool operator == (const Node& a) const
				{
					return a.x == x && a.y == y;
				}

				bool operator != (const Node& a) const
				{
					return a.x != x || a.y != y;
				}
		};
	}
}
#endif

