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

ToBuild = {}
NumBuilt = {}

TempNightIncomeChanges = 0
TempNoonIncomeChanges = 0
TempMoneyReserved = 0
Need = {}
NeedForPower = {}
NeedForSurvival = {}
LastCommands = {}

function GetPowerAtDawnCached()
	if Cached_PowerAtDawn == nil then
		Cached_PowerAtDawn = GetPowerAtDawn()
	end
	return Cached_PowerAtDawn
end

function GetPowerAtDuskCached()
	if Cached_PowerAtDusk == nil then
		Cached_PowerAtDusk = GetPowerAtDusk()
	end
	return Cached_PowerAtDusk
end

function GetIncomeAtNoonCached()
	if Cached_IncomeAtNoon == nil then
		Cached_IncomeAtNoon = GetIncomeAtNoon()
	end
	return Cached_IncomeAtNoon
end

function GetIncomeAtNightCached()
	if Cached_IncomeAtNight == nil then
		Cached_IncomeAtNight = GetIncomeAtNight()
	end
	return Cached_IncomeAtNight
end

function PowerQuote()
	if GetIncomeAtNightCached() >= 0 then
		return 10.0
	else
		return (GetIncomeAtNoonCached()) / -(GetIncomeAtNightCached())
	end
end

function ShouldBuild(UnitType)
	if IsResearched(UnitType) then
		if NeedForPower[UnitType] then
			return true
		end
		if (GetPowerAtDawnCached() - GetUnitTypeBuildCost(UnitType) - TempMoneyReserved > 500) then
			if GetIncomeAtNightCached() + GetUnitTypeIncomeAtNight(UnitType) + TempNightIncomeChanges >= 0 then
				return true
			else
				return (GetIncomeAtNoonCached() + TempNoonIncomeChanges + GetUnitTypeIncomeAtNoon(UnitType)) / -(GetIncomeAtNightCached() + TempNightIncomeChanges + GetUnitTypeIncomeAtNight(UnitType)) > 1.3
			end
		else
			return false
		end
	else
		return GetPowerAtDawnCached() - GetResearchCost(GetUnitTypeResearch(UnitType)) - TempMoneyReserved > 500
	end
end

function IncNumBuilt(UnitType)
	if NumBuilt[UnitType] == nil then
		NumBuilt[UnitType] = 1
	else
		NumBuilt[UnitType] = NumBuilt[UnitType] + 1
	end
end

function DecNumBuilt(UnitType)
	if NumBuilt[UnitType] == nil then
		return
	end
	NumBuilt[UnitType] = NumBuilt[UnitType] - 1
	if NumBuilt[UnitType] == 0 then
		NumBuilt[UnitType] = nil
	end
end

function GetNumBuilt(UnitType)
	if NumBuilt[UnitType] == nil then
		return 0
	else
		return NumBuilt[UnitType]
	end
end

function AppendToBuildList(UnitType)
--	Output("Append " .. GetUnitTypeName(UnitType) .. "\n")
	table.insert(ToBuild, UnitType)
end

function PrependToBuildList(UnitType)
	table.insert(ToBuild, 1, UnitType)
end

function SendBackFirstInQueue()
	UnitType = ToBuild[1]
	table.remove(ToBuild, 1)
	table.insert(ToBuild, UnitType)
end

BuildList = {}

Builders = {}
Researchers = {}

BuildRequirements = {}
ResearchRequirements = {}

IdleList = {}
CheckedIdleList = {}
AvailableBuilders = {}
CheckedBuilders = {}

function GetResearcherCached(Research)
	if Researchers[Research] == nil then
		Researchers[Research] = GetResearcher(Research)
	end
	return Researchers[Research]
end

function GetBuilderCached(UnitType)
	if Builders[UnitType] == nil then
		Builders[UnitType] = GetBuilder(UnitType)
	end
	return Builders[UnitType]
end

function GetResearchReqsCached(Research)
	if ResearchRequirements[Research] == nil then
		ResearchRequirements[Research] = GetResearchRequirements(Research)
	end
	return ResearchRequirements[Research]
end

function GetBuildReqsCached(UnitType)
	if BuildRequirements[UnitType] == nil then
		BuildRequirements[UnitType] = GetUnitTypeRequirements(UnitType)
	end
	return BuildRequirements[UnitType]
