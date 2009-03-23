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

function InitRace()

	LoadLuaScript("/data/unittypes/mainBuilding.unit")
	LoadLuaScript("/data/unittypes/tankFactory.unit")
	LoadLuaScript("/data/unittypes/barracks.unit")
			
	LoadLuaScript("/data/unittypes/solarPanel.unit")
	LoadLuaScript("/data/unittypes/surfaceGeothermal.unit")
	LoadLuaScript("/data/unittypes/deepGeothermal.unit")
			
	LoadLuaScript("/data/unittypes/smallLightTower.unit")
	LoadLuaScript("/data/unittypes/mediumLightTower.unit")
	LoadLuaScript("/data/unittypes/largeLightTower.unit")
			
	LoadLuaScript("/data/unittypes/defenseTower.unit")
	
	LoadLuaScript("/data/unittypes/builder.unit")
		
	LoadLuaScript("/data/unittypes/explorer.unit")
	LoadLuaScript("/data/unittypes/portableLightSource.unit")

	LoadLuaScript("/data/unittypes/smallAttackRobot.unit")
	LoadLuaScript("/data/unittypes/largeAttackRobot.unit")

	LoadLuaScript("/data/unittypes/smallTank.unit")
	LoadLuaScript("/data/unittypes/largeTank.unit")

	local r = {}
	r.id = "Blah"
	r.name = "Blah"
	r.description = "Bleh"
	r.luaEffectObj = "Testing"
	r.crequirements = "MainBuilding, Builder(b >= 1, e >= 4), ResearchMediumLightTower"
	r.erequirements = "MainBuilding"
	CreateResearch(r)

end			
