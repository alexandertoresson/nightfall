#ifndef __UNITRENDER_H__
#define __UNITRENDER_H__

#ifdef DEBUG_DEP
#warning "unitrender.h"
#endif

#include "unit-pre.h"
#include "unittype-pre.h"
#include "sdlheader.h"
#include "vector3d.h"

namespace Game
{
	namespace Dimension
	{
		extern GLfloat unitMaterialAmbient[2][2][4];
		extern GLfloat unitMaterialDiffuse[2][2][4];
		extern GLfloat unitMaterialSpecular[2][2][4];
		extern GLfloat unitMaterialEmission[2][2][4];
		extern GLfloat unitMaterialShininess[2][2];

		void SetUnitCoordSpace(Unit* unit, bool ignoreCompleteness = false);
		bool DoesHitUnit(Unit* unit, int clickx, int clicky, float& distance);
		Utilities::Vector3D GetUnitWindowPos(Unit* unit);
		void SetBillBoardCoordSystem(Unit* unit);

		void RenderBuildOutline(UnitType* type, int start_x, int start_y);

		TransformData* CreateTransformData(Utilities::Vector3D pretrans, Utilities::Vector3D rot, Utilities::Vector3D aftertrans, Utilities::Vector3D scale);
		Animation* CreateAnimation(TransformAnim* transAnim);
		TransformAnim* CreateTransAnim(MorphAnim* morphAnim, TransformAnim** children, int numChildren, float length, int numKeyFrames, ...);
		MorphAnim* CreateMorphAnim(float length, int numKeyFrames, ...);

		void PrepareAnimationData(Unit* const);
	}
}

#ifdef DEBUG_DEP
#warning "unitrender.h-end"
#endif

#endif