end

function GetUnitTypeResearch(UnitType)
	return GetBuildReqsCached(UnitType)["researchs"]["1"]["research"]
end

function SetUnitAvailableForBuilding(Unit, UnitType)
	if AvailableBuilders[UnitType] == nil then
		AvailableBuilders[UnitType] = {}
	end
	AvailableBuilders[UnitType][Unit] = true
	AvailableBuilders[Unit] = true
end

function SetUnitBuilding(Unit, UnitType)
	if not (AvailableBuilders[UnitType] == nil) then
		AvailableBuilders[UnitType][Unit] = nil
	end
	if not (CheckedBuilders[UnitType] == nil) then
		CheckedBuilders[UnitType][Unit] = nil
	end
	AvailableBuilders[Unit] = nil
end

function MoveUnitToChecked(Unit, UnitType)
	if not (AvailableBuilders[UnitType] == nil) then
		AvailableBuilders[UnitType][Unit] = nil
	end
	if CheckedBuilders[UnitType] == nil then
		CheckedBuilders[UnitType] = {}
	end
	CheckedBuilders[UnitType][Unit] = true
end

function MoveBackChecked(UnitType)
	AvailableBuilders[UnitType] = CheckedBuilders[UnitType]
	CheckedBuilders[UnitType] = {}
end

function MoveUnitToIdleChecked(Unit)
	IdleList[Unit] = nil
	CheckedIdleList[Unit] = true
end

function MoveBackCheckedIdle(Unit)
	for Unit,value in pairs(CheckedIdleList) do
		IdleList[Unit] = true
	end
	CheckedIdleList = {}
end

function PerformAI_Unit_AI(Unit, action)
	PerformAI_Unit_Generic(Unit, action)
end

function Empty(table)
	if (table == nil) or (table == {}) then
		return true
	else
		for key,value in pairs(table) do
			return false
		end
		return true
	end
end

function InitAI()

	InitAI_Generic()

	AppendToBuildList(GetUnitTypeFromString("Builder"))
	AppendToBuildList(GetUnitTypeFromString("SolarPanel"))
	AppendToBuildList(GetUnitTypeFromString("SurfaceGeothermal"))
	AppendToBuildList(GetUnitTypeFromString("DeepGeothermal"))
	AppendToBuildList(GetUnitTypeFromString("SmallLightTower"))
	AppendToBuildList(GetUnitTypeFromString("MediumLightTower"))
	AppendToBuildList(GetUnitTypeFromString("LargeLightTower"))
	AppendToBuildList(GetUnitTypeFromString("SmallAttackRobot"))
	AppendToBuildList(GetUnitTypeFromString("LargeAttackRobot"))
	AppendToBuildList(GetUnitTypeFromString("SmallTank"))
	AppendToBuildList(GetUnitTypeFromString("LargeTank"))
	AppendToBuildList(GetUnitTypeFromString("Barracks"))
	AppendToBuildList(GetUnitTypeFromString("TankFactory"))
	AppendToBuildList(GetUnitTypeFromString("MainBuilding"))
	AppendToBuildList(GetUnitTypeFromString("DefenseTower"))
	AppendToBuildList(GetUnitTypeFromString("Explorer"))
end

iterations = 0
startframe = 0

