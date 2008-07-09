#ifndef __UNITTYPE_H_PRE__
#define __UNITTYPE_H_PRE__

#ifdef DEBUG_DEP
#warning "unittype.h-pre"
#endif

#include "sdlheader.h"

namespace Game
{
	namespace Dimension
	{
		struct Scanline;
		struct RangeScanlines;
		struct RangeArray;

		gc_ptr<RangeArray> GenerateRangeArray(float maxrange, float minrange);

		struct UnitType;

		struct ProjectileType;

		struct MorphAnim;
		struct TransformAnim;
		struct TransformData;
		struct Animation;
		
		gc_ptr<UnitType> GetUnitTypeByID(unsigned i);
	}
}

#endif

