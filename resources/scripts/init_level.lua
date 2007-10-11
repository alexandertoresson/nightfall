function DoFile(filename)
	func, err = loadfile(GetLUAScript(filename))
	if func == nil then
		Output("Error encountered while loading " .. filename .. ": " .. err .. "\n")
	else
		func()
	end
end

DoFile("ai_human.lua")
DoFile("ai_gaia.lua")
DoFile("ai_ai.lua")

InitLevel_Current = InitLevel

function LoadDefaultUnitTypes()

	LoadUnitType("unittypes/mainBuilding.unit")
	LoadUnitType("unittypes/tankFactory.unit")
	LoadUnitType("unittypes/barracks.unit")
			
	LoadUnitType("unittypes/solarPanel.unit")
	LoadUnitType("unittypes/surfaceGeothermal.unit")
	LoadUnitType("unittypes/deepGeothermal.unit")
			
	LoadUnitType("unittypes/smallLightTower.unit")
	LoadUnitType("unittypes/mediumLightTower.unit")
	LoadUnitType("unittypes/largeLightTower.unit")
			
	LoadUnitType("unittypes/defenseTower.unit")
	
	LoadUnitType("unittypes/builder.unit")
		
	LoadUnitType("unittypes/explorer.unit")
	LoadUnitType("unittypes/portableLightSource.unit")

	LoadUnitType("unittypes/smallAttackRobot.unit")
	LoadUnitType("unittypes/largeAttackRobot.unit")

	LoadUnitType("unittypes/smallTank.unit")
	LoadUnitType("unittypes/largeTank.unit")
			
	LoadUnitType("unittypes/monster.unit")

end

function InitLevel()

	SetPlayers()

	LoadDefaultUnitTypes()

	InitLevel_Current()
		
end
