
function table_maxn(table)
	i = 1
	while table[i] ~= nil do
		i = i + 1
	end
	return i-1
end

LastCheckForTarget = {}

function PerformAI_Unit_Generic(Unit)

	if not (GetUnitType(Unit) == Builder.pointer) then
		action = GetUnitAction(Unit)

		if action == UnitAction.None and (LastCommands[Unit] == nil or os.difftime(os.time(), LastCheckForTarget[Unit]) > 0.1) and GetUnitCanAttack(Unit) then
			targetUnit = GetNearestUnitInRange(Unit, RangeType.Sight, PlayerState.Enemy)
			LastCheckForTarget[Unit] = os.time()
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
end

