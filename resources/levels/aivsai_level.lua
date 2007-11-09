function SetPlayers()
	InitPlayers(16)
end

function InitLevel()
	--
	-- Edit as you like, but keep the order! All methods
	-- have been placed in the appropriate order. You may
	-- risk crashing the application by moving them around
	--

	---------------------------------------------------

	if not LoadHeightmap("evenbiggerflat.pgm") then
		return false
	end
	SetMaximumBuildingAltitude(0.5)

	---------------------------------------------------

	-- 1:st argument -  is the square seen?
	-- 2:nd argument -  is the square lighted?

	SetTerrainAmbientDiffuse(false, false, 0.25, 0.25, 0.25, 1.0)
	SetTerrainAmbientDiffuse(true, false, 0.35, 0.35, 0.35, 1.0)
	SetTerrainAmbientDiffuse(false, true, 0.25, 0.25, 0.25, 1.0)
	SetTerrainAmbientDiffuse(true, true, 0.6, 0.6, 0.6, 1.0)

	SetWaterAmbientDiffuse(false, false, 0.1, 0.02, 0.02, 0.66)
	SetWaterAmbientDiffuse(true, false, 0.2, 0.04, 0.04, 0.66)
	SetWaterAmbientDiffuse(false, true, 0.1, 0.02, 0.02, 0.66)
	SetWaterAmbientDiffuse(true, true, 0.4, 0.08, 0.08, 0.66)

	SetWaterColor(0.2, 0, 0)

	---------------------------------------------------

	map_texture = LoadTerrainTexture("terrain3.png")
	PrepareGUI(map_texture)
	FreeSurface(map_texture)

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
	CreateUnit(MainBuilding.pointer, cur_player, 211.5,  211.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 201.5,  201.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 206.5,  211.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 211.5,  205.5, 0)
	
--	CreateUnit(LargeTank.pointer,      cur_player, 71.5,  71.5, 0)
	cur_player = GetPlayerByIndex(2)
	CreateUnit(MainBuilding.pointer, cur_player, 83.5,  83.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 73.5,  73.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 78.5,  83.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 83.5,  77.5, 0)

	cur_player = GetPlayerByIndex(3)
	CreateUnit(MainBuilding.pointer, cur_player, 47.5,  83.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 51.5,  73.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 53.5,  83.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 47.5,  77.5, 0)

	cur_player = GetPlayerByIndex(4)
	CreateUnit(MainBuilding.pointer, cur_player, 83.5,  47.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 73.5,  51.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 78.5,  47.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 83.5,  53.5, 0)

	cur_player = GetPlayerByIndex(5)
	CreateUnit(MainBuilding.pointer, cur_player, 175.5,  47.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 179.5,  51.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 181.5,  47.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 175.5,  53.5, 0)

	cur_player = GetPlayerByIndex(6)
	CreateUnit(MainBuilding.pointer, cur_player, 211.5,  83.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 201.5,  73.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 206.5,  83.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 211.5,  77.5, 0)

	cur_player = GetPlayerByIndex(7)
	CreateUnit(MainBuilding.pointer, cur_player, 175.5,  83.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 179.5,  73.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 181.5,  83.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 175.5,  77.5, 0)

	cur_player = GetPlayerByIndex(8)
	CreateUnit(MainBuilding.pointer, cur_player, 211.5,  47.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 201.5,  51.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 206.5,  47.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 211.5,  53.5, 0)

	cur_player = GetPlayerByIndex(9)
	CreateUnit(MainBuilding.pointer, cur_player, 47.5,  175.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 51.5,  179.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 53.5,  175.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 47.5,  181.5, 0)

	cur_player = GetPlayerByIndex(10)
	CreateUnit(MainBuilding.pointer, cur_player, 83.5,  211.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 73.5,  201.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 78.5,  211.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 83.5,  205.5, 0)

	cur_player = GetPlayerByIndex(11)
	CreateUnit(MainBuilding.pointer, cur_player, 47.5,  211.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 51.5,  201.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 53.5,  211.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 47.5,  205.5, 0)

	cur_player = GetPlayerByIndex(12)
	CreateUnit(MainBuilding.pointer, cur_player, 83.5,  175.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 73.5,  179.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 78.5,  175.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 83.5,  181.5, 0)

	cur_player = GetPlayerByIndex(13)
	CreateUnit(MainBuilding.pointer, cur_player, 175.5,  175.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 179.5,  179.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 181.5,  175.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 175.5,  181.5, 0)

	cur_player = GetPlayerByIndex(14)
	CreateUnit(MainBuilding.pointer, cur_player, 47.5,  47.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 51.5,  51.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 53.5,  47.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 47.5,  53.5, 0)

	cur_player = GetPlayerByIndex(15)
	CreateUnit(MainBuilding.pointer, cur_player, 175.5,  211.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 179.5,  201.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 181.5,  211.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 175.5,  205.5, 0)

	cur_player = GetPlayerByIndex(16)
	CreateUnit(MainBuilding.pointer, cur_player, 211.5,  175.5, 0)
	CreateUnit(Builder.pointer,      cur_player, 201.5,  179.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 206.5,  175.5, 0)
	CreateUnit(SolarPanel.pointer,   cur_player, 211.5,  181.5, 0)

--	cur_player = GetPlayerByIndex(1)
--	for y = 51,91 do
--		for x = 51,91 do
--			CreateUnit(Builder.pointer,      cur_player, x+0.5,  y+0.5, 0)
--		end
--	end

	FocusCameraOnCoord(39, 40, 20, 140)

	return true
end
