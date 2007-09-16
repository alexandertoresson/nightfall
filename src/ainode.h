#ifndef __AINODE_H__
#define __AINODE_H__
namespace Game
{
	namespace AI
	{
		struct Node
		{
			Node* pParent;
			Node* pChild;

			int x;
			int y;
		};
	}
}
#endif

