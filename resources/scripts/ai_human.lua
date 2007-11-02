loadfile(GetLUAScript("ai_generic.lua"))()

function PerformAI_Unit_Human(Unit)
	PerformAI_Unit_Generic(Unit)
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

function UnitEvent_BuildComplete_Human(Unit, UnitType, TargetUnit)
end

function UnitEvent_BuildCancelled_Human(Unit, UnitType, TargetUnit)
end

function UnitEvent_ResearchComplete_Human(Unit, UnitType, TargetUnit)
end

function UnitEvent_ResearchCancelled_Human(Unit, UnitType, TargetUnit)
end

function UnitEvent_IsAttacked_Human(Unit, attacker)
	if GetUnitType(attacker) == GetUnitTypeFromString("Grue") then
		return
	end
	if GetUnitCanAttack(Unit) and not (GetUnitType(Unit) == GetUnitTypeFromString("Builder")) then
		CommandAttack(Unit, attacker)
	end
end

