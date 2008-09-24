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
function SetPlayers()
	AddPlayer("GAIA", PlayerType.AI, "insects", "gaia");
	AddPlayer("USER", PlayerType.Human, "USER", "human");
	for i = 2,16 do
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

	if not LoadTerrain("evenbiggerflat") then
		return false
	end
	SetMaximumBuildingAltitude(0.5)

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
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 211.5,  211.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      201.5,  201.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   206.5,  211.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   211.5,  205.5, 0, cur_player)
--	CreateUnit(GetUnitTypeFromString("LargeTank", cur_player),      cur_player, 200.5,  200.5, 0)
	
	cur_player = GetPlayerByIndex(2)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 83.5,  83.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      73.5,  73.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   78.5,  83.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   83.5,  77.5, 0, cur_player)

	cur_player = GetPlayerByIndex(3)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 47.5,  83.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      51.5,  73.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   53.5,  83.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   47.5,  77.5, 0, cur_player)

	cur_player = GetPlayerByIndex(4)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 83.5,  47.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      73.5,  51.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   78.5,  47.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   83.5,  53.5, 0, cur_player)

	cur_player = GetPlayerByIndex(5)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 175.5,  47.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      179.5,  51.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   181.5,  47.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   175.5,  53.5, 0, cur_player)

	cur_player = GetPlayerByIndex(6)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 211.5,  83.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      201.5,  73.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   206.5,  83.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   211.5,  77.5, 0, cur_player)

	cur_player = GetPlayerByIndex(7)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 175.5,  83.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      179.5,  73.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   181.5,  83.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   175.5,  77.5, 0, cur_player)

	cur_player = GetPlayerByIndex(8)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 211.5,  47.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      201.5,  51.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   206.5,  47.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   211.5,  53.5, 0, cur_player)

	cur_player = GetPlayerByIndex(9)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 47.5,  175.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      51.5,  179.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   53.5,  175.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   47.5,  181.5, 0, cur_player)

	cur_player = GetPlayerByIndex(10)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 83.5,  211.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      73.5,  201.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   78.5,  211.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   83.5,  205.5, 0, cur_player)

	cur_player = GetPlayerByIndex(11)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 47.5,  211.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      51.5,  201.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   53.5,  211.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   47.5,  205.5, 0, cur_player)

	cur_player = GetPlayerByIndex(12)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 83.5,  175.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      73.5,  179.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   78.5,  175.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   83.5,  181.5, 0, cur_player)

	cur_player = GetPlayerByIndex(13)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 175.5,  175.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      179.5,  179.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   181.5,  175.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   175.5,  181.5, 0, cur_player)

	cur_player = GetPlayerByIndex(14)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 47.5,  47.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      51.5,  51.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   53.5,  47.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   47.5,  53.5, 0, cur_player)

	cur_player = GetPlayerByIndex(15)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 175.5,  211.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      179.5,  201.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   181.5,  211.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   175.5,  205.5, 0, cur_player)

	cur_player = GetPlayerByIndex(16)
	CreateUnit(GetUnitTypeFromString("MainBuilding", cur_player), 211.5,  175.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("Builder", cur_player),      201.5,  179.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   206.5,  175.5, 0, cur_player)
	CreateUnit(GetUnitTypeFromString("SolarPanel", cur_player),   211.5,  181.5, 0, cur_player)

--	cur_player = GetPlayerByIndex(1)
--	for y = 51,91 do
--		for x = 51,91 do
--			CreateUnit(GetUnitTypeFromString("Builder", cur_player),      cur_player, x+0.5,  y+0.5, 0)
--		end
--	end

	FocusCameraOnCoord(39, 40, 20, 140)

	return true
end
