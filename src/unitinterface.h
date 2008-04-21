#ifndef __UNITINTERFACE_H__
#define __UNITINTERFACE_H__

#ifdef DEBUG_DEP
#warning "unitinterface.h"
#endif

#include "unitinterface-pre.h"

#include "luawrapper-pre.h"
#include "unit-pre.h"
#include "dimension-pre.h"
#include <string>

namespace UnitLuaInterface
{
	void Init(Utilities::Scripting::LuaVMState* pVM);
	bool IsValidUnitTypePointer(Game::Dimension::UnitType* unittype);
	bool IsValidResearchPointer(Game::Dimension::Research* research);
	Game::Dimension::UnitType *GetUnitTypeByID(Game::Dimension::Player* owner, std::string str);
	Game::Dimension::Research *GetResearchByID(Game::Dimension::Player* owner, std::string str);
}

#ifdef DEBUG_DEP
#warning "unitinterface.h-end"
#endif

#endif
