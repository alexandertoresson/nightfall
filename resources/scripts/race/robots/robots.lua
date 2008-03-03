
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
	r.crequirements = "MainBuilding, Builder(b >= 1, e >= 4), ResearchMediumLightTower | MainBuilding, TankFactory, !ResearchLargeLightTower"
	r.erequirements = "MainBuilding"
	CreateResearch(r)

end			
