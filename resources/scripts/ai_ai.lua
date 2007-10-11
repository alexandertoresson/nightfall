loadfile(GetLUAScript("ai_generic.lua"))()

ToBuild = {}
NumBuilt = {}

TempNightIncomeChanges = 0
TempNoonIncomeChanges = 0
TempMoneyReserved = 0
Need = {}
NeedForPower = {}

function GetPowerAtDawnCached(Player)
	if Cached_PowerAtDawn == nil then
		Cached_PowerAtDawn = GetPowerAtDawn(Player)
	end
	return Cached_PowerAtDawn
end

function GetPowerAtDuskCached(Player)
	if Cached_PowerAtDusk == nil then
		Cached_PowerAtDusk = GetPowerAtDusk(Player)
	end
	return Cached_PowerAtDusk
end

function GetIncomeAtNoonCached(Player)
	if Cached_IncomeAtNoon == nil then
		Cached_IncomeAtNoon = GetIncomeAtNoon(Player)
	end
	return Cached_IncomeAtNoon
end

function GetIncomeAtNightCached(Player)
	if Cached_IncomeAtNight == nil then
		Cached_IncomeAtNight = GetIncomeAtNight(Player)
	end
	return Cached_IncomeAtNight
end

function PowerQuote(Player)
	if GetIncomeAtNightCached(Player) >= 0 then
		return 10.0
	else
		return (GetIncomeAtNoonCached(Player)) / -(GetIncomeAtNightCached(Player))
	end
end

function ShouldBuild(Player, UnitType)
	if IsResearched(Player, UnitType) then
		if NeedForPower[UnitType] then
			return true
		end
		if (GetPowerAtDawnCached(Player) - GetUnitTypeBuildCost(UnitType) - TempMoneyReserved > 500) then
			if GetIncomeAtNightCached(Player) + GetUnitTypeIncomeAtNight(UnitType) + TempNightIncomeChanges >= 0 then
				return true
			else
				return (GetIncomeAtNoonCached(Player) + TempNoonIncomeChanges + GetUnitTypeIncomeAtNoon(UnitType)) / -(GetIncomeAtNightCached(Player) + TempNightIncomeChanges + GetUnitTypeIncomeAtNight(UnitType)) > 1.3
			end
		else
			return false
		end
	else
		return GetPowerAtDawnCached(Player) - GetUnitTypeResearchCost(UnitType) - TempMoneyReserved > 500
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

IdleList = {}
AvailableBuilders = {}
CheckedBuilders = {}

function GetResearcherCached(UnitType)
	if Researchers[UnitType] == nil then
		Researchers[UnitType] = GetResearcher(UnitType)
	end
	return Researchers[UnitType]
end

function GetBuilderCached(UnitType)
	if Builders[UnitType] == nil then
		Builders[UnitType] = GetBuilder(UnitType)
	end
	return Builders[UnitType]
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

function PerformAI_Unit_AI(Unit)
	PerformAI_Unit_Generic(Unit)
end

function Empty(table)
	if (table == nil) or (table == {}) then
		return true
	end
	return false
end

