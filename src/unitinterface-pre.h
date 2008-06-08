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
	
	enum AIContexts
	{
		AI_CONTEXT_UNIT,
		AI_CONTEXT_UNITTYPE,
		AI_CONTEXT_PLAYER
	};
	
	void ApplyScheduledActions();
	void ApplyScheduledDamagings();
	void PostProcessStrings();
}

#endif

