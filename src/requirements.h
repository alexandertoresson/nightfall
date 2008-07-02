#ifndef __REQUIREMENTS_H__
#define __REQUIREMENTS_H__

#ifdef DEBUG_DEP
#warning "requirements.h"
#endif

#include "requirements-pre.h"

#include "dimension-pre.h"

namespace Game
{
	namespace Dimension
	{		

		struct UnitRequirement
		{
			ref_ptr<UnitType> type;
			int minExisting, maxExisting;
			int minBuilt, maxBuilt;
		};

		struct ResearchRequirement
		{
			ref_ptr<Research> research;
			bool desiredState;
		};

		struct ConjunctiveRequirements
		{
			std::vector<ResearchRequirement> researchs;
			std::vector<UnitRequirement> units;
			std::string cReqString;
			bool isSatisfied;
			
			ConjunctiveRequirements()
			{
				isSatisfied = false;
			}
		};

		struct ObjectRequirements
		{
			ConjunctiveRequirements creation;
			ConjunctiveRequirements existance;
			int time;
			int money;
			int power;
		};
	}
}

#ifdef DEBUG_DEP
#warning "requirements.h-end"
#endif

#endif
