require "resources/scripts/ai_generic"

NeedBuilder = false
UnitAIHasBeenPerformed = true
--Tested = ""

ToBuild = {}
NumBuilding = {}
NumBuilt = {}

TempNightIncomeChanges = 0
TempNoonIncomeChanges = 0
TempMoneyReserved = 0
Need = {}

LastChecked = {}

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
		if UnitType == GetUnitTypeFromString("SolarPanel") or UnitType == GetUnitTypeFromString("SurfaceGeothermal") or UnitType == GetUnitTypeFromString("DeepGeothermal") then
			return true
		end
		if (GetPowerAtDawnCached(Player) - GetUnitTypeBuildCost(UnitType) - TempMoneyReserved > 500) then
			if GetIncomeAtNightCached(Player) + GetUnitTypeIncomeAtNight(UnitType) + TempNightIncomeChanges >= 0 then
				return true
			else
				return (GetIncomeAtNoonCached(Player) + TempNoonIncomeChanges + GetUnitTypeIncomeAtNoon(UnitType)) / -(GetIncomeAtNightCached(Player) + TempNightIncomeChanges + GetUnitTypeIncomeAtNight(UnitType)) > 1.9
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
	NumBuilding[UnitType] = 1
end

function PrependToBuildList(UnitType)
	table.insert(ToBuild, 1, UnitType)
	NumBuilding[UnitType] = 1
end

function SendBackFirstInQueue()
	UnitType = ToBuild[1]
	table.remove(ToBuild, 1)
	table.insert(ToBuild, UnitType)
end

CanBuild = {}
BuildList = {}

Builders = {}
Researchers = {}

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

DoingNothing = {}

function PerformAI_Unit_AI(Unit)
--	Tested = Tested .. " " .. GetUnitTypeName(GetUnitType(Unit))
	action = GetUnitAction(Unit)
	timepassed = LastChecked[Unit] == nil or os.difftime(os.time(), LastChecked[Unit]) > 3.00
	if timepassed and action == UnitAction.None then
		InBuildList = false
		for i = 1,table_maxn(ToBuild) do
			if not CanBuild[ToBuild[i]] then
				can_produce = false
				if IsResearched(GetUnitOwner(Unit), ToBuild[i]) then
					builder = GetBuilderCached(ToBuild[i])
					if GetUnitType(Unit) == builder then
						can_produce = true
						cost = GetUnitTypeBuildCost(ToBuild[i])
					end
				else
					builder = GetResearcherCached(ToBuild[i])
					if GetUnitType(Unit) == builder then
						can_produce = true
						cost = GetUnitTypeResearchCost(ToBuild[i])
					end
				end
				if can_produce then
					if ShouldBuild(GetUnitOwner(Unit), ToBuild[i]) then
						InBuildList = true
						CanBuild[ToBuild[i]] = true
						Temp = {UnitType = ToBuild[i], Builder = Unit, Weight = cost}
						table.insert(BuildList, Temp)
					else
						Need[GetUnitTypeFromString("SolarPanel")] = true
						Need[GetUnitTypeFromString("DeepGeothermal")] = true
						Need[GetUnitTypeFromString("SurfaceGeothermal")] = true
					end
				end
			end
		end
		if not InBuildList and GetUnitTypeIsMobile(GetUnitType(Unit)) and timepassed then
			table.insert(DoingNothing, Unit)
		end
	end
	UnitAIHasBeenPerformed = true
	PerformAI_Unit_Generic(Unit)
end

function PerformAI_Player_AI(Player)
	if UnitAIHasBeenPerformed then
		for i = 1,table_maxn(BuildList) do
			if not Need[BuildList[i].UnitType] then
				BuildList[i].Weight = BuildList[i].Weight * (GetNumBuilt(BuildList[i].UnitType) + 1)
			end
		end
		
		table.sort(BuildList, function(a,b) return a.Weight<b.Weight end)

		for i = 1,table_maxn(BuildList) do
			UnitType = BuildList[i].UnitType
			Builder = BuildList[i].Builder
			x, y = GetUnitPosition(Builder)
			if GetUnitTypeIsMobile(UnitType) then
				valid = true
			else
				if UnitType == GetUnitTypeFromString("SmallLightTower") or UnitType == GetUnitTypeFromString("MediumLightTower") or UnitType == GetUnitTypeFromString("LargeLightTower") then
					x, y, valid = GetSuitablePositionForLightTower(UnitType, Player, math.floor(x + math.random(-30, 30)), math.floor(y + math.random(-30, 30)))
				else
					x, y, valid = GetNearestSuitableAndLightedPosition(UnitType, Player, math.floor(x + math.random(-30, 30)), math.floor(y + math.random(-30, 30)))
				end
				LastChecked[Builder] = os.time()
			end
			if valid then
				if IsResearched(Player, UnitType) then
					CommandBuild(Builder, x, y, UnitType)
					IncNumBuilt(UnitType)
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
					for j = i+1,table_maxn(BuildList) do
						if GetUnitTypeIsMobile(GetUnitType(BuildList[j].Builder)) then
							table.insert(DoingNothing, BuildList[j].Builder)
						end
					end
					break
				else
					CommandResearch(Builder, UnitType)
					IncNumBuilt(UnitType)
					TempMoneyReserved = TempMoneyReserved + GetUnitTypeResearchCost(UnitType)
					Need[UnitType] = false
