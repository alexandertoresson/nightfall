
function table_maxn(table)
	i = 1
	while table[i] ~= nil do
		i = i + 1
	end
	return i-1
end

LastCheckForTarget = {}

function PerformAI_Unit_Generic(Unit)

	action = GetUnitAction(Unit)

	if action == UnitAction.None and (LastCommands[Unit] == nil or os.difftime(os.time(), LastCheckForTarget[Unit]) > 0.1) and GetUnitCanAttack(Unit) then
		targetUnit = GetNearestUnitInRange(Unit, RangeType.Sight, PlayerState.Enemy)
		LastCheckForTarget[Unit] = os.time()
		if IsNonNull(targetUnit) then
			CommandUnit_TargetUnit(Unit, targetUnit, UnitAction.Attack, Null())
		end
		action = GetUnitAction(Unit);
	end

	if action == UnitAction.Goto or action == UnitAction.Follow or action == UnitAction.Attack or action == UnitAction.Build then
	
--		should_move = GetUnitIsMobile(Unit)
--
--		if GetUnitCanAttack(Unit) and action == UnitAction.Attack then
--			targetUnit = GetUnitTargetUnit(Unit);
--			if CanReach(Unit, targetUnit) then
--				if CanAttack(Unit) then
--					InitiateAttack(Unit, targetUnit)
--				end
--				should_move = false
--				SetUnitIsMoving(Unit, false);
--			end
--		end
--
--		if action == UnitAction.Build then
--			if IsWithinRangeForBuilding(Unit) then
--				Build(Unit)
--				should_move = false
--				SetUnitIsMoving(Unit, false);
--			end
--		end
--
--		if should_move then
--			Move(Unit)	
--		end
		
		if GetUnitCanAttackWhileMoving(Unit) and (LastCommands[Unit] == nil or os.difftime(os.time(), LastCheckForTarget[Unit]) > 0.1) and CanAttack(Unit) then
			LastCheckForTarget[Unit] = os.time()
			if action == UnitAction.Attack then
				targetUnit = GetUnitTargetUnit(Unit);
			else
				targetUnit = Null()
			end
			if not IsNonNull(targetUnit) then
				targetUnit = GetNearestUnitInRange(Unit, RangeType.Sight, PlayerState.Enemy)
			end
			if IsNonNull(targetUnit) then
				InitiateAttack(Unit, targetUnit)
			end
		end
	end
end