function PerformAI_Player_AI(Player)

	CanBuild = {}

	BuildList = {}

	iterations = 0;

	for i = 1,table_maxn(ToBuild) do
		iterations = iterations + 1
		if not CanBuild[ToBuild[i]] then
			can_produce = false
			if IsResearched(Player, ToBuild[i]) then
				builder = GetBuilderCached(ToBuild[i])
				if not Empty(AvailableBuilders[builder]) then
					can_produce = true
					cost = GetUnitTypeBuildCost(ToBuild[i])
				end
			else
				builder = GetResearcherCached(ToBuild[i])
				if not Empty(AvailableBuilders[builder]) then
					can_produce = true
					cost = GetUnitTypeResearchCost(ToBuild[i])
				end
			end
			if can_produce then
				if ShouldBuild(Player, ToBuild[i]) then
					InBuildList = true
					CanBuild[ToBuild[i]] = true
					Temp = {UnitType = ToBuild[i], BuilderType = builder, Weight = cost}
					table.insert(BuildList, Temp)
				else
					Need[SolarPanel.pointer] = true
					Need[DeepGeothermal.pointer] = true
					Need[SurfaceGeothermal.pointer] = true
				end
			end
		end
	end

	for i = 1,table_maxn(BuildList) do
		iterations = iterations + 1
		if NeedForPower[BuildList[i].UnitType] then
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

	table.sort(BuildList, function(a,b) return a.NewWeight<b.NewWeight end)
	iterations = 0;

	for i = 1,table_maxn(BuildList) do
		UnitType = BuildList[i].UnitType
		BuilderType = BuildList[i].BuilderType
		unitavailable = false
		valid = false
		if not (AvailableBuilders[BuilderType] == nil) then
			if GetUnitTypeIsMobile(UnitType) then
				builder = nil
				for key,value in pairs(AvailableBuilders[BuilderType]) do
					builder = key
					iterations = iterations + 1
					if not (AvailableBuilders[builder] == nil) then
						unitavailable = true
						valid = true
						MoveUnitToChecked(builder, BuilderType)
						break
					end
				end
			else
				if Empty(AvailableBuilders[BuilderType]) and not Empty(CheckedBuilders[BuilderType]) then
					MoveBackChecked(BuilderType)
				end
				for key,value in pairs(AvailableBuilders[BuilderType]) do
					builder = key
					iterations = iterations + 1
					if not (AvailableBuilders[builder] == nil) then
						unitavailable = true
						valid = true
						MoveUnitToChecked(builder, BuilderType)
					end
				end
				if valid then
					x, y = GetUnitPosition(builder)
					if UnitType == SmallLightTower.pointer or UnitType == MediumLightTower.pointer or UnitType == LargeLightTower.pointer then
						x, y, valid = GetSuitablePositionForLightTower(UnitType, Player, math.floor(x + math.random(-30, 30)), math.floor(y + math.random(-30, 30)))
					else
						x, y, valid = GetNearestSuitableAndLightedPosition(UnitType, Player, math.floor(x + math.random(-30, 30)), math.floor(y + math.random(-30, 30)))
					end
				end
			end
		end
		if valid then
			if IsResearched(Player, UnitType) then
				CommandBuild(builder, x, y, UnitType)

				if GetUnitTypeIncomeAtNoon(UnitType) < 0 then
					TempNoonIncomeChanges = TempNoonIncomeChanges + GetUnitTypeIncomeAtNoon(UnitType)
				end
				if GetUnitTypeIncomeAtNight(UnitType) < 0 then
					TempNightIncomeChanges = TempNightIncomeChanges + GetUnitTypeIncomeAtNight(UnitType)
				end
				TempMoneyReserved = TempMoneyReserved + GetUnitTypeBuildCost(UnitType)
				Need[UnitType] = false
--					Output("Build ")
--					Output(GetUnitTypeName(UnitType))
--					Output("\n")
--					Output("Reserved: " .. TempMoneyReserved .. " " .. TempNoonIncomeChanges .. " " .. TempNightIncomeChanges .. "\n")
--					Output("General: " .. GetPowerAtDawnCached(Player) .. " " .. PowerQuote(Player) .. "\n")
				break
			else
				CommandResearch(builder, UnitType)

				TempMoneyReserved = TempMoneyReserved + GetUnitTypeResearchCost(UnitType)
				Need[UnitType] = false
