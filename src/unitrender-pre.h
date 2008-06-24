#ifndef __UNITRENDER_H_PRE__
#define __UNITRENDER_H_PRE__

#ifdef DEBUG_DEP
#warning "unitrender.h-pre"
#endif

#include "sdlheader.h"

namespace Game
{
	namespace Dimension
	{
		void SetParticleCoordSpace(float x, float y, float z, float scale = 1.0f);
		
		void InitRenderUnits();

		class UnitNode;
		class UnitSelectionNode;

		class ProjectileNode;
		class BuildOutlineNode;
	}
}

#endif
