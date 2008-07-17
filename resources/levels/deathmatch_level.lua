function SetPlayers()
	AddPlayer("GAIA", PlayerType.AI, "insects", "gaia");
	AddPlayer("USER", PlayerType.Human, "USER", "human");
	for i = 2,3 do
		AddPlayer("USER", PlayerType.AI, "robots", "ai");
	end
	SetCurrentPlayer(1);
	SetCurrentPlayerView(1);
end

function InitLevel()
	--
	-- Edit as you like, but keep the order! All methods
	-- have been placed in the appropriate order. You may
	-- risk crashing the application by moving them around
	--

	---------------------------------------------------

	SetWaterLevel(-1.0);
	if not LoadHeightmap("deathmatch.pgm") then
		return false
	end
	SetMaximumBuildingAltitude(-0.0005)

	---------------------------------------------------
	
	SetWaterMaterial("materials/water.mat");
	SetTerrainMaterial("materials/terrain-deathmatch.mat");

	---------------------------------------------------

	InitSkybox(30, 9)

	---------------------------------------------------

	SetHourLength(10.0)
	SetDayLength(24)

	---------------------------------------------------
	day = AllocEnvironmentalCondition()

	SetHours(day, 6, 18)
	SetType(day, "day")
	SetMusicList(day, "musicDay")
	SetSkybox(day, "marsian")
	SetSunPos(day, 1024.0, 1024.0, 1024.0, 1.0)
	SetDiffuse(day, 1.0, 1.0, 1.0, 1.0)
	SetAmbient(day, 1.0, 1.0, 1.0, 1.0)
	SetFogParams(day, 12.0, 14.0, 0.15)
	SetFogColor(day, -1.0, -1.0, -1.0, -1.0)

	---------------------------------------------------
	night = AllocEnvironmentalCondition()

	SetHours(night, 18, 6)
	SetType(night, "night")
	SetMusicList(night, "musicNight")
	SetSkybox(night, "marsian_night")
	SetSunPos(night, 1024.0, 1024.0, 1024.0, 1.0)
	SetDiffuse(night, 0.2, 0.2, 0.2, 1.0)
	SetAmbient(night, 0.2, 0.2, 0.2, 1.0)
	SetFogParams(night, 8.5, 12.5, 0.4)
	SetFogColor(night, -1.0, -1.0, -1.0, -1.0)

	AddEnvironmentalCondition(day)
	AddEnvironmentalCondition(night)

	if ValidateEnvironmentalConditions() == false then
		Output ("Environmental condition validation failed!!!")
	end

	---------------------------------------------------

	SetCurrentHour(6)

	---------------------------------------------------

	return true
end
	
function InitLevelUnits()

	cur_player = GetPlayerByIndex(1)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 94.5,  94.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      102.5,  98.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   97.5,  102.5, 0, cur_player)

	cur_player = GetPlayerByIndex(2)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 162.5,  112.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      167.5,  112.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   162.5,  117.5, 0, cur_player)

	cur_player = GetPlayerByIndex(3)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 129.5,  155.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      134.5,  155.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   129.5,  150.5, 0, cur_player)

--	FocusCameraOnCoord(39, 40, 20, 140)

	return true
end