function PerformAI_Player_AI()

	CanBuild = {}

	BuildList = {}

	currentFrame = GetCurrentFrame()

	for i = 1,table_maxn(ToBuild) do
		iterations = iterations + 1
		if not CanBuild[ToBuild[i]] then
			can_produce = false
			if Empty(AvailableBuilders[ToBuild[i]]) and not Empty(CheckedBuilders[ToBuild[i]]) then
				MoveBackChecked(ToBuild[i])
			end
			if IsResearched(ToBuild[i]) then
				builder = GetBuilderCached(ToBuild[i])
				if not Empty(AvailableBuilders[builder]) then
					can_produce = true
					cost = GetUnitTypeBuildCost(ToBuild[i])
				end
			else
				research = GetUnitTypeResearch(ToBuild[i])
				builder = GetResearcherCached(research)
				if not Empty(AvailableBuilders[builder]) then
					can_produce = true
					cost = GetResearchCost(research)
				end
			end
			if ToBuild[i] == GetUnitTypeFromString("Explorer") then
				if GetNumBuilt(ToBuild[i]) > 5 then
					can_produce = false
				end
			end
			if can_produce then
				should_build = ShouldBuild(ToBuild[i])
				if ToBuild[i] == GetUnitTypeFromString("MainBuilding") and GetNumBuilt(GetUnitTypeFromString("MainBuilding")) == 0 then
					should_build = true
					NeedForSurvival[GetUnitTypeFromString("MainBuilding")] = true
				end
				if ToBuild[i] == GetUnitTypeFromString("Builder") and GetNumBuilt(GetUnitTypeFromString("Builder")) == 0 then
					should_build = true
					NeedForSurvival[GetUnitTypeFromString("Builder")] = true
				end
				if should_build then
					InBuildList = true
					CanBuild[ToBuild[i]] = true
					Temp = {UnitType = ToBuild[i], BuilderType = builder, Weight = cost}
					table.insert(BuildList, Temp)
				else
					Need[GetUnitTypeFromString("SolarPanel")] = true
					Need[GetUnitTypeFromString("DeepGeothermal")] = true
					Need[GetUnitTypeFromString("SurfaceGeothermal")] = true
				end
			end
		end
	end

	for i = 1,table_maxn(BuildList) do
		iterations = iterations + 1
		if NeedForSurvival[BuildList[i].UnitType] then
			BuildList[i].NewWeight = 1
		elseif NeedForPower[BuildList[i].UnitType] then
			BuildList[i].NewWeight = BuildList[i].Weight
		elseif Need[BuildList[i].UnitType] then
			BuildList[i].NewWeight = BuildList[i].Weight * math.sqrt(GetNumBuilt(BuildList[i].UnitType) + 1)
		else
			BuildList[i].NewWeight = BuildList[i].Weight * (GetNumBuilt(BuildList[i].UnitType) + 1)
		end
--		Output(GetUnitTypeName(BuildList[i].UnitType) .. " " .. BuildList[i].NewWeight .. "\n")
	end
	
--	Output(iterations)
--	Output(" ")

	NeedForPower = {}
	NeedForSurvival = {}

	table.sort(BuildList, function(a,b) return a.NewWeight<b.NewWeight end)

	for i = 1,table_maxn(BuildList) do
		UnitType = BuildList[i].UnitType
		BuilderType = BuildList[i].BuilderType
		unitavailable = false
		valid = false
		checked = 0
		if not (AvailableBuilders[BuilderType] == nil) then
			for key,value in pairs(AvailableBuilders[BuilderType]) do
				builder = key
				iterations = iterations + 1
				if not (AvailableBuilders[builder] == nil) then
					unitavailable = true
					valid = true
					MoveUnitToChecked(builder, BuilderType)
				end
				checked = checked + 1
				if checked == 10 then
					break
				end
			end
			if not GetUnitTypeIsMobile(UnitType) then
				if valid then
					x, y = GetUnitPosition(builder)
					if UnitType == GetUnitTypeFromString("SmallLightTower") or UnitType == GetUnitTypeFromString("MediumLightTower") or UnitType == GetUnitTypeFromString("LargeLightTower") then
						x, y, valid = GetSuitablePositionForLightTower(UnitType, math.floor(x + math.random(-30, 30)), math.floor(y + math.random(-30, 30)))
					else
						x, y, valid = GetNearestSuitableAndLightedPosition(UnitType, math.floor(x + math.random(-30, 30)), math.floor(y + math.random(-30, 30)))
					end
				end
			end
		end
		if valid then
			if IsResearched(UnitType) then
				
				CommandBuild(builder, x, y, UnitType)

				Need[UnitType] = false
--					Output("Build ")
--					Output(GetUnitTypeName(UnitType))
--					Output("\n")
--					Output("Reserved: " .. TempMoneyReserved .. " " .. TempNoonIncomeChanges .. " " .. TempNightIncomeChanges .. "\n")
--					Output("General: " .. GetPowerAtDawnCached(Player) .. " " .. PowerQuote(Player) .. "\n")
				break
			else
				CommandResearch(builder, GetUnitTypeResearch(UnitType))

				Need[UnitType] = false
