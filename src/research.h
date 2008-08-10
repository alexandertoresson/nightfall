#ifndef __RESEARCH_H__
#define __RESEARCH_H__

#include "handle.h"
#include "requirements.h"

namespace Game
{
	namespace Dimension
	{
		
		struct Research : public HasHandle<Research>
		{
			std::string id;
			std::string name;
			std::string description;
			bool isResearched;
			gc_ptr<Unit> researcher;
			std::string luaEffectObj;
			ObjectRequirements requirements;
			GLuint icon;
			gc_ptr<Player> player;

			Research()
			{
				researcher = NULL;
			}

			void shade()
			{
				researcher.shade();
				player.shade();
			}
		};
	}
}

#endif