--					Output("Research ")
--					Output(GetUnitTypeName(UnitType))
--					Output("\n")
--					Output("Reserved: " .. TempMoneyReserved .. " " .. TempNoonIncomeChanges .. " " .. TempNightIncomeChanges .. "\n")
				break
			end
			SendBackFirstInQueue()
		else
			if unitavailable then
				if UnitType == SolarPanel.pointer or UnitType == DeepGeothermal.pointer or UnitType == SurfaceGeothermal.pointer then
					NeedForPower[SmallLightTower.pointer] = true
					NeedForPower[MediumLightTower.pointer] = true
					NeedForPower[LargeLightTower.pointer] = true
				else
					Need[SmallLightTower.pointer] = true
					Need[MediumLightTower.pointer] = true
					Need[LargeLightTower.pointer] = true
				end
			else
				Need[BuilderType] = true
			end
		end
	end

--	Output(iterations)
--	Output(" ")

	iterations = 0

	if not (lastAttacker == nil) then
		if not IsValidUnit(lastAttacker) then
			lastAttacker = nil
		end
	end

	for Unit,value in pairs(IdleList) do
		if (GetUnitType(Unit) == Builder.pointer) or not GetUnitCanAttack(Unit) or (lastAttacker == nil) then
			if GetUnitAction(Unit) == UnitAction.None then
				x, y = GetUnitPosition(Unit)
				Type = GetUnitType(Unit)
				new_x = x + math.floor(math.random(-10, 10))
				new_y = y + math.floor(math.random(-10, 10))
				if SquaresAreLightedAround(Type, Player, new_x, new_y) then
					CommandGoto(Unit, new_x, new_y)
				end
			end
		else
			CommandAttack(Unit, lastAttacker)
		end
	end

--	Output(iterations)
--	Output("\n")

	if ToBuild[1] == nil then
--				Output("Replace\n")
		AppendToBuildList(Builder.pointer)
		AppendToBuildList(SolarPanel.pointer)
		AppendToBuildList(SurfaceGeothermal.pointer)
		AppendToBuildList(DeepGeothermal.pointer)
		AppendToBuildList(SmallLightTower.pointer)
		AppendToBuildList(MediumLightTower.pointer)
		AppendToBuildList(LargeLightTower.pointer)
		AppendToBuildList(SmallAttackRobot.pointer)
		AppendToBuildList(LargeAttackRobot.pointer)
		AppendToBuildList(SmallTank.pointer)
		AppendToBuildList(LargeTank.pointer)
		AppendToBuildList(Barracks.pointer)
		AppendToBuildList(TankFactory.pointer)
		AppendToBuildList(MainBuilding.pointer)
		AppendToBuildList(DefenseTower.pointer)
	end
	if GetPowerAtDawnCached(Player) - TempMoneyReserved < 1000 or PowerQuote(Player) < 2.00 then
--			Output("Lowpower\n")
		NeedForPower[SolarPanel.pointer] = true
		NeedForPower[DeepGeothermal.pointer] = true
		NeedForPower[SurfaceGeothermal.pointer] = true
	end
		
	if LastSell == nil or os.difftime(os.time(), LastSell) > 0.50 then
		if GetMoney(Player) < 50 then
			if GetPowerAtDawnCached(Player) - TempMoneyReserved > 600 then
				SellPower(Player, 100)
				LastSell = os.time()
			end
		end
	end
	Cached_PowerAtDawn = nil
	Cached_PowerAtDusk = nil
	Cached_IncomeAtNoon = nil
	Cached_IncomeAtNight = nil
end

function UnitEvent_BuildComplete_AI(Unit, UnitType, TargetUnit)
--	if GetUnitTypeIncomeAtNoon(UnitType) < 0 then
--		TempNoonIncomeChanges = TempNoonIncomeChanges - GetUnitTypeIncomeAtNoon(UnitType)
--	end
--	if GetUnitTypeIncomeAtNight(UnitType) < 0 then
--		TempNightIncomeChanges = TempNightIncomeChanges - GetUnitTypeIncomeAtNight(UnitType)
--	end
--	TempMoneyReserved = TempMoneyReserved - GetUnitTypeBuildCost(UnitType)
--	Output("Complete Build: " .. GetUnitTypeName(UnitType) .. "\n")
--	Output("Reserved: " .. TempMoneyReserved .. " " .. TempNoonIncomeChanges .. " " .. TempNightIncomeChanges .. "\n")
end

