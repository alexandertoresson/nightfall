require "resources/scripts/ai_generic"

NumMonsters = 0
CreationTimes = {}
LastCommands = {}

function PerformAI_Unit_Gaia(Unit)
	if GetUnitIsHurtByLight(Unit) then
		if GetTime() >= 6.0 and GetTime() <= 18.0 then
			Attack(Unit, 0.3 * GetUnitMaxHealth(Unit))
		else 
			if GetUnitLightAmount(Unit) > 0.001 then
				Attack(Unit, GetUnitLightAmount(Unit) * 0.3 * GetUnitMaxHealth(Unit))
			end
		end
	end

	if GetUnitType(Unit) == GetUnitTypeFromString("Grue") then

		if (math.random() < 1/100) then
			Attack(Unit, 1.0 * GetUnitMaxHealth(Unit) + 1)
			NumMonsters = NumMonsters - 1
		end

		action = GetUnitAction(Unit)

		if (action == UnitAction.None and (LastCommands[Unit] == nil or os.difftime(os.time(), LastCommands[Unit]) > 0.25)) and math.random() < 0.8 then
			targetUnit = GetNearestUnitInRange(Unit, RangeType.Sight, PlayerState.Neutral)
			LastCommands[Unit] = os.time();
			if IsNonNull(targetUnit) then
				CommandUnit_TargetUnit(Unit, targetUnit, UnitAction.Attack, Null())
			end
		end

	end

	if action == UnitAction.None and (LastCommands[Unit] == nil or os.difftime(os.time(), LastCommands[Unit]) > 0.25) then
		if math.random() > 0.3 then
			x, y = GetUnitPosition(Unit)
			CommandUnit_TargetPos(Unit, x + math.random(-1, 1) * 10, y + math.random(-1, 1) * 10, UnitAction.Move, Null())
		end
		LastCommands[Unit] = os.time();
	end

	PerformAI_Unit_Generic(Unit)
end

function PerformAI_Player_Gaia(Player)
	if (GetTime() < 6.0 or GetTime() > 18.0) then
		if NumMonsters < 100 then
			width, height = GetMapDimensions()
			UnitType = GetUnitTypeFromString("Grue")

			repeat
				x = (width-1) * math.random()
				y = (height-1) * math.random()
			until CanCreateUnitAt(UnitType, Player, x, y)

			CreateUnit(UnitType, Player, x,  y)
--			Unit = CreateUnit(UnitType, Player, x,  y)
--			CreationTimes[Unit] = os.time()
			NumMonsters = NumMonsters + 1
		end
	else
		NumMonsters = 0
	end
end

function CommandUnit_TargetPos_Gaia(Unit, x, y, action, argument)
	-- ignore received commands
end

function CommandUnit_TargetUnit_Gaia(Unit, target, action, argument)
	-- ignore received commands
end