--					Output("Research ")
--					Output(GetUnitTypeName(UnitType))
--					Output("\n")
--					Output("Reserved: " .. TempMoneyReserved .. " " .. TempNoonIncomeChanges .. " " .. TempNightIncomeChanges .. "\n")
					for j = i+1,table_maxn(BuildList) do
						if GetUnitTypeIsMobile(GetUnitType(BuildList[j].Builder)) then
							table.insert(DoingNothing, BuildList[j].Builder)
						end
					end
					break
				end
				SendBackFirstInQueue()
			else
				if GetUnitTypeIsMobile(GetUnitType(Builder)) then
					table.insert(DoingNothing, Builder)
				end
				Need[GetUnitTypeFromString("SmallLightTower")] = true
				Need[GetUnitTypeFromString("MediumLightTower")] = true
				Need[GetUnitTypeFromString("LargeLightTower")] = true
			end
		end

		for i = 1,table_maxn(DoingNothing) do
			Unit = DoingNothing[i]
			if GetUnitAction(Unit) == UnitAction.None then
				x, y = GetUnitPosition(Unit)
				Type = GetUnitType(Unit)
				x = x + math.floor(math.random(-10, 10))
				y = y + math.floor(math.random(-10, 10))
				if SquaresAreLightedAround(Type, Player, x, y) then
					CommandGoto(Unit, x, y)
				end
				LastChecked[Unit] = os.time()
			end
		end

		NoChange = true
		if ToBuild[1] == nil then
--				Output("Replace\n")
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
			NoChange = false
		end
		if GetPowerAtDawnCached(Player) - TempMoneyReserved < 1000 or PowerQuote(Player) < 2.00 or NeedPower then
--			Output("Lowpower\n")
			Need[GetUnitTypeFromString("SolarPanel")] = true
			Need[GetUnitTypeFromString("DeepGeothermal")] = true
			Need[GetUnitTypeFromString("SurfaceGeothermal")] = true
			NoChange = false
			NeedPower = false
		end
		NeedBuilder = true
		UnitAIHasBeenPerformed = false
--		Tested = ""
--		if NoChange then
--			SendBackFirstInQueue()
--		end
		CanBuild = {}
		BuildList = {}
		DoingNothing = {}
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
	if GetUnitTypeIncomeAtNoon(UnitType) < 0 then
		TempNoonIncomeChanges = TempNoonIncomeChanges - GetUnitTypeIncomeAtNoon(UnitType)
	end
	if GetUnitTypeIncomeAtNight(UnitType) < 0 then
		TempNightIncomeChanges = TempNightIncomeChanges - GetUnitTypeIncomeAtNight(UnitType)
	end
	TempMoneyReserved = TempMoneyReserved - GetUnitTypeBuildCost(UnitType)
--	Output("Complete Build: " .. GetUnitTypeName(UnitType) .. "\n")
--	Output("Reserved: " .. TempMoneyReserved .. " " .. TempNoonIncomeChanges .. " " .. TempNightIncomeChanges .. "\n")
end

function UnitEvent_BuildCancelled_AI(Unit, UnitType, TargetUnit)
	if GetUnitTypeIncomeAtNoon(UnitType) < 0 then
		TempNoonIncomeChanges = TempNoonIncomeChanges - GetUnitTypeIncomeAtNoon(UnitType)
	end
	if GetUnitTypeIncomeAtNight(UnitType) < 0 then
		TempNightIncomeChanges = TempNightIncomeChanges - GetUnitTypeIncomeAtNight(UnitType)
	end
	TempMoneyReserved = TempMoneyReserved - GetUnitTypeBuildCost(UnitType)
	DecNumBuilt(UnitType)
--	Output("Cancelled Build: " .. GetUnitTypeName(UnitType) .. "\n")
--	Output("Reserved: " .. TempMoneyReserved .. " " .. TempNoonIncomeChanges .. " " .. TempNightIncomeChanges .. "\n")
end

function UnitEvent_ResearchComplete_AI(Unit, UnitType)
	TempMoneyReserved = TempMoneyReserved - GetUnitTypeResearchCost(UnitType)
--	Output("Complete Research: " .. GetUnitTypeName(UnitType) .. "\n")
--	Output("Reserved: " .. TempMoneyReserved .. " " .. TempNoonIncomeChanges .. " " .. TempNightIncomeChanges .. "\n")
end

function UnitEvent_ResearchCancelled_AI(Unit, UnitType)
	TempMoneyReserved = TempMoneyReserved - GetUnitTypeResearchCost(UnitType)
--	Output("Cancelled Research: " .. GetUnitTypeName(UnitType) .. "\n")
--	Output("Reserved: " .. TempMoneyReserved .. " " .. TempNoonIncomeChanges .. " " .. TempNightIncomeChanges .. "\n")
end

function CommandUnit_TargetPos_AI(Unit, x, y, action, argument)
	-- ignore received commands
end

function CommandUnit_TargetUnit_AI(Unit, target, action, argument)
	-- ignore received commands
end
