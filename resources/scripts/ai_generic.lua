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

function table_maxn(table)
	i = 1
	while table[i] ~= nil do
		i = i + 1
	end
	return i-1
end

LastCheckForTarget = {}
LastCommands = {}

function GetAIFPS_Cached()
	if AIFPS == nil then
		AIFPS = GetAIFPS();
	end
	return AIFPS
end

function InitAI_Generic()
	SetRegularAIEnabled(GetUnitTypeFromString("Builder"), EventType.PerformUnitAI, false, AIContext.UnitType)
	SetRegularAIEnabled(GetUnitTypeFromString("MainBuilding"), EventType.PerformUnitAI, false, AIContext.UnitType)
	SetRegularAIEnabled(GetUnitTypeFromString("SolarPanel"), EventType.PerformUnitAI, false, AIContext.UnitType)
	SetRegularAIEnabled(GetUnitTypeFromString("SurfaceGeothermal"), EventType.PerformUnitAI, false, AIContext.UnitType)
	SetRegularAIEnabled(GetUnitTypeFromString("DeepGeothermal"), EventType.PerformUnitAI, false, AIContext.UnitType)
	SetRegularAIEnabled(GetUnitTypeFromString("SmallLightTower"), EventType.PerformUnitAI, false, AIContext.UnitType)
	SetRegularAIEnabled(GetUnitTypeFromString("MediumLightTower"), EventType.PerformUnitAI, false, AIContext.UnitType)
	SetRegularAIEnabled(GetUnitTypeFromString("LargeLightTower"), EventType.PerformUnitAI, false, AIContext.UnitType)
	SetRegularAIEnabled(GetUnitTypeFromString("Barracks"), EventType.PerformUnitAI, false, AIContext.UnitType)
	SetRegularAIEnabled(GetUnitTypeFromString("TankFactory"), EventType.PerformUnitAI, false, AIContext.UnitType)
	SetRegularAIEnabled(GetUnitTypeFromString("Explorer"), EventType.PerformUnitAI, false, AIContext.UnitType)
end

function PerformAI_Unit_Generic(Unit, action)

	if action == UnitAction.None and (LastCheckForTarget[Unit] == nil or GetCurrentFrame() - LastCheckForTarget[Unit] > GetAIFPS() * 0.1) and GetUnitCanAttack(Unit) then
		targetUnit = GetNearestUnitInRange(Unit, RangeType.Sight, PlayerState.Enemy)
		LastCheckForTarget[Unit] = GetCurrentFrame()
		if IsNonNull(targetUnit) then
			CommandUnit(Unit, targetUnit, 0, 0, UnitAction.Attack)
		end
		action = GetUnitAction(Unit);
	end

--	if action == UnitAction.Goto or action == UnitAction.Follow or action == UnitAction.Attack or action == UnitAction.Build then
--	
--		if GetUnitCanAttackWhileMoving(Unit) and (LastCommands[Unit] == nil or os.difftime(os.time(), LastCheckForTarget[Unit]) > 0.1) and GetUnitCanAttack(Unit) then
--			LastCheckForTarget[Unit] = os.time()
--			if action == UnitAction.Attack then
--				targetUnit = GetUnitTargetUnit(Unit);
--			else
--				targetUnit = Null()
--			end
--			if not IsNonNull(targetUnit) then
--				targetUnit = GetNearestUnitInRange(Unit, RangeType.Sight, PlayerState.Enemy)
--			end
--			if IsNonNull(targetUnit) then
--				InitiateAttack(Unit, targetUnit)
--			end
--		end
--	end
end

function UnitEvent_BecomeIdle_Generic(Unit)
	SetRegularAIEnabled(Unit, EventType.PerformUnitAI, true, AIContext.Unit)
end

function UnitEvent_NewCommand_Generic(Unit, action, x, y, goal, arg)
	SetRegularAIEnabled(Unit, EventType.PerformUnitAI, false, AIContext.Unit)
end

