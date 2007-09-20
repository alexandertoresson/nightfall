#ifndef __UNITINTERFACE_H_PRE__
#define __UNITINTERFACE_H_PRE__

#ifdef DEBUG_DEP
#warning "unitinterface.h-pre"
#endif

#define __UNITINTERFACE_H_PRE_END__

#include "luawrapper.h"
#include "unit.h"
#include "networking.h"

#endif

#ifdef __NETWORKING_H_END__

#ifdef __UNIT_H_END__

#ifdef __LUAWRAPPER_H_END__

#ifndef __UNITINTERFACE_H__
#define __UNITINTERFACE_H__

#ifdef DEBUG_DEP
#warning "unitinterface.h"
#endif

namespace UnitLuaInterface
{
	int LGetUnitHealth(LuaVM* pVM);
	int LGetUnitPower(LuaVM* pVM);
	int LGetUnitAction(LuaVM* pVM);
	int LGetUnitPosition(LuaVM* pVM);
	int LGetUnitRotation(LuaVM* pVM);
	int LGetUnitType(LuaVM* pVM);
	int LGetUnitOwner(LuaVM* pVM);
	int LGetUnitLastAttack(LuaVM* pVM);
	int LGetUnitLastAttacked(LuaVM* pVM);
	int LGetUnitTargetUnit(LuaVM* pVM);
	int LGetUnitTargetPos(LuaVM* pVM);

	int LSetUnitHealth(LuaVM* pVM);
	int LSetUnitPower(LuaVM* pVM);
	int LSetUnitAction(LuaVM* pVM);
	int LSetUnitPosition(LuaVM* pVM);
	int LSetUnitRotation(LuaVM* pVM);
	int LSetUnitType(LuaVM* pVM);
	int LSetUnitOwner(LuaVM* pVM);
	int LSetUnitTargetUnit(LuaVM* pVM);

	int LClearProjectiles(LuaVM* pVM);
	int LFireProjectileAtLocation(LuaVM* pVM);
	int LFireProjectileAtTarget(LuaVM* pVM);
	int LAttack(LuaVM* pVM);
	int LCanAttack(LuaVM* pVM);
	int LCalculateUnitDamage(LuaVM* pVM);
	int LCanReach(LuaVM* pVM);

	int LChangePath(LuaVM* pVM);
	int LCommandGoto(LuaVM* pVM);
	int LCommandMoveAttack(LuaVM* pVM);
	int LCommandBuild(LuaVM* pVM);
	int LCommandFollow(LuaVM* pVM);
	int LCommandAttack(LuaVM* pVM);
	int LCommandCollect(LuaVM* pVM);
	int LCommandRepair(LuaVM* pVM);
	int LCommandMoveAttackUnit(LuaVM* pVM);

	int LMove(LuaVM* pVM);
	int LClearAllActions(LuaVM* pVM);

	int LCreateUnit(LuaVM* pVM);

	int LGetUnitTypeFromString(LuaVM* pVM);

	int LGetPlayerByIndex(LuaVM* pVM);
	int LGetPlayerByName(LuaVM* pVM);
	int LInitPlayers(LuaVM* pVM);
	
	int LLoadHeightmap(LuaVM* pVM);
	int LSetHeightmapModifier(LuaVM* pVM);
	int LSetMaximumBuildingAltitude(LuaVM* pVM);
	int LLoadTerrainTexture(LuaVM* pVM);
	int LSetTerrainAmbientDiffuse(LuaVM* pVM);
	int LSetTerrainSpecular(LuaVM* pVM);
	int LSetTerrainEmission(LuaVM* pVM);
	int LSetTerrainShininess(LuaVM* pVM);

	int LSetWaterLevel(LuaVM* pVM);
	int LSetWaterHeight(LuaVM* pVM);
	int LSetWaterAmbientDiffuse(LuaVM* pVM);
	int LSetWaterSpecular(LuaVM* pVM);
	int LSetWaterEmission(LuaVM* pVM);
	int LSetWaterShininess(LuaVM* pVM);
	int LSetWaterColor(LuaVM* pVM);
	
	int LPrepareGUI(LuaVM* pVM);
	int LFreeSurface(LuaVM* pVM);
	
	int LAllocEnvironmentalCondition(LuaVM* pVM);
	int LSetHours(LuaVM* pVM);
	int LSetType(LuaVM* pVM);
	int LSetMusicList(LuaVM* pVM);
	int LSetSkybox(LuaVM* pVM);
	int LSetSunPos(LuaVM* pVM);
	int LSetDiffuse(LuaVM* pVM);
	int LSetAmbient(LuaVM* pVM);
	int LSetFogParams(LuaVM* pVM);
	int LSetFogColor(LuaVM* pVM);
	int LAddEnvironmentalCondition(LuaVM* pVM);
	int LValidateEnvironmentalConditions(LuaVM* pVM);
	int LInitSkybox(LuaVM* pVM);
	int LSetDayLength(LuaVM* pVM);
	int LSetHourLength(LuaVM* pVM);
	int LSetCurrentHour(LuaVM* pVM);

	int LFocusCameraOnUnit(LuaVM* pVM);
	int LFocusCameraOnCoord(LuaVM* pVM);
	int LRotateCamera(LuaVM* pVM);
	int LZoomCamera(LuaVM* pVM);

	void Init(void);
}

#ifdef DEBUG_DEP
#warning "unitinterface.h-end"
#endif

#define __UNITINTERFACE_H_END__

#include <sstream>

#endif

#endif

#endif

#endif
