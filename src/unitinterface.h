#ifndef __UNITINTERFACE_H__
#define __UNITINTERFACE_H__

#ifdef DEBUG_DEP
#warning "unitinterface.h"
#endif

#include "sdlheader.h"
#include "unitinterface-pre.h"

#include "luawrapper-pre.h"
#include "unit-pre.h"
#include "dimension-pre.h"
#include <string>

namespace UnitLuaInterface
{
	void Init(Utilities::Scripting::LuaVMState* pVM);
	gc_ptr<Game::Dimension::UnitType>& GetUnitTypeByID(const gc_ptr<Game::Dimension::Player>& owner, std::string str);
	gc_ptr<Game::Dimension::Research>& GetResearchByID(const gc_ptr<Game::Dimension::Player>& owner, std::string str);
}

#ifdef DEBUG_DEP
#warning "unitinterface.h-end"
#endif

#endif
