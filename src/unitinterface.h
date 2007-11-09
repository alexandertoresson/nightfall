#ifndef __UNITINTERFACE_H_PRE__
#define __UNITINTERFACE_H_PRE__

#ifdef DEBUG_DEP
#warning "unitinterface.h-pre"
#endif

namespace UnitLuaInterface
{
	enum EventType
	{
		EVENTTYPE_COMMANDCOMPLETED,
		EVENTTYPE_COMMANDCANCELLED,
		EVENTTYPE_NEWCOMMAND,
		EVENTTYPE_BECOMEIDLE,
		EVENTTYPE_ISATTACKED,
		EVENTTYPE_PERFORMUNITAI,
		EVENTTYPE_UNITCREATION,
		EVENTTYPE_UNITKILLED,
		EVENTTYPE_PERFORMPLAYERAI
	};
	void ApplyScheduledActions();
}
#define __UNITINTERFACE_H_PRE_END__

#include "luawrapper.h"
#include "unit.h"
#include "networking.h"

#endif

#ifdef __NETWORKING_H_END__

#ifdef __UNIT_H_END__

#ifdef __LUAWRAPPER_H_END__

#ifndef __UNITINTERFACE_H__
#define __UNITINTERFACE_H__

#ifdef DEBUG_DEP
#warning "unitinterface.h"
#endif

namespace UnitLuaInterface
{
	void Init(Utilities::Scripting::LuaVirtualMachine* pVM);
	bool IsValidUnitTypePointer(Game::Dimension::UnitType* unittype);
	Game::Dimension::UnitType *GetUnitTypeByID(std::string str);
}

#ifdef DEBUG_DEP
#warning "unitinterface.h-end"
#endif

#define __UNITINTERFACE_H_END__

#include <sstream>

#endif

#endif

#endif

#endif