function UnitEvent_BuildCancelled_AI(Unit, UnitType, TargetUnit)
--	if GetUnitTypeIncomeAtNoon(UnitType) < 0 then
--		TempNoonIncomeChanges = TempNoonIncomeChanges - GetUnitTypeIncomeAtNoon(UnitType)
--	end
--	if GetUnitTypeIncomeAtNight(UnitType) < 0 then
--		TempNightIncomeChanges = TempNightIncomeChanges - GetUnitTypeIncomeAtNight(UnitType)
--	end
--	TempMoneyReserved = TempMoneyReserved - GetUnitTypeBuildCost(UnitType)
--	DecNumBuilt(UnitType)
--	Output("Cancelled Build: " .. GetUnitTypeName(UnitType) .. "\n")
--	Output("Reserved: " .. TempMoneyReserved .. " " .. TempNoonIncomeChanges .. " " .. TempNightIncomeChanges .. "\n")
end

function UnitEvent_ResearchComplete_AI(Unit, UnitType)
--	TempMoneyReserved = TempMoneyReserved - GetUnitTypeResearchCost(UnitType)
--	Output("Complete Research: " .. GetUnitTypeName(UnitType) .. "\n")
--	Output("Reserved: " .. TempMoneyReserved .. " " .. TempNoonIncomeChanges .. " " .. TempNightIncomeChanges .. "\n")
end

function UnitEvent_ResearchCancelled_AI(Unit, UnitType)
--	TempMoneyReserved = TempMoneyReserved - GetUnitTypeResearchCost(UnitType)
--	Output("Cancelled Research: " .. GetUnitTypeName(UnitType) .. "\n")
--	Output("Reserved: " .. TempMoneyReserved .. " " .. TempNoonIncomeChanges .. " " .. TempNightIncomeChanges .. "\n")
end

function CommandUnit_TargetPos_AI(Unit, x, y, action, argument)
	-- ignore received commands
end

function CommandUnit_TargetUnit_AI(Unit, target, action, argument)
	-- ignore received commands
end

function UnitEvent_UnitCreation_AI(Unit, Type, x, y)
	IncNumBuilt(GetUnitType(Unit))
	if GetUnitCanBuild(Unit) then
		SetUnitAvailableForBuilding(Unit, Type)
	end
end

function UnitEvent_UnitKilled_AI(Unit)
	if GetUnitCanBuild(Unit) then
		AvailableBuilders[GetUnitType(Unit)][Unit] = nil
		AvailableBuilders[Unit] = nil
		IdleList[Unit] = nil
	end
	DecNumBuilt(GetUnitType(Unit))
end

function UnitEvent_BecomeIdle_AI(Unit)
	IdleList[Unit] = true
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
		TempMoneyReserved = TempMoneyReserved - GetUnitTypeResearchCost(UnitType)
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
		TempMoneyReserved = TempMoneyReserved - GetUnitTypeResearchCost(UnitType)
		SetUnitAvailableForBuilding(Unit, GetUnitType(Unit))
	end
end

function UnitEvent_NewCommand_AI(Unit, action, x, y, goal, arg)
	IdleList[Unit] = nil
	if (action == UnitAction.Build) or (action == UnitAction.Research) then
		SetUnitBuilding(Unit, GetUnitType(Unit))
	end
end

function UnitEvent_IsAttacked_AI(Unit, attacker)
	if GetUnitType(attacker) == Grue.pointer then
		return
	end
	if GetUnitCanAttack(Unit) and not (GetUnitType(Unit) == Builder.pointer) then
		CommandAttack(Unit, attacker)
	end
	lastAttacker = attacker
end