--					Output("Research ")
--					Output(GetUnitTypeName(UnitType))
--					Output("\n")
--					Output("Reserved: " .. TempMoneyReserved .. " " .. TempNoonIncomeChanges .. " " .. TempNightIncomeChanges .. "\n")
				break
			end
			SendBackFirstInQueue()
		else
			if checked ~= 10 then
				if unitavailable then
					if UnitType == GetUnitTypeFromString("SolarPanel") or UnitType == GetUnitTypeFromString("DeepGeothermal") or UnitType == GetUnitTypeFromString("SurfaceGeothermal") then
						NeedForPower[GetUnitTypeFromString("SmallLightTower")] = true
						NeedForPower[GetUnitTypeFromString("MediumLightTower")] = true
						NeedForPower[GetUnitTypeFromString("LargeLightTower")] = true
					else
						Need[GetUnitTypeFromString("SmallLightTower")] = true
						Need[GetUnitTypeFromString("MediumLightTower")] = true
						Need[GetUnitTypeFromString("LargeLightTower")] = true
					end
				else
					Need[BuilderType] = true
				end
			end
		end
	end

--	Output(iterations)
--	Output(" ")

	if not (lastAttacker == nil) then
		if not IsValidUnit(lastAttacker) then
			lastAttacker = nil
		end
	end

	if LastMoveBackIdle == nil or currentFrame - LastMoveBackIdle > GetAIFPS_Cached() * 3 then
		MoveBackCheckedIdle()
		LastMoveBackIdle = currentFrame
	end

	for Unit,value in pairs(IdleList) do
		iterations = iterations + 1
		if (GetUnitType(Unit) == GetUnitTypeFromString("Builder")) or not GetUnitCanAttack(Unit) or (lastAttacker == nil) then
			if GetUnitAction(Unit) == UnitAction.None then
				x, y = GetUnitPosition(Unit)
				Type = GetUnitType(Unit)
				new_x = x + math.floor(math.random(-10, 10))
				new_y = y + math.floor(math.random(-10, 10))
				if Type == GetUnitTypeFromString("Explorer") then 
					new_x = x + math.floor(math.random(-100, 100))
					new_y = y + math.floor(math.random(-100, 100))
					CommandGoto(Unit, new_x, new_y)
					LastCommands[Unit] = currentFrame
				elseif SquaresAreLightedAround(Type, new_x, new_y) then
					CommandGoto(Unit, new_x, new_y)
					LastCommands[Unit] = currentFrame
				end
			end
		else
			CommandAttack(Unit, lastAttacker)
			LastCommands[Unit] = currentFrame
		end
		MoveUnitToIdleChecked(Unit)
	end

--	Output(iterations)
--	Output("\n")

	if currentFrame - startframe > GetAIFPS_Cached() * 10 then
--		Output(iterations)
--		Output("\n")
		startframe = currentFrame
		iterations = 0
	end

	if GetPowerAtDawnCached() - TempMoneyReserved < 1000 or PowerQuote() < 2.00 then
--			Output("Lowpower\n")
		NeedForPower[GetUnitTypeFromString("SolarPanel")] = true
		NeedForPower[GetUnitTypeFromString("DeepGeothermal")] = true
		NeedForPower[GetUnitTypeFromString("SurfaceGeothermal")] = true
	end
		
	if GetMoney() < 50 then
		if GetPowerAtDawnCached() - TempMoneyReserved > 600 then
			SellPower(100)
		end
	end

	Cached_PowerAtDawn = nil
	Cached_PowerAtDusk = nil
	Cached_IncomeAtNoon = nil
	Cached_IncomeAtNight = nil
end

function CommandUnit_AI(Unit, target, x, y, action, argument, rotation)
	-- ignore received commands
end

function UnitEvent_UnitCreation_AI(Unit)
	IncNumBuilt(GetUnitType(Unit))
	if GetUnitCanBuild(Unit) then
		SetUnitAvailableForBuilding(Unit, GetUnitType(Unit))
	end
end

