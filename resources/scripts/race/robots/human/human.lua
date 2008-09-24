--{
-- Nightfall - Real-time strategy game
--
-- Copyright (c) 2008 Marcus Klang, Alexander Toresson and Leonard Wickmark
-- 
-- This file is part of Nightfall.
-- 
-- Nightfall is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
-- 
-- Nightfall is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
-- 
-- You should have received a copy of the GNU General Public License
-- along with Nightfall.  If not, see <http://www.gnu.org/licenses/>.
--}
loadfile(GetLUAScript("ai_generic.lua"))()

function InitAI()

	InitAI_Generic()

end

function PerformAI_Unit_Human(Unit, action)
	PerformAI_Unit_Generic(Unit, action)
end

function PerformAI_Player_Human()
--	Output(GetPowerAtDusk())
--	Output(" ")
--	Output(GetPowerAtDawn())
--	Output(" ")
--	Output(GetIncomeAtNoon())
--	Output(" ")
--	Output(GetIncomeAtNight())
--	Output(Console.Newline)
end

function CommandUnit_Human(Unit, target, x, y, action, argument, rotation)
	CommandUnit(Unit, target, x, y, action, argument, rotation)
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

