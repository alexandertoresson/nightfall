
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

function InitAI_Generic(Player)
	SetEventHandler(GetUnitTypeFromString("Builder"), EventType.PerformUnitAI, "", Player)
	SetEventHandler(GetUnitTypeFromString("MainBuilding"), EventType.PerformUnitAI, "", Player)
	SetEventHandler(GetUnitTypeFromString("SolarPanel"), EventType.PerformUnitAI, "", Player)
	SetEventHandler(GetUnitTypeFromString("SurfaceGeothermal"), EventType.PerformUnitAI, "", Player)
	SetEventHandler(GetUnitTypeFromString("DeepGeothermal"), EventType.PerformUnitAI, "", Player)
	SetEventHandler(GetUnitTypeFromString("SmallLightTower"), EventType.PerformUnitAI, "", Player)
	SetEventHandler(GetUnitTypeFromString("MediumLightTower"), EventType.PerformUnitAI, "", Player)
	SetEventHandler(GetUnitTypeFromString("LargeLightTower"), EventType.PerformUnitAI, "", Player)
	SetEventHandler(GetUnitTypeFromString("Barracks"), EventType.PerformUnitAI, "", Player)
	SetEventHandler(GetUnitTypeFromString("TankFactory"), EventType.PerformUnitAI, "", Player)
	SetEventHandler(GetUnitTypeFromString("Explorer"), EventType.PerformUnitAI, "", Player)
end

function PerformAI_Unit_Generic(Unit, action)

	if action == UnitAction.None and (LastCheckForTarget[Unit] == nil or GetCurrentFrame() - LastCheckForTarget[Unit] > GetAIFPS() * 0.1) and GetUnitCanAttack(Unit) then
		targetUnit = GetNearestUnitInRange(Unit, RangeType.Sight, PlayerState.Enemy)
		LastCheckForTarget[Unit] = GetCurrentFrame()
		if IsNonNull(targetUnit) then
			CommandUnit_TargetUnit(Unit, targetUnit, UnitAction.Attack, Null())
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