function UnitEvent_UnitKilled_AI(Unit)
	if GetUnitCanBuild(Unit) then
		if AvailableBuilders[GetUnitType(Unit)] ~= nil then
			AvailableBuilders[GetUnitType(Unit)][Unit] = nil
		end
		if CheckedBuilders[GetUnitType(Unit)] ~= nil then
			CheckedBuilders[GetUnitType(Unit)][Unit] = nil
		end
		AvailableBuilders[Unit] = nil
	end
	if GetUnitIsMobile(Unit) then
		IdleList[Unit] = nil
		CheckedIdleList[Unit] = nil
	end
	DecNumBuilt(GetUnitType(Unit))
end

function UnitEvent_BecomeIdle_AI(Unit)
	if GetUnitIsMobile(Unit) then
		IdleList[Unit] = true
	end
	UnitEvent_BecomeIdle_Generic(Unit)
end

function UnitEvent_CommandCompleted_AI(Unit, action, x, y, goal, arg)
	if action == UnitAction.Build then
		UnitType = arg;
		if GetUnitTypeIncomeAtNoon(UnitType) < 0 then
			TempNoonIncomeChanges = TempNoonIncomeChanges - GetUnitTypeIncomeAtNoon(UnitType)
		end
		if GetUnitTypeIncomeAtNight(UnitType) < 0 then
			TempNightIncomeChanges = TempNightIncomeChanges - GetUnitTypeIncomeAtNight(UnitType)
		end
		TempMoneyReserved = TempMoneyReserved - GetUnitTypeBuildCost(UnitType)
		SetUnitAvailableForBuilding(Unit, GetUnitType(Unit))
	elseif action == UnitAction.Research then
		TempMoneyReserved = TempMoneyReserved - GetResearchCost(arg)
		SetUnitAvailableForBuilding(Unit, GetUnitType(Unit))
	end
end

function UnitEvent_CommandCancelled_AI(Unit, action, x, y, goal, arg)
	if action == UnitAction.Build then
		UnitType = arg;
		if GetUnitTypeIncomeAtNoon(UnitType) < 0 then
			TempNoonIncomeChanges = TempNoonIncomeChanges - GetUnitTypeIncomeAtNoon(UnitType)
		end
		if GetUnitTypeIncomeAtNight(UnitType) < 0 then
			TempNightIncomeChanges = TempNightIncomeChanges - GetUnitTypeIncomeAtNight(UnitType)
		end
		TempMoneyReserved = TempMoneyReserved - GetUnitTypeBuildCost(UnitType)
		SetUnitAvailableForBuilding(Unit, GetUnitType(Unit))
	elseif action == UnitAction.Research then
		TempMoneyReserved = TempMoneyReserved - GetResearchCost(arg)
		SetUnitAvailableForBuilding(Unit, GetUnitType(Unit))
	end
end

function UnitEvent_NewCommand_AI(Unit, action, x, y, goal, arg)
	if GetUnitIsMobile(Unit) then
		IdleList[Unit] = nil
		CheckedIdleList[Unit] = nil
	end
	if (action == UnitAction.Build) or (action == UnitAction.Research) then
		if action == UnitAction.Build then
			UnitType = GetUnitType(Unit)
			if GetUnitTypeIncomeAtNoon(UnitType) < 0 then
				TempNoonIncomeChanges = TempNoonIncomeChanges + GetUnitTypeIncomeAtNoon(UnitType)
			end
			if GetUnitTypeIncomeAtNight(UnitType) < 0 then
				TempNightIncomeChanges = TempNightIncomeChanges + GetUnitTypeIncomeAtNight(UnitType)
			end
			TempMoneyReserved = TempMoneyReserved + GetUnitTypeBuildCost(UnitType)
		else
			TempMoneyReserved = TempMoneyReserved + GetResearchCost(arg)
		end
		SetUnitBuilding(Unit, GetUnitType(Unit))
	end
	UnitEvent_NewCommand_Generic(Unit, action, x, y, goal, arg)
end

function UnitEvent_IsAttacked_AI(Unit, attacker)
	if GetUnitType(attacker) == GetUnitTypeFromString("Grue") then
		return
	end
	if GetUnitCanAttack(Unit) and not (GetUnitType(Unit) == GetUnitTypeFromString("Builder")) then
		CommandAttack(Unit, attacker)
	end
	lastAttacker = attacker
end

