loadfile(GetLUAScript("ai_generic.lua"))()

function InitAI_Human(Player)

	InitAI_Generic(Player)

end

function PerformAI_Unit_Human(Unit, action)
	PerformAI_Unit_Generic(Unit, action)
end

function PerformAI_Player_Human(Player)
--	Output(GetPowerAtDusk(Player))
--	Output(" ")
--	Output(GetPowerAtDawn(Player))
--	Output(" ")
--	Output(GetIncomeAtNoon(Player))
--	Output(" ")
--	Output(GetIncomeAtNight(Player))
--	Output(Console.Newline)
end

function CommandUnit_TargetPos_Human(Unit, x, y, action, argument)
	CommandUnit_TargetPos(Unit, x, y, action, argument)
end

function CommandUnit_TargetUnit_Human(Unit, target, action, argument)
	CommandUnit_TargetUnit(Unit, target, action, argument)
end

function UnitEvent_UnitKilled_Human(Unit)
end

function UnitEvent_UnitCreation_Human(Unit)
end

function UnitEvent_BecomeIdle_Human(Unit)
	UnitEvent_BecomeIdle_Generic(Unit)
end

function UnitEvent_CommandCompleted_Human(Unit, action, x, y, goal, arg)
end

function UnitEvent_CommandCancelled_Human(Unit, action, x, y, goal, arg)
end

function UnitEvent_NewCommand_Human(Unit, action, x, y, goal, arg)
	UnitEvent_NewCommand_Generic(Unit, action, x, y, goal, arg)
end

function UnitEvent_IsAttacked_Human(Unit, attacker)
end

function UnitEvent_IsAttacked_Human(Unit, attacker)
	if GetUnitType(attacker) == GetUnitTypeFromString("Grue") then
		return
	end
	if GetUnitCanAttack(Unit) and not (GetUnitType(Unit) == GetUnitTypeFromString("Builder")) then
		CommandAttack(Unit, attacker)
	end
end

