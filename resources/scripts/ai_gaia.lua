loadfile(GetLUAScript("ai_generic.lua"))()

NumMonsters = 0
CreationTimes = {}
LastCommands = {}

function InitAI_Gaia(Player)

	InitAI_Generic(Player)

end

function PerformAI_Unit_Gaia(Unit, action)

	if GetUnitType(Unit) == GetUnitTypeFromString("Grue") then

		if GetTime() >= 6.0 and GetTime() <= 18.0 then
			Attack(Unit, 0.3 * GetUnitMaxHealth(Unit))
		else 
			if GetUnitLightAmount(Unit) > 0.001 then
				Attack(Unit, GetUnitLightAmount(Unit) * 0.3 * GetUnitMaxHealth(Unit))
			end
		end

		if (math.random() < 1/100) then
			Attack(Unit, 1.0 * GetUnitMaxHealth(Unit) + 1)
		end

		action = GetUnitAction(Unit)

		if (action == UnitAction.None and (LastCommands[Unit] == nil or os.difftime(os.time(), LastCommands[Unit]) > 0.25)) and math.random() < 0.8 then
			targetUnit = GetNearestUnitInRange(Unit, RangeType.Sight, PlayerState.Neutral)
			LastCommands[Unit] = os.time()
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

	PerformAI_Unit_Generic(Unit, action)
end

function PerformAI_Player_Gaia(Player)
	if (GetTime() < 6.0 or GetTime() > 18.0) then
		if NumMonsters < 25 then
			width, height = GetMapDimensions()
			UnitType = GetUnitTypeFromString("Grue")

			repeat
				x = (width-1) * math.random()
				y = (height-1) * math.random()
			until CanCreateUnitAt(UnitType, Player, x, y)

			CreateUnit(UnitType, Player, x,  y)
		end
	else
		NumMonsters = 0
	end
end

function UnitEvent_UnitKilled_Gaia(Unit)
	if GetUnitType(Unit) == GetUnitTypeFromString("Grue") then
		NumMonsters = NumMonsters - 1
	end
end

function UnitEvent_UnitCreation_Gaia(Unit)
	if GetUnitType(Unit) == GetUnitTypeFromString("Grue") then
		NumMonsters = NumMonsters + 1
	end
end

function UnitEvent_BecomeIdle_Gaia(Unit)
end

function UnitEvent_CommandCompleted_Gaia(Unit, action, x, y, goal, arg)
end

function UnitEvent_CommandCancelled_Gaia(Unit, action, x, y, goal, arg)
end

function UnitEvent_NewCommand_Gaia(Unit, action, x, y, goal, arg)
	-- Do not call the generic function to disable AI for grues;
	-- that would cause them to _not_ get hurt while moving in
	-- light.
end

function UnitEvent_IsAttacked_Gaia(Unit, attacker)
end

function CommandUnit_TargetPos_Gaia(Unit, x, y, action, argument)
	-- ignore received commands
end

function CommandUnit_TargetUnit_Gaia(Unit, target, action, argument)
	-- ignore received commands
end
