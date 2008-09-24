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

	LoadLuaScript("unittypes/mainBuilding.unit")
	LoadLuaScript("unittypes/tankFactory.unit")
	LoadLuaScript("unittypes/barracks.unit")
			
	LoadLuaScript("unittypes/solarPanel.unit")
	LoadLuaScript("unittypes/surfaceGeothermal.unit")
	LoadLuaScript("unittypes/deepGeothermal.unit")
			
	LoadLuaScript("unittypes/smallLightTower.unit")
	LoadLuaScript("unittypes/mediumLightTower.unit")
	LoadLuaScript("unittypes/largeLightTower.unit")
			
	LoadLuaScript("unittypes/defenseTower.unit")
	
	LoadLuaScript("unittypes/builder.unit")
		
	LoadLuaScript("unittypes/explorer.unit")
	LoadLuaScript("unittypes/portableLightSource.unit")

	LoadLuaScript("unittypes/smallAttackRobot.unit")
	LoadLuaScript("unittypes/largeAttackRobot.unit")

	LoadLuaScript("unittypes/smallTank.unit")
	LoadLuaScript("unittypes/largeTank.unit")

	local r = {}
	r.id = "Blah"
	r.name = "Blah"
	r.description = "Bleh"
	r.luaEffectObj = "Testing"
	r.crequirements = "MainBuilding, Builder(b >= 1, e >= 4), ResearchMediumLightTower"
	r.erequirements = "MainBuilding"
	CreateResearch(r)

end			
