#include "unit.h"

#include "aipathfinding.h"
#include "luawrapper.h"
#include "effect.h"
#include "networking.h"
#include "textures.h"
#include "environment.h"
#include <cstdarg>

namespace Game
{
	namespace Dimension
	{
		vector<Unit*> unitsSelected;
		vector<Unit*> unitGroups[10];
		GLuint        genericTexture;
		Unit**        unitByID;
		Uint32*       frameRemovedAt;
		unsigned short         nextID;
		
		GLfloat    unitMaterialAmbient[2][2][4] = {
							    {
							      {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, not lighted
							      {0.4f, 0.4f, 0.4f, 1.0f}    // seen, not lighted
							    },
							    {
							      {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, lighted
							      {0.8f, 0.8f, 0.8f, 1.0f}    // seen, lighted
							    }
							  };
		
		GLfloat    unitMaterialDiffuse[2][2][4] = {
							    {
							      {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, not lighted
							      {0.4f, 0.4f, 0.4f, 1.0f}    // seen, not lighted
							    },
							    {
							      {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, lighted
							      {0.8f, 0.8f, 0.8f, 1.0f}    // seen, lighted
							    }
							  };
		
		GLfloat    unitMaterialSpecular[2][2][4] = {
							     {
							       {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, not lighted
							       {0.0f, 0.0f, 0.0f, 1.0f}    // seen, not lighted
							     },
							     {
							       {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, lighted
							       {0.0f, 0.0f, 0.0f, 1.0f}    // seen, lighted
							     }
							   };
		
		GLfloat    unitMaterialEmission[2][2][4] = {
							     {
							       {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, not lighted
							       {0.0f, 0.0f, 0.0f, 1.0f}    // seen, not lighted
							     },
							     {
							       {0.0f, 0.0f, 0.0f, 1.0f},   // not seen, lighted
							       {0.0f, 0.0f, 0.0f, 1.0f}    // seen, lighted
							     }
							   };
		
		GLfloat    unitMaterialShininess[2][2] = {
							   {
							     10.0,   // not seen, not lighted
							     10.0    // seen, not lighted
							   },
							   {
							     10.0,   // not seen, lighted
							     10.0    // seen, lighted
							   }
							 };
							 
		GLfloat unitBuildingMaximumAltitude = -0.3f;

		void PlayActionSound(Unit* unit, Audio::SoundNodeAction action)
		{
			if (unit == NULL)
				return;

			if (action > Audio::SFX_ACT_COUNT)
				return;

			Audio::AudioFXInfo* fx = unit->type->actionSounds[action];
			if (fx == NULL)
				return;

			Utilities::Vector3D groundPos = Dimension::GetTerrainCoord(unit->pos.x, unit->pos.y);
			Audio::PlayOnceFromLocation(fx->pSound, &fx->channel, &groundPos,
				Game::Rules::GameWindow::Instance()->GetCamera()->GetPosVector(),
				fx->strength);
		}
		
		void PlayRepeatingActionSound(Unit* unit, Audio::SoundNodeAction action)
		{				
			if (unit->soundNodes[action] != NULL)
				return;
				
			if (unit->type->actionSounds[action] == NULL)
				return;
			
			Audio::AudioFXInfo* inf = unit->type->actionSounds[action];
			
			unit->soundNodes[action] = Audio::CreateSoundNode(inf->pSound, unit->pos.x, unit->pos.y, 0, 0, inf->strength, -1);
			Audio::SetSpeakerUnit(unit->soundNodes[action]->value, unit);
		}
		
		void StopRepeatingActionSound(Unit* unit, Audio::SoundNodeAction action)
		{
			if (unit == NULL)
				return;
				
			if (unit->soundNodes[action] == NULL)
				return;
				
			Audio::SetSpeakerUnit(unit->soundNodes[action]->value, NULL);
			Audio::RemoveSoundNode(unit->soundNodes[action]);
			unit->soundNodes[action] = NULL;
		}
		
		void SetParticleCoordSpace(float x, float y, float z, float scale)
		{
			float unit_x, unit_y, unit_z;
			Utilities::Vector3D up_vector, normal, rotate_axis;
			unit_x = x;
			unit_z = y;

			// Translate to the position of the terrain at the position of the unit
			unit_y = GetTerrainHeight(unit_x, unit_z) + z;
			glTranslatef(unit_x * 0.125 - terrainOffsetX, unit_y, unit_z * 0.125 - terrainOffsetY);
			glScalef(0.0625*scale, 0.0625*scale, 0.0625*scale);
		}

		// Set the coordinate space of a unit, so you then can just render the unit and it will be placed at the appropriate
		// position in the landscape
		void SetUnitCoordSpace(Unit* unit, bool ignoreCompleteness /* = false */)
		{
				UnitType* type = unit->type;
				float unit_x, unit_y, unit_z, degrees_to_rotate;
				Utilities::Vector3D up_vector, normal, rotate_axis;
			
				unit_x = unit->pos.x;
				unit_z = unit->pos.y;

				if ((type->widthOnMap >> 1) << 1 == type->widthOnMap)
				{
					unit_x -= 0.5;
				}
				if ((type->heightOnMap >> 1) << 1 == type->heightOnMap)
				{
					unit_z -= 0.5;
				}
				
				// Translate to the position of the terrain at the position of the unit
				unit_y = GetTerrainHeight(unit_x, unit_z);
				glTranslatef(unit_x * 0.125 - terrainOffsetX, unit_y, unit_z * 0.125 - terrainOffsetY);

				// rotate so the unit will be placed correctly onto possibly leaning ground, by rotating by the difference between
				// the up vector and the terrain normal (get degrees to rotate by with dot product, get axis with cross product)
				up_vector.set(0.0f, 1.0f, 0.0f);
				normal = GetTerrainNormal(unit->pos.x, unit->pos.y);

				rotate_axis = up_vector;
				rotate_axis.cross(normal);

				degrees_to_rotate = acos(up_vector.dot(normal)) * (180 / PI);

				glRotatef(degrees_to_rotate, rotate_axis.x, rotate_axis.y, rotate_axis.z);

				// rotate the unit by how much it's supposed to be rotated
				glRotatef(unit->rotation, 0.0f, 1.0f, 0.0f);
				
				// scale down
				if (ignoreCompleteness)
					glScalef(0.0625*type->size, 0.0625*type->size, 0.0625*type->size);
				else
					glScalef(0.0625*type->size * unit->completeness / 100.0, 0.0625*type->size * unit->completeness / 100.0, 0.0625*type->size * unit->completeness / 100.0);
				
				// translate upwards (in the direction of the terrain normal, because of the rotation before)
				glTranslatef(0.0, 1.05, 0.0);
				
		}

		// sets the coordinate system so you can render a billboard at the current position
		// assumes that SetUnitCoordSpace has been called already
		void SetBillBoardCoordSystem(Unit *unit)
		{
			Utilities::Vector3D near_plane, far_plane, up, right, look, up_vector, rotate_axis, normal, window_coord;
			float degrees_to_rotate;
			GLfloat matrix[16];

			glRotatef(-unit->rotation, 0.0f, 1.0f, 0.0f);
			
			up_vector.set(0.0f, 1.0f, 0.0f);
			normal = GetTerrainNormal(unit->pos.x, unit->pos.y);

			rotate_axis = up_vector;
			rotate_axis.cross(normal);

			degrees_to_rotate = acos(up_vector.dot(normal)) * (180 / PI);

			glRotatef(-degrees_to_rotate, rotate_axis.x, rotate_axis.y, rotate_axis.z);

			WorldCoordToWindowCoord(Utilities::Vector3D(0, 0, 0), window_coord);

			WindowCoordToVector(window_coord.x, window_coord.y, near_plane, far_plane);

			look = near_plane;
			look.normalize();
			up.set(0.0, 1.0, 0.0);

			right = up;
			right.cross(look);

			right.normalize();

			up = look;
			up.cross(right);

			matrix[0] = right.x;
			matrix[1] = right.y;
			matrix[2] = right.z;
			matrix[3] = 0.0;

			matrix[4] = up.x;
			matrix[5] = up.y;
			matrix[6] = up.z;
			matrix[7] = 0.0;

			matrix[8] = look.x;
			matrix[9] = look.y;
			matrix[10] = look.z;
			matrix[11] = 0.0;

			matrix[12] = 0.0;
			matrix[13] = 0.0;
			matrix[14] = 0.0;
			matrix[15] = 1.0;

			glMultMatrixf((GLfloat*) &matrix);
		}

		// Set the coordinate space of a projectile, so you then can just render the projectile and it will be placed at the appropriate
		// position
		void SetProjectileCoordSpace(Projectile* proj)
		{
				ProjectileType* type = proj->type;
				float degrees_to_rotate;
				Utilities::Vector3D up_vector, rotate_axis;
				
				glTranslatef(proj->pos.x, proj->pos.y, proj->pos.z);

				// rotate so the unit will be placed correctly onto possibly leaning ground, by rotating by the difference between
				// the up vector and the terrain normal (get degrees to rotate by with dot product, get axis with cross product)
				up_vector.set(0.0f, 1.0f, 0.0f);

				rotate_axis = up_vector;
				rotate_axis.cross(proj->direction);

				degrees_to_rotate = acos(up_vector.dot(proj->direction)) * (180 / PI);

				glRotatef(degrees_to_rotate, rotate_axis.x, rotate_axis.y, rotate_axis.z);

				// scale down
				glScalef(0.0625*type->size, 0.0625*type->size, 0.0625*type->size);
				
		}

		void GetTypeUpperLeftCorner(UnitType* type, float mx, float my, int& lx, int& uy)
		{
			lx = (int) mx - (type->widthOnMap>>1);
			uy = (int) my - (type->heightOnMap>>1);
		}

		void GetUnitUpperLeftCorner(Unit* unit, int& lx, int& uy)
		{
			lx = (int) unit->pos.x - (unit->type->widthOnMap>>1);
			uy = (int) unit->pos.y - (unit->type->heightOnMap>>1);
			if (lx < 0 || uy < 0 || lx + unit->type->widthOnMap-1 >= pWorld->width || uy + unit->type->heightOnMap-1 >= pWorld->height)
			{
				cout << "CRITICAL ERROR: UNIT PLACED OUTSIDE MAP" << endl;
			}
		}

		void GetUnitUpperLeftCorner(Unit* unit, float mx, float my, int& lx, int& uy)
		{
			lx = (int) mx - (unit->type->widthOnMap>>1);
			uy = (int) my - (unit->type->heightOnMap>>1);
		}

		void GetUnitUpperLeftCorner(Unit* unit, int mx, int my, int& lx, int& uy)
		{
			lx = mx - (unit->type->widthOnMap>>1);
			uy = my - (unit->type->heightOnMap>>1);
		}

		bool WithinRange(UnitType* type, float attacker_x, float attacker_y, float target_x, float target_y, float maxrange, float minrange)
		{
			int start_x, start_y;
			float distance;
			GetTypeUpperLeftCorner(type, target_x, target_y, start_x, start_y);
			for (int y = start_y; y < start_y + type->heightOnMap; y++)
			{
				for (int x = start_x; x < start_x + type->widthOnMap; x++)
				{
					distance = sqrt(pow(attacker_x - (x + 0.5), 2) + pow(attacker_y - (y + 0.5), 2));
					if ((distance <= maxrange) && (distance >= minrange))
					{
						return true;
					}
				}
			}
			return false;
		}

		bool WithinRange(Unit* unit, float pos_x, float pos_y, float maxrange, float minrange)
		{
			return WithinRange(unit->type, pos_x, pos_y, unit->pos.x, unit->pos.y, maxrange, minrange);
		}

		bool WouldBeAbleToReach(Unit* attacker, float attacker_x, float attacker_y, Unit* target)
		{
			if(target == NULL)
			{
				return false;
			}
			return WithinRange(target, attacker_x, attacker_y, attacker->type->attackMaxRange, attacker->type->attackMinRange);
		}

		float Distance2D(float x, float y)
		{
			return sqrt(pow(x, 2) + pow(y, 2));
		}

		void GetNearestUnoccupiedPosition(UnitType* type, int& x, int& y)
		{
			int num_steps = 1;
			if (SquaresAreWalkable_AllKnowing(type, x, y))
			{
				return;
			}

			for (int j = 0; j < 10000; j++)
			{
				for (int i = 0; i < num_steps; i++)
				{
					x++;
					if (SquaresAreWalkable_AllKnowing(type, x, y))
					{
						return;
					}
				}

				for (int i = 0; i < num_steps; i++)
				{
					y++;
					if (SquaresAreWalkable_AllKnowing(type, x, y))
					{
						return;
					}
				}

				num_steps++;
				
				for (int i = 0; i < num_steps; i++)
				{
					x--;
					if (SquaresAreWalkable_AllKnowing(type, x, y))
					{
						return;
					}
				}

				for (int i = 0; i < num_steps; i++)
				{
					y--;
					if (SquaresAreWalkable_AllKnowing(type, x, y))
					{
						return;
					}
				}

				num_steps++;
			}
		}

		bool IsWithinRangeForBuilding(Unit* unit)
		{
			if (unit->type->isMobile)
			{
				if (unit->pMovementData->action.arg)
				{
					return WithinRange((UnitType*) unit->pMovementData->action.arg, unit->pos.x, unit->pos.y, unit->pMovementData->action.goal.pos.x, unit->pMovementData->action.goal.pos.y, 1.5f, 0.0f);
				}
				else
				{
					return WithinRange((UnitType*) unit->pMovementData->action.goal.unit->type, unit->pos.x, unit->pos.y, unit->pMovementData->action.goal.unit->pos.x, unit->pMovementData->action.goal.unit->pos.y, 1.5f, 0.0f);
				}
			}
			else
			{
				return true;
			}
		}

		bool IsSuitableForBuilding(UnitType* type, UnitType *test_type, Player* player, int build_x, int build_y)
		{
			int m_add, m_sub;
			int start_x, start_y, end_x, end_y;
			int changes = 0;
			bool last_val, cur_val;

			if (!SquaresAreWalkable(type, player, build_x, build_y, SIW_IGNORE_OWN_MOBILE_UNITS))
			{
				return false;
			}

			m_add = (int) floor(test_type->widthOnMap / 2.0);
			m_sub = (int) floor(test_type->widthOnMap / 2.0 - 0.5);

			GetTypeUpperLeftCorner(type, build_x, build_y, start_x, start_y);

			end_x = start_x + type->widthOnMap + m_add;
			end_y = start_y + type->heightOnMap + m_add;

			start_x -= m_sub + 1;
			start_y -= m_sub + 1;

			last_val = SquaresAreWalkable(test_type, player, start_x, start_y, SIW_IGNORE_OWN_MOBILE_UNITS);
			
			for (int x = start_x + 1; x <= end_x; x++)
			{
				cur_val = SquaresAreWalkable(test_type, player, x, start_y, SIW_IGNORE_OWN_MOBILE_UNITS);
				if (cur_val != last_val)
				{
					changes++;
				}
				last_val = cur_val;
			}
			
			last_val = SquaresAreWalkable(test_type, player, start_x, end_y, SIW_IGNORE_OWN_MOBILE_UNITS);
			
			for (int x = start_x + 1; x <= end_x; x++)
			{
				cur_val = SquaresAreWalkable(test_type, player, x, end_y, SIW_IGNORE_OWN_MOBILE_UNITS);
				if (cur_val != last_val)
				{
					changes++;
				}
				last_val = cur_val;
			}
			
			last_val = SquaresAreWalkable(test_type, player, start_x, start_y, SIW_IGNORE_OWN_MOBILE_UNITS);

			for (int y = start_y + 1; y <= end_y; y++)
			{
				cur_val = SquaresAreWalkable(test_type, player, start_x, y, SIW_IGNORE_OWN_MOBILE_UNITS);
				if (cur_val != last_val)
				{
					changes++;
				}
				last_val = cur_val;
			}
			
			last_val = SquaresAreWalkable(test_type, player, end_x, start_y, SIW_IGNORE_OWN_MOBILE_UNITS);

			for (int y = start_y + 1; y <= end_y; y++)
			{
				cur_val = SquaresAreWalkable(test_type, player, end_x, y, SIW_IGNORE_OWN_MOBILE_UNITS);
				if (cur_val != last_val)
				{
					changes++;
				}
				last_val = cur_val;
			}
			
			return changes <= 2;
		}
		
		bool IsSuitableForBuilding(UnitType* type, Player* player, int build_x, int build_y)
		{
			return (IsSuitableForBuilding(type, unitTypeMap["LargeTank"], player, build_x, build_y) && 
			        IsSuitableForBuilding(type, unitTypeMap["SmallTank"], player, build_x, build_y) &&
			        IsSuitableForBuilding(type, unitTypeMap["LargeAttackRobot"], player, build_x, build_y) &&
			        IsSuitableForBuilding(type, unitTypeMap["SmallAttackRobot"], player, build_x, build_y));
		}

		int PositionSearch_NumStepsTaken = 0;

		bool GetNearestSuitableAndLightedPosition(UnitType* type, Player* player, int& x, int& y)
		{
			int num_steps = 1;
			if (SquaresAreLightedAround(type, player, x, y) && IsSuitableForBuilding(type, player, x, y) && SquaresAreWalkable(type, player, x, y))
			{
				return true;
			}

			for (int j = 0; j < 32; j++)
			{
				for (int i = 0; i < num_steps; i++)
				{
					x++;
					if (SquaresAreLightedAround(type, player, x, y) && IsSuitableForBuilding(type, player, x, y) && SquaresAreWalkable(type, player, x, y))
					{
						return true;
					}
				}

				for (int i = 0; i < num_steps; i++)
				{
					y++;
					if (SquaresAreLightedAround(type, player, x, y) && IsSuitableForBuilding(type, player, x, y) && SquaresAreWalkable(type, player, x, y))
					{
						return true;
					}
				}

				num_steps++;
				
				for (int i = 0; i < num_steps; i++)
				{
					x--;
					if (SquaresAreLightedAround(type, player, x, y) && IsSuitableForBuilding(type, player, x, y) && SquaresAreWalkable(type, player, x, y))
					{
						return true;
					}
				}

				for (int i = 0; i < num_steps; i++)
				{
					y--;
					if (SquaresAreLightedAround(type, player, x, y) && IsSuitableForBuilding(type, player, x, y) && SquaresAreWalkable(type, player, x, y))
					{
						return true;
					}
				}

				PositionSearch_NumStepsTaken += num_steps * 4 - 2;

				num_steps++;

				if (PositionSearch_NumStepsTaken > 4096 && j > 8)
					return false;
			}
			return false;
		}

		float PercentLightedAtPosition(UnitType* type, int x, int y)
		{
			int** NumLightsOnSquare = pWorld->NumLightsOnSquare;
			int covered = 0, total = 0;
			int start_x, start_y, end_x, end_y;

			start_x = x - type->lightRange < 0 ? 0 : (int) (x - type->lightRange);
			start_y = y - type->lightRange < 0 ? 0 : (int) (y - type->lightRange);
			end_x = x + type->lightRange >= pWorld->width ? pWorld->width-1 : (int) (x + type->lightRange);
			end_y = y + type->lightRange >= pWorld->height ? pWorld->height-1 : (int) (y + type->lightRange);
			for (int ny = start_y; ny < end_y; ny++)
			{
				for (int nx = start_x; nx < end_x; nx++)
				{
					if (Distance2D(nx - x, ny - y) <= type->lightRange)
					{
						if (NumLightsOnSquare[ny][nx])
						{
							covered++;
						}
						total++;
					}
				}
			}
			return (float) covered / (float) total;
		}

		bool GetSuitablePositionForLightTower(UnitType* type, Player* player, int& x, int& y)
		{
			int num_steps = 1;
			Dimension::Environment::FourthDimension* pDimension = Dimension::Environment::FourthDimension::Instance();
			double curTime = pDimension->GetCurrentHour();
			if (curTime > 6.0 && curTime < 9.0 && strcmp(type->id, "SmallLightTower") == 0)
			{
				if (IsSuitableForBuilding(type, player, x, y) && SquaresAreWalkable(type, player, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
				{
					return true;
				}

				for (int j = 0; j < 32; j++)
				{
					for (int i = 0; i < num_steps; i++)
					{
						x++;
						if (IsSuitableForBuilding(type, player, x, y) && SquaresAreWalkable(type, player, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					for (int i = 0; i < num_steps; i++)
					{
						y++;
						if (IsSuitableForBuilding(type, player, x, y) && SquaresAreWalkable(type, player, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					num_steps++;
					
					for (int i = 0; i < num_steps; i++)
					{
						x--;
						if (IsSuitableForBuilding(type, player, x, y) && SquaresAreWalkable(type, player, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					for (int i = 0; i < num_steps; i++)
					{
						y--;
						if (IsSuitableForBuilding(type, player, x, y) && SquaresAreWalkable(type, player, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					PositionSearch_NumStepsTaken += num_steps * 4 - 2;

					num_steps++;

					if (PositionSearch_NumStepsTaken > 4096 && j > 8)
						return false;
				}
			}
			else
			{
				if (SquaresAreLightedAround(type, player, x, y) && IsSuitableForBuilding(type, player, x, y) && SquaresAreWalkable(type, player, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
				{
					return true;
				}

				for (int j = 0; j < 32; j++)
				{
					for (int i = 0; i < num_steps; i++)
					{
						x++;
						if (SquaresAreLightedAround(type, player, x, y) && IsSuitableForBuilding(type, player, x, y) && SquaresAreWalkable(type, player, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					for (int i = 0; i < num_steps; i++)
					{
						y++;
						if (SquaresAreLightedAround(type, player, x, y) && IsSuitableForBuilding(type, player, x, y) && SquaresAreWalkable(type, player, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					num_steps++;
					
					for (int i = 0; i < num_steps; i++)
					{
						x--;
						if (SquaresAreLightedAround(type, player, x, y) && IsSuitableForBuilding(type, player, x, y) && SquaresAreWalkable(type, player, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					for (int i = 0; i < num_steps; i++)
					{
						y--;
						if (SquaresAreLightedAround(type, player, x, y) && IsSuitableForBuilding(type, player, x, y) && SquaresAreWalkable(type, player, x, y) && PercentLightedAtPosition(type, x, y) < 0.66)
						{
							return true;
						}
					}

					PositionSearch_NumStepsTaken += num_steps * 4 - 2;

					num_steps++;
					
					if (PositionSearch_NumStepsTaken > 4096 && j > 8)
						return false;
				}
			}
			return false;
		}

		double GetIncomeAtNoon(Player* player)
		{
			float income = 0;
			for (vector<Unit*>::iterator it = player->vUnits.begin(); it != player->vUnits.end(); it++)
			{
				UnitType* unittype = (*it)->type;
				income += unittype->powerIncrement;
				income -= unittype->powerUsage + unittype->lightPowerUsage + (unittype->attackPowerUsage + unittype->movePowerUsage + unittype->buildPowerUsage) * 0.5;
			}
			return income;
		}

		double GetIncomeAtNight(Player* player)
		{
			float income = 0;
			for (vector<Unit*>::iterator it = player->vUnits.begin(); it != player->vUnits.end(); it++)
			{
				UnitType* unittype = (*it)->type;
				if (unittype->powerType == POWERTYPE_TWENTYFOURSEVEN)
				{
					income += unittype->powerIncrement;
				}
				income -= unittype->powerUsage + unittype->lightPowerUsage + (unittype->attackPowerUsage + unittype->movePowerUsage + unittype->buildPowerUsage) * 0.5;
			}
			return income;
		}

		double GetNightLength()
		{
			Dimension::Environment::FourthDimension* pDimension = Dimension::Environment::FourthDimension::Instance();
			return 12 * pDimension->GetHourLength();
		}

		double GetDayLength()
		{
			Dimension::Environment::FourthDimension* pDimension = Dimension::Environment::FourthDimension::Instance();
			return 12 * pDimension->GetHourLength();
		}

		double GetPower(Player* player)
		{
			return player->resources.power;
		}

		double GetMoney(Player* player)
		{
			return player->resources.money;
		}

		void SellPower(Player* player, double amount)
		{
			if (amount > player->resources.power)
				amount = player->resources.power;
			player->resources.money += amount;
			player->resources.power -= amount;
			player->oldResources.money += amount;
			player->oldResources.power -= amount;
		}

		double GetPowerAtDawn(Player* player)
		{
			Dimension::Environment::FourthDimension* pDimension = Dimension::Environment::FourthDimension::Instance();
			double curPower = player->resources.power;
			double curTime = pDimension->GetCurrentHour();
			double baseIncrease = GetIncomeAtNight(player);
			if (curTime >= 6.0 && curTime < 18.0)
			{
				curPower = GetPowerAtDusk(player);
				curTime = 18.0;
			}
			double timeUntilDawn;
			if (curTime < 6.0)
			{
				timeUntilDawn = 6.0 - curTime;
			}
			else
			{
				timeUntilDawn = 6.0 + 24.0 - curTime;
			}
			curPower += baseIncrease * timeUntilDawn * pDimension->GetHourLength();
			return curPower;
		}

		double GetPowerAtDusk(Player* player)
		{
			Dimension::Environment::FourthDimension* pDimension = Dimension::Environment::FourthDimension::Instance();
			double curPower = player->resources.power;
			double curTime = pDimension->GetCurrentHour();
			double baseIncrease = GetIncomeAtNight(player);
			double solarIncrease = GetIncomeAtNoon(player) - baseIncrease;
			if (curTime < 6.0 || curTime >= 18.0)
			{
				curPower = GetPowerAtDawn(player);
				curTime = 6.0;
			}
			double step = 0.01;
			for (double time = curTime; time < 18.0; time += step)
			{
				curPower += (baseIncrease + solarIncrease * (pow(sin((time-6.0)/12*PI), 1.0/3) * 0.8 + 0.2)) * step * pDimension->GetHourLength();
			}
			return curPower;
		}

		bool IsOutsideOfBuildingPlace(Unit* unit, UnitType* build_type, int build_x, int build_y, int x, int y)
		{
			int start_x_u, start_y_u;
			int start_x_t, start_y_t;
			GetUnitUpperLeftCorner(unit, x, y, start_x_u, start_y_u);
			GetTypeUpperLeftCorner(build_type, build_x, build_y, start_x_t, start_y_t);
			for (int ny = start_y_u; ny < start_y_u + unit->type->heightOnMap; ny++)
			{
				for (int nx = start_x_u; nx < start_x_u + unit->type->widthOnMap; nx++)
				{
					if (nx >= start_x_t && nx < start_x_t + build_type->widthOnMap &&
					    ny >= start_y_t && ny < start_y_t + build_type->heightOnMap)
					{
						return false;
					}
				}
			}
			return true;
		}

		void NearestSquareFromBuildingPlace(Unit* unit, UnitType* build_type, int build_x, int build_y, int &x, int &y)
		{
			int num_steps = 1;
			x = build_x;
			y = build_y;
			if (IsOutsideOfBuildingPlace(unit, build_type, build_x, build_y, x, y) && SquaresAreWalkable(unit, x, y, SIW_IGNORE_MOVING))
			{
				return;
			}

			for (int j = 0; j < 10000; j++)
			{
				for (int i = 0; i < num_steps; i++)
				{
					x++;
					if (IsOutsideOfBuildingPlace(unit, build_type, build_x, build_y, x, y) && SquaresAreWalkable(unit, x, y, SIW_IGNORE_MOVING))
					{
						return;
					}
				}

				for (int i = 0; i < num_steps; i++)
				{
					y++;
					if (IsOutsideOfBuildingPlace(unit, build_type, build_x, build_y, x, y) && SquaresAreWalkable(unit, x, y, SIW_IGNORE_MOVING))
					{
						return;
					}
				}

				num_steps++;
				
				for (int i = 0; i < num_steps; i++)
				{
					x--;
					if (IsOutsideOfBuildingPlace(unit, build_type, build_x, build_y, x, y) && SquaresAreWalkable(unit, x, y, SIW_IGNORE_MOVING))
					{
						return;
					}
				}

				for (int i = 0; i < num_steps; i++)
				{
					y--;
					if (IsOutsideOfBuildingPlace(unit, build_type, build_x, build_y, x, y) && SquaresAreWalkable(unit, x, y, SIW_IGNORE_MOVING))
					{
						return;
					}
				}

				num_steps++;
			}
		}

		void FacePos(Unit* unit, Position pos)
		{
			Utilities::Vector3D direction, zero_rot;
			direction.set(pos.x - unit->pos.x, 0.0, pos.y - unit->pos.y);
			direction.normalize();
			zero_rot.set(-1.0, 0.0, 0.0);
			unit->rotation = acos(zero_rot.dot(direction)) * (180 / PI);
			if (direction.z < 0) unit->rotation = 180 - unit->rotation + 180;
		}

		void FaceUnit(Unit* unit, Unit* targetUnit)
		{
			FacePos(unit, targetUnit->pos);
		}

		void Incomplete(Unit* unit)
		{
			UpdateLightedSquares(unit, (int) unit->pos.x, (int) unit->pos.y, 0);
			UpdateSeenSquares(unit, (int) unit->pos.x, (int) unit->pos.y, 0);
		}

		void Complete(Unit* unit)
		{
			UpdateLightedSquares(unit, (int) unit->pos.x, (int) unit->pos.y, 1);
			UpdateSeenSquares(unit, (int) unit->pos.x, (int) unit->pos.y, 1);
		}

		void Build(Unit* unit)
		{
			double build_cost;
			double power_usage =  unit->type->buildPowerUsage / AI::aiFps;
			UnitType* build_type = (UnitType*) unit->pMovementData->action.arg;

			if (unit->owner->resources.power < power_usage)
			{
				return;
			}

			unit->owner->resources.power -= power_usage;

			if (!unit->pMovementData->action.goal.unit)
			{
				if (!build_type->isResearched[unit->owner->index])
				{
					AI::CancelAction(unit);
					return;
				}

				if (unit->type->isMobile)
				{
					unit->pMovementData->action.goal.unit = CreateUnit(build_type, unit->owner, unit->pMovementData->action.goal.pos.x, unit->pMovementData->action.goal.pos.y);
					if (unit->pMovementData->action.goal.unit)
						Incomplete(unit->pMovementData->action.goal.unit);
				}
				else
				{
					unit->pMovementData->action.goal.unit = CreateUnitNoDisplay(build_type, unit->owner);
				}
				if (unit->pMovementData->action.goal.unit)
				{
					unit->pMovementData->action.goal.unit->completeness = 0.0;
					unit->pMovementData->action.goal.unit->isCompleted = false;
					unit->pMovementData->action.goal.unit->health = 0.0;
				}
				else
				{
					Unit* cur_unit = pppElements[(int)unit->pMovementData->action.goal.pos.y][(int)unit->pMovementData->action.goal.pos.x];

					if (cur_unit && cur_unit->type == build_type)
					{
						unit->pMovementData->action.goal.unit = cur_unit;
//						cout << "assign" << endl;
						AI::SendUnitEventToLua_BuildCancelled(unit);
					}
					else
					{
						if (SquaresAreWalkable(build_type, unit->owner, (int) unit->pMovementData->action.goal.pos.x, (int) unit->pMovementData->action.goal.pos.y, SIW_IGNORE_OWN_MOBILE_UNITS))
						{
							int start_x, start_y;
							GetTypeUpperLeftCorner(build_type, (int)unit->pMovementData->action.goal.pos.x, (int)unit->pMovementData->action.goal.pos.y, start_x, start_y);
							Uint32 curtime = SDL_GetTicks();
							for (int y = start_y; y < start_y + build_type->heightOnMap; y++)
							{
								for (int x = start_x; x < start_x + build_type->widthOnMap; x++)
								{
									if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height && pppElements[y][x] && !pppElements[y][x]->isMoving)
									{
										if ((curtime - pppElements[y][x]->lastCommand) / 1000.0 > 1.0)
										{
											int goto_x, goto_y;
											NearestSquareFromBuildingPlace(pppElements[y][x], build_type, (int)unit->pMovementData->action.goal.pos.x, (int)unit->pMovementData->action.goal.pos.y, goto_x, goto_y);
	//										cout << "command " << pppElements[y][x]->type->id << endl;
											CommandUnit(pppElements[y][x], goto_x+0.5, goto_y+0.5, AI::ACTION_GOTO, unit->pMovementData->action.arg, true, true);
										}
									}
								}
							}
//							cout << "wait" << endl;
						}
						else
						{
//							cout << "cancel" << endl;
							AI::CancelAction(unit);
						}
						return;
					}
				}
			}

			if (unit->pMovementData->action.goal.unit->action == AI::ACTION_DIE)
			{
				AI::CancelAction(unit);
				return;
			}

			if (unit->type->isMobile)
			{
				FaceUnit(unit, unit->pMovementData->action.goal.unit);
			}

			build_cost = unit->pMovementData->action.goal.unit->type->buildCost / (unit->pMovementData->action.goal.unit->type->buildTime * AI::aiFps);

			if (unit->owner->resources.money < build_cost)
			{
				return;
			}

#ifdef CHECKSUM_DEBUG
			Networking::checksum_output << "BUILD " << AI::currentFrame << ": " << unit->id << " " << unit->pMovementData->action.goal.unit->id << endl;
#endif

			unit->owner->resources.money -= build_cost;

			unit->pMovementData->action.goal.unit->completeness += 100.0 / (unit->pMovementData->action.goal.unit->type->buildTime * AI::aiFps);
			unit->pMovementData->action.goal.unit->health += unit->pMovementData->action.goal.unit->type->maxHealth / (unit->pMovementData->action.goal.unit->type->buildTime * AI::aiFps);

			if (unit->pMovementData->action.goal.unit->health >= unit->pMovementData->action.goal.unit->type->maxHealth)
			{
				unit->pMovementData->action.goal.unit->health = unit->pMovementData->action.goal.unit->type->maxHealth;
			}

			if (unit->pMovementData->action.goal.unit->completeness >= 100.0)
			{
				unit->pMovementData->action.goal.unit->completeness = 100.0;
				unit->pMovementData->action.goal.unit->isCompleted = true;
				if (!unit->pMovementData->action.goal.unit->isDisplayed)
				{
					int new_x = (int) unit->pos.x, new_y = (int) unit->pos.y;
					GetNearestUnoccupiedPosition(unit->pMovementData->action.goal.unit->type, new_x, new_y);
					DisplayUnit(unit->pMovementData->action.goal.unit, new_x, new_y);
				}
				else
				{
					Complete(unit->pMovementData->action.goal.unit);
				}

				if (unit->rallypoint != NULL)
				{
					AI::CommandUnit(unit->pMovementData->action.goal.unit, unit->rallypoint->x, unit->rallypoint->y, AI::ACTION_GOTO, NULL, true, true);
				}
				
				AI::CompleteAction(unit);
			}

		}
		
		void Research(Unit* unit)
		{
			double research_cost;
			double power_usage = unit->type->buildPowerUsage / AI::aiFps;
			UnitType* research_type = (UnitType*) unit->pMovementData->action.arg;

			if ((research_type->isBeingResearchedBy[unit->owner->index] && research_type->isBeingResearchedBy[unit->owner->index] != unit) || research_type->isResearched[unit->owner->index])
			{
				AI::CancelAction(unit);
				return;
			}

			research_type->isBeingResearchedBy[unit->owner->index] = unit;

			if (unit->owner->resources.power < power_usage)
			{
				return;
			}

			unit->owner->resources.power -= power_usage;

			research_cost = research_type->researchCost / (research_type->researchTime * AI::aiFps);

			if (unit->owner->resources.money < research_cost)
			{
				return;
			}

#ifdef CHECKSUM_DEBUG
			Networking::checksum_output << "RESEARCH " << AI::currentFrame << ": " << unit->id << endl;
#endif

			unit->owner->resources.money -= research_cost;

			unit->action_completeness += 100.0 / (research_type->buildTime * AI::aiFps);

			if (unit->action_completeness >= 100.0)
			{
				research_type->isBeingResearchedBy[unit->owner->index] = NULL;
				research_type->isResearched[unit->owner->index] = true;
				AI::CompleteAction(unit);
			}
			
		}
		
		void CancelBuild(Dimension::Unit* pUnit)
		{	
			if (pUnit == NULL)
				return;

			if (pUnit->pMovementData == NULL)
				return;

			if (pUnit->pMovementData->action.arg == NULL)
				return;

			float cost;
			Dimension::UnitType* build_type = (Dimension::UnitType*) pUnit->pMovementData->action.arg;

			if (pUnit->pMovementData->action.goal.unit && pUnit->pMovementData->action.goal.unit->action != AI::ACTION_DIE)
			{
				Unit* target = pUnit->pMovementData->action.goal.unit;
				cost = build_type->buildCost;
				pUnit->owner->resources.money += cost * target->completeness / 200;
				// The following is needed because DeleteUnit() expects the unit to be complete and visible.
				if (!pUnit->pMovementData->action.goal.unit->isDisplayed)
				{
					int new_x = (int) pUnit->pos.x, new_y = (int) pUnit->pos.y;
					GetNearestUnoccupiedPosition(target->type, new_x, new_y);
					DisplayUnit(target, new_x, new_y);
				}
				else
				{
					Complete(target);
				}
				pUnit->pMovementData->action.goal.unit = NULL; // Zero out target unit before calling DeleteUnit,
				                                        // to prevent DeleteUnit from calling CancelAction which calls
				                                        // CancelBuild which calls DeleteUnit which calls CancelAction....
				DeleteUnit(target);
			}
			AI::SendUnitEventToLua_BuildCancelled(pUnit);
		}

		void CancelResearch(Dimension::Unit* pUnit)
		{
			float cost;
			Dimension::UnitType* research_type = (Dimension::UnitType*) pUnit->pMovementData->action.arg;
			cost = research_type->researchCost;
			pUnit->owner->resources.money += cost * pUnit->action_completeness / 200;
			AI::SendUnitEventToLua_ResearchCancelled(pUnit);
		}

		// returns true when the unit can attack at the current time
		bool CanAttack(Unit* attacker)
		{
			if (AI::currentFrame - attacker->lastAttack >= (AI::aiFps / attacker->type->attackSpeed))
			{
				attacker->lastAttack = AI::currentFrame;
				return true;
			}
			return false;
		}

		// perform an attack at target, returning true if the target has been eliminated
		bool Attack(Unit* target, float damage)
		{
#ifdef CHECKSUM_DEBUG
			Networking::checksum_output << "DAMAGE " << AI::currentFrame << ": " << target->id << " " << damage << endl;
#endif
			if (target->health > 1e-3)
			{
				target->lastAttacked = AI::currentFrame; // only update time of last attack if the unit is not already dead
			}
			target->health -= damage;
			if (target->health <= 1e-3 && target->action != AI::ACTION_DIE)
			{
				target->health = 0;
				target->action = AI::ACTION_DIE;
				PlayActionSound(target, Audio::SFX_ACT_DEATH_FNF);
				//Start a explosion
				if(UnitIsVisible(target, Dimension::currentPlayerView))
				{
					FX::pParticleSystems->InitEffect(target->pos.x, target->pos.y, 0.0f, target->type->size, FX::PARTICLE_SPHERICAL_EXPLOSION);
				}
			}
			if (target->action == AI::ACTION_DIE)
			{
				return true;
			}
			return false;
		}

		Uint16 a_seed = 23467;
		float attack_rand()
		{
			a_seed = (a_seed + 82473) ^ 53672;
			return (a_seed / 65535.0);
		}

		// calculate how much damage a unit does
		float CalcUnitDamage(Unit* unit)
		{
			return unit->type->minAttack + (unit->type->maxAttack - unit->type->minAttack) * attack_rand();
		}

		void InitiateAttack(Unit* attacker, Unit* target)
		{
			Position goto_pos;
			Utilities::Vector3D goal_pos;	
			double power_usage =  attacker->type->attackPowerUsage;
			
			if (attacker->owner->resources.power < power_usage)
			{
				return;
			}

			attacker->owner->resources.power -= power_usage;

			if (attacker->type->isMobile)
			{
				FaceUnit(attacker, target);
			}
			if (!attacker->type->projectileType)
			{
				if (Attack(target, CalcUnitDamage(attacker)))
				{
					AI::CompleteAction(attacker);
				}
			}
			else
			{
				goto_pos = target->pos;
				goal_pos = Dimension::GetTerrainCoord(goto_pos.x, goto_pos.y);
				goal_pos.y += target->type->height * 0.25 * 0.0625;
				Projectile *proj = CreateProjectile(attacker->type->projectileType, Dimension::GetTerrainCoord(attacker->pos.x, attacker->pos.y), goal_pos);
				proj->goalUnit = target;
				attacker->projectiles.push_back(proj);
				PlayActionSound(attacker, Audio::SFX_ACT_FIRE_FNF);
			}
		}

		void HandleProjectiles(Unit* pUnit)
		{
			Projectile *proj;
			if (pUnit->projectiles.size())
			{
				for (unsigned index = 0; index < pUnit->projectiles.size(); )
				{
					proj = pUnit->projectiles.at(index);
					if (proj->pos.distance(proj->goalPos) < proj->type->speed * (1.0 / AI::aiFps))
					{
						if (Attack(proj->goalUnit, CalcUnitDamage(pUnit)))
						{
							AI::CompleteAction(pUnit);
						}
						pUnit->projectiles.erase(pUnit->projectiles.begin()+index);
					}
					else
					{
						proj->pos += proj->direction * proj->type->speed * (1.0 / AI::aiFps);
						index++;
					}
				}
			}
		}
		
		// returns true if attacker can reach target with an attack
		bool CanReach(Unit* attacker, Unit* target)
		{
			return WouldBeAbleToReach(attacker, attacker->pos.x, attacker->pos.y, target);
		}

		bool CanSee(Unit* attacker, Unit* target)
		{
			return WithinRange(target, attacker->pos.x, attacker->pos.y, attacker->type->sightRange, 0.0f);
		}

		int calls = 0;

		Unit* GetNearestUnitInRange(Unit* unit, RangeType rangeType, PlayerState state)
		{
			Unit* bestUnit = NULL;
			float bestDistance = 1000000.0, distance;
			Unit* curUnit;
			vector<Unit*>* units;
			
			for (vector<Player*>::iterator it = pWorld->vPlayers.begin(); it != pWorld->vPlayers.end(); it++)
			{
				Player *owner = *it;
				if (unit->owner->states[owner->index] & state)
				{
					units = &owner->vUnits;
					for (vector<Unit*>::iterator it_unit = units->begin(); it_unit != units->end(); it_unit++)
					{
						curUnit = *it_unit;
						if (curUnit != unit)
						{
							bool found = false;
							if (rangeType == RANGE_SIGHT)
							{
								if (CanSee(unit, curUnit))
								{
									found = true;
								}
							}
							else if (rangeType == RANGE_ATTACK)
							{
								if (CanReach(unit, curUnit))
								{
									found = true;
								}
							}
							if (found)
							{
								distance = Distance2D(curUnit->pos.x - unit->pos.x, curUnit->pos.y - unit->pos.y);
								if (distance < bestDistance)
								{
									bestUnit = curUnit;
									bestDistance = distance;
								}
							}
						}
					}
				}
			}
			return bestUnit;
		}

		bool UnitIsRendered(Unit *unit, Player *player)
		{
			int start_x, start_y;
			int** NumUnitsSeeingSquare = player->NumUnitsSeeingSquare;
			GetUnitUpperLeftCorner(unit, start_x, start_y);
			for (int y = start_y; y < start_y + unit->type->heightOnMap; y++)
			{
				for (int x = start_x; x < start_x + unit->type->widthOnMap; x++)
				{
					if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
					{
						if (BigSquareIsRendered(x, y) && NumUnitsSeeingSquare[y][x])
						{
							return true;
						}
					}
				}
			}
			return false;
		}

		float GetLightAmountOnUnit(Unit* unit)
		{
			float temp = 0.0;
			int num = 0;
			int start_x, start_y;
			bool is_lighted;
			GetUnitUpperLeftCorner(unit, start_x, start_y);
			for (int y = start_y; y < start_y + unit->type->heightOnMap; y++)
			{
				for (int x = start_x; x < start_x + unit->type->widthOnMap; x++)
				{
					if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
					{
						is_lighted = pWorld->NumLightsOnSquare[y][x] > 0 ? 1 : 0;
						temp += is_lighted;
						num++;
					}
				}
			}
			return temp / num;
		}

		bool UnitIsVisible(Unit *unit, Player *player)
		{
			int start_x, start_y;
			int** NumUnitsSeeingSquare = player->NumUnitsSeeingSquare;
			GetUnitUpperLeftCorner(unit, start_x, start_y);
			for (int y = start_y; y < start_y + unit->type->heightOnMap; y++)
			{
				for (int x = start_x; x < start_x + unit->type->widthOnMap; x++)
				{
					if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
					{
						if (NumUnitsSeeingSquare[y][x])
						{
							return true;
						}
					}
				}
			}
			return false;
		}

		inline bool SquareIsVisible_UnGuarded(Player *player, int x, int y)
		{
			return player->NumUnitsSeeingSquare[y][x];
		}

		bool SquareIsVisible(Player *player, int x, int y)
		{
			if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
			{
				return player->NumUnitsSeeingSquare[y][x];
			}
			else
			{
				return false;
			}
		}

		inline bool MovementTypeCanWalkOnSquare_UnGuarded(MovementType mType, int x, int y)
		{
			int steepness = pWorld->ppSteepness[y][x];
			float height = HeightMipmaps[0][0].ppHeights[y][x];
			switch (mType)
			{
				case MOVEMENT_HUMAN:
					return steepness < 72 && height > waterLevel;
					break;
				case MOVEMENT_VEHICLE:
					return steepness < 63 && height > waterLevel;
					break;
				case MOVEMENT_TANK:
					return steepness < 54 && height > waterLevel;
					break;
				case MOVEMENT_BUILDING:
					return steepness < 51 && height > waterLevel && height < unitBuildingMaximumAltitude;
					break;
				default:
					return false;
					break;
			}
		}

		bool MovementTypeCanWalkOnSquare(MovementType mType, int x, int y)
		{
			if (x < 0 || y < 0 || x >= pWorld->width || y >= pWorld->height)
			{
				return false;
			}
			return MovementTypeCanWalkOnSquare_UnGuarded(mType, x, y);
		}

		inline bool UnitTypeCanWalkOnSquare(UnitType* type, int x, int y)
		{
			return MovementTypeCanWalkOnSquare(type->movementType, x, y);
		}

		inline bool UnitTypeCanWalkOnSquare_UnGuarded(UnitType* type, int x, int y)
		{
			return MovementTypeCanWalkOnSquare_UnGuarded(type->movementType, x, y);
		}

		inline bool UnitTypeCanWalkOnSquares(UnitType* type, int pos_x, int pos_y)
		{
			int start_x, start_y;
			GetTypeUpperLeftCorner(type, pos_x, pos_y, start_x, start_y);
			for (int y = start_y; y < start_y + type->heightOnMap; y++)
			{
				for (int x = start_x; x < start_x + type->widthOnMap; x++)
				{
					if (!UnitTypeCanWalkOnSquare(type, x, y))
					{
						return false;
					}
				}
			}
			return true;
		}

		inline bool SquareIsWalkable_Internal(Unit *unit, UnitType *type, Player *player, int x, int y, int flags)
		{
			bool walkable;
			if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
			{
				walkable = UnitTypeCanWalkOnSquare_UnGuarded(type, x, y);
				if (walkable && (flags & SIW_ALLKNOWING || SquareIsVisible_UnGuarded(player, x, y)))
				{
					if (pppElements[y][x] == NULL || pppElements[y][x] == unit)
					{
						return true;
					}
					if (flags & SIW_IGNORE_MOVING && pppElements[y][x]->isMoving)
					{
						return true;
					}
					if (flags & SIW_IGNORE_OWN_MOBILE_UNITS && pppElements[y][x]->owner == player && pppElements[y][x]->type->isMobile)
					{
						return true;
					}
					return false;
				}
				else
				{
					return walkable;
				}
			}
			return false;
		}

		inline bool SquareIsWalkable(Unit *unit, int x, int y, int flags)
		{
			return SquareIsWalkable_Internal(unit, unit->type, unit->owner, x, y, flags);
		}

		inline bool SquareIsWalkable(Unit *unit, int x, int y)
		{
			return SquareIsWalkable_Internal(unit, unit->type, unit->owner, x, y, SIW_DEFAULT);
		}

		inline bool SquareIsWalkable_AllKnowing(Unit *unit, int x, int y)
		{
			return SquareIsWalkable_Internal(unit, unit->type, unit->owner, x, y, SIW_ALLKNOWING);
		}

		inline bool SquareIsWalkable(UnitType *type, Player *player, int x, int y, int flags)
		{
			return SquareIsWalkable_Internal(NULL, type, player, x, y, flags);
		}

		inline bool SquareIsWalkable(UnitType *type, Player *player, int x, int y)
		{
			return SquareIsWalkable_Internal(NULL, type, player, x, y, SIW_DEFAULT);
		}

		inline bool SquareIsWalkable_AllKnowing(UnitType *type, int x, int y)
		{
			return SquareIsWalkable_Internal(NULL, type, NULL, x, y, SIW_ALLKNOWING);
		}

		bool SquaresAreWalkable(Unit *unit, int x, int y, int flags)
		{
			int start_x, start_y;
			int end_x, end_y;
			GetUnitUpperLeftCorner(unit, x, y, start_x, start_y);
			end_x = start_x + unit->type->widthOnMap;
			end_y = start_y + unit->type->heightOnMap;
			for (int ny = start_y; ny < end_y; ny++)
			{
				for (int nx = start_x; nx < end_x; nx++)
				{
					if (!SquareIsWalkable(unit, nx, ny, flags))
					{
						return false;
					}
				}
			}
			return true;
		}

		bool SquaresAreWalkable(Unit *unit, int x, int y)
		{
			return SquaresAreWalkable(unit, x, y, SIW_DEFAULT);
		}

		bool SquaresAreWalkable(UnitType *type, Player *player, int x, int y, int flags)
		{
			int start_x, start_y;
			GetTypeUpperLeftCorner(type, x, y, start_x, start_y);
			for (int ny = start_y; ny < start_y + type->heightOnMap; ny++)
			{
				for (int nx = start_x; nx < start_x + type->widthOnMap; nx++)
				{
					if (!SquareIsWalkable(type, player, nx, ny, flags))
					{
						return false;
					}
				}
			}
			return true;
		}

		inline bool SquaresAreWalkable(UnitType *type, Player *player, int x, int y)
		{
			return SquaresAreWalkable(type, player, x, y, SIW_DEFAULT);
		}

		inline bool SquaresAreWalkable_AllKnowing(Unit *unit, int x, int y)
		{
			return SquaresAreWalkable(unit, x, y, SIW_ALLKNOWING);
		}

		bool SquaresAreWalkable_AllKnowing(UnitType *type, int x, int y)
		{
			return SquaresAreWalkable(type, NULL, x, y, SIW_ALLKNOWING);
		}

		bool SquareIsLighted(Player *player, int x, int y)
		{
			if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
			{
				if (player->NumUnitsSeeingSquare[y][x])
				{
					return pWorld->NumLightsOnSquare[y][x] != 0;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}

		bool SquaresAreLighted(UnitType *type, Player *player, int x, int y)
		{
			int start_x, start_y;
			GetTypeUpperLeftCorner(type, x, y, start_x, start_y);
			for (int ny = start_y; ny < start_y + type->heightOnMap; ny++)
			{
				for (int nx = start_x; nx < start_x + type->widthOnMap; nx++)
				{
					if (!SquareIsLighted(player, nx, ny))
					{
						return false;
					}
				}
			}
			return true;
		}

		bool SquaresAreLightedAround(UnitType *type, Player *player, int x, int y)
		{
			int start_x, start_y;
			GetTypeUpperLeftCorner(type, x, y, start_x, start_y);
			start_x--;
			start_y--;
			for (int ny = start_y; ny < start_y + type->heightOnMap + 2; ny++)
			{
				for (int nx = start_x; nx < start_x + type->widthOnMap + 2; nx++)
				{
					if (!SquareIsLighted(player, nx, ny))
					{
						return false;
					}
				}
			}
			return true;
		}

		bool SquareIsGoal(Unit *unit, int x, int y, bool use_internal)
		{
			Unit*       target = NULL;
			Position   pos;
			void*       arg;
			AI::Node*   start;
			AI::Node*   goal;
			
			if (!use_internal)
			{
				target = unit->pMovementData->action.goal.unit;
				pos    = unit->pMovementData->action.goal.pos;
				arg    = unit->pMovementData->action.arg;
				
				start  = unit->pMovementData->pStart;
				goal   = unit->pMovementData->pGoal;
			}
			else
			{
				target = unit->pMovementData->_action.goal.unit;
				pos    = unit->pMovementData->_action.goal.pos;
				arg    = unit->pMovementData->_action.arg;
				
				start  = unit->pMovementData->_start;
				goal   = unit->pMovementData->_goal;
			}
		
			if(target)
			{
				if (unit->action == AI::ACTION_ATTACK || unit->action == AI::ACTION_MOVE_ATTACK_UNIT)
				{
					return WouldBeAbleToReach(unit, x+0.5, y+0.5, target);
				}
				else if (unit->action == AI::ACTION_FOLLOW)
				{
					return WithinRange(target->type, x, y, target->pos.x, target->pos.y, 1.5f, 0.0f);
				}
				else if (unit->action == AI::ACTION_BUILD)
				{
					if (arg)
					{
						return WithinRange((UnitType*) arg, x, y, pos.x, pos.y, 1.5f, 0.0f);
					}
					else
					{
						return WithinRange(target->type, x, y, target->pos.x, target->pos.y, 1.5f, 0.0f);
					}
				}
				else
				{
					return (x == goal->x) && (y == goal->y);
				}
			}
			else
			{
				return (x == goal->x) && (y == goal->y);
			}
		}		

		// operation is 0 for removing seen squares, 1 for adding seen squares.
		void UpdateSeenSquares(Unit* unit, int x, int y, int operation)
		{
			int start_x, start_y, end_x, end_y;
			int** NumUnitsSeeingSquare = unit->owner->NumUnitsSeeingSquare;
			
			if (operation == 0 && unit->hasSeen == false)
			{
				cout << "SEEN SQUARES MANAGEMENT WARNING: Attempted to delete seen squares twice" << endl;
				return;
			}
			else if (operation == 1 && unit->hasSeen == true)
			{
				cout << "SEEN SQUARES MANAGEMENT WARNING: Attempted to add seen squares twice" << endl;
				return;
			}

			unit->hasSeen = operation == 1 ? true : false;

			start_x = x - unit->type->sightRange < 0 ? 0 : (int) (x - unit->type->sightRange);
			start_y = y - unit->type->sightRange < 0 ? 0 : (int) (y - unit->type->sightRange);
			end_x = x + unit->type->sightRange >= pWorld->width ? pWorld->width-1 : (int) (x + unit->type->sightRange);
			end_y = y + unit->type->sightRange >= pWorld->height ? pWorld->height-1 : (int) (y + unit->type->sightRange);
			for (int ny = start_y; ny < end_y; ny++)
			{
				for (int nx = start_x; nx < end_x; nx++)
				{
					if (Distance2D(nx - x, ny - y) <= unit->type->sightRange)
					{
						if (operation)
						{
							NumUnitsSeeingSquare[ny][nx]++;
							if (NumUnitsSeeingSquare[ny][nx] == 1 && pppElements[ny][nx] && pppElements[ny][nx]->owner != unit->owner)
							{
								int player_index = unit->owner->index;
								Unit* rev_unit = pppElements[ny][nx];

								rev_unit->lastSeenPositions[player_index] = rev_unit->curAssociatedSquare;
							}
						}
						else
						{
							NumUnitsSeeingSquare[ny][nx]--;
						}
					}
				}
			}
		}

		// operation is 0 for removing lighted squares, 1 for adding lighted squares.
		void UpdateLightedSquares(Unit* unit, int x, int y, int operation)
		{
			int start_x, start_y, end_x, end_y;
			int** NumLightsOnSquare = pWorld->NumLightsOnSquare;

			if (unit->type->lightRange < 1e-3)
			{
				return;
			}

			if (operation == 1 && unit->lightState == LIGHT_OFF)
			{
				return;
			}
			else if (operation == 0 && !unit->isLighted)
			{
				return;
			}

			if (operation == 1)
			{
				if (unit->isLighted)
				{
					cout << "LIGHTED SQUARES MANAGEMENT WARNING: Attempt to light the squares of a unit twice" << endl;
					return;
				}
				unit->isLighted = true;
			}
			else if (operation == 0)
			{
				unit->isLighted = false;
			}

			start_x = x - unit->type->lightRange < 0 ? 0 : (int) (x - unit->type->lightRange);
			start_y = y - unit->type->lightRange < 0 ? 0 : (int) (y - unit->type->lightRange);
			end_x = x + unit->type->lightRange >= pWorld->width ? pWorld->width-1 : (int) (x + unit->type->lightRange);
			end_y = y + unit->type->lightRange >= pWorld->height ? pWorld->height-1 : (int) (y + unit->type->lightRange);
			for (int ny = start_y; ny < end_y; ny++)
			{
				for (int nx = start_x; nx < end_x; nx++)
				{
					if (Distance2D(nx - x, ny - y) <= unit->type->lightRange)
					{
						if (operation == 1)
						{
							NumLightsOnSquare[ny][nx]++;
						}
						else
						{
							NumLightsOnSquare[ny][nx]--;
						}
					}
				}
			}
		}

		bool SetAssociatedSquares(Unit* unit, int new_x, int new_y)
		{
			if (!SquaresAreWalkable(unit, new_x, new_y))
			{
				return false;
			}
			
			UpdateSeenSquares(unit, new_x, new_y, 1); // add new
			UpdateLightedSquares(unit, new_x, new_y, 1); // add new
			
			unit->curAssociatedSquare.x = new_x;
			unit->curAssociatedSquare.y = new_y;

			GetUnitUpperLeftCorner(unit, new_x, new_y, new_x, new_y);
			for (int y = new_y; y < new_y + unit->type->heightOnMap; y++)
			{
				for (int x = new_x; x < new_x + unit->type->widthOnMap; x++)
				{
					if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
					{
						pppElements[y][x] = unit;
					}
				}
			}

			for (unsigned int i = 0; i < pWorld->vPlayers.size(); i++)
			{
				if (UnitIsVisible(unit, pWorld->vPlayers.at(i)))
				{
					unit->lastSeenPositions[i].x = new_x;
					unit->lastSeenPositions[i].y = new_y;
				}
			}
			
			return true;
		}

		void DeleteAssociatedSquares(Unit* unit, int old_x, int old_y)
		{
			UpdateSeenSquares(unit, old_x, old_y, 0); // remove old
			UpdateLightedSquares(unit, old_x, old_y, 0); // remove old

			GetUnitUpperLeftCorner(unit, old_x, old_y, old_x, old_y);
			for (int y = old_y; y < old_y + unit->type->heightOnMap; y++)
			{
				for (int x = old_x; x < old_x + unit->type->widthOnMap; x++)
				{
					if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
					{
						pppElements[y][x] = NULL;
					}
				}
			}

			return;
		}

		// updates pppElements according to how the unit has moved on the grid
		bool UpdateAssociatedSquares(Unit* unit, int new_x, int new_y, int old_x, int old_y)
		{
			if (!SquaresAreWalkable(unit, new_x, new_y))
			{
				return false;
			}
			
			if (unit->curAssociatedSquare.x != old_x || unit->curAssociatedSquare.y != old_y)
			{
				cout << "ASSOCIATED SQUARES MANAGEMENT WARNING: Attempted to delete squares in another place than where they were once added" << endl;
				old_x = unit->curAssociatedSquare.x;
				old_y = unit->curAssociatedSquare.y;
			}

			DeleteAssociatedSquares(unit, old_x, old_y);
			return SetAssociatedSquares(unit, new_x, new_y);
		}

		void SetLightState(Unit* unit, LightState lightState)
		{
			UpdateLightedSquares(unit, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, 0); // del old
			unit->lightState = lightState;
			UpdateLightedSquares(unit, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, 1); // add new
		}

		void NotEnoughPowerForLight(Unit* unit)
		{
			if (unit->type->lightRange < 1e-3)
			{
				return;
			}

			if (unit->isLighted)
			{
				UpdateLightedSquares(unit, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, 0); // remove
			}
		}

		void EnoughPowerForLight(Unit* unit)
		{
			if (unit->type->lightRange < 1e-3)
			{
				return;
			}

			if (!unit->isLighted)
			{
				UpdateLightedSquares(unit, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y, 1); // add
			}
		}

		int GetTraversalTime(Unit *unit, int x, int y, int dx, int dy)
		{
			return (int) ((10 + abs((pWorld->ppHeight[y][x] - pWorld->ppHeight[y+dy][x+dx])) * 256) * sqrt(pow((float)dx, 2) + pow((float)dy, 2)));
		}

		void ChangePath(Unit* pUnit, float goal_x, float goal_y, AI::UnitAction action, Unit* target, void* arg)
		{
			if (pUnit->type->isMobile)
			{
				AI::CommandPathfinding(pUnit, pUnit->curAssociatedSquare.x, pUnit->curAssociatedSquare.y, goal_x, goal_y, action, target, arg);
			}
		}

		bool CheckPath(Unit* pUnit)
		{
			AI::Node *curnode = pUnit->pMovementData->pStart, *parent;
			bool invalid_path = false;

			// The code below checks whether the path is correct. It cannot, however, check whether pointers
			// are valid in any other way than checking whether they are NULL or do not match other pointers. Non-null
			// invalid pointers will cause _crashes_, and that is not the fault of this piece of code nor this routine.
			if (curnode->pParent)
			{
				invalid_path = true;
				cout << "CRITICAL ERROR IN PATH DETECTED BY CheckPath(): pStart has pParent" << endl;
			}
			else
			{
				parent = curnode;
				curnode = curnode->pChild;
				while (curnode)
				{
					if (curnode->pParent != parent)
					{
						invalid_path = true;
						cout << "CRITICAL ERROR IN PATH DETECTED BY CheckPath(): pNode->pChild->pParent != pNode" << endl;
						break;
					}
					parent = curnode;
					curnode = curnode->pChild;
				}
				if (parent->pChild == NULL && parent != pUnit->pMovementData->pGoal)
				{
					invalid_path = true;
					cout << "CRITICAL ERROR IN PATH DETECTED BY CheckPath(): pGoal IS NOT THE LAST ITEM IN LINKED LIST OF NODES" << endl;
				}
			}
			return invalid_path;
		}

		bool MoveUnit(Unit* pUnit)
		{
			float distance, distance_per_frame;
			AI::UnitAction action = pUnit->action;
			Position goto_pos;
			Utilities::Vector3D move; // abused to calculate movement per axis in 2d and the rotation of the model when going in a specific direction...
			Utilities::Vector3D zero_rot;
			Utilities::Vector3D goal_pos;
			bool should_move = true;

			double power_usage =  pUnit->type->movePowerUsage / AI::aiFps;

			if (pUnit->owner->resources.power < power_usage)
			{
				return true;
			}

			pUnit->owner->resources.power -= power_usage;

			if (pUnit->pMovementData->pStart)
			{
				if ((action == AI::ACTION_FOLLOW || action == AI::ACTION_ATTACK) && pUnit->owner->type != PLAYER_TYPE_REMOTE)
				{
					if (pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].x != (int)pUnit->pMovementData->action.goal.pos.x ||
					    pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].y != (int)pUnit->pMovementData->action.goal.pos.y)
					{
						ChangePath(pUnit, pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].x, pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].y, pUnit->action, pUnit->pMovementData->action.goal.unit, pUnit->pMovementData->action.arg);
					}
				}
			
				if (!pUnit->pMovementData->pCurGoalNode)
				{
					AI::Node *curnode = pUnit->pMovementData->pStart;
					bool recalc_path = CheckPath(pUnit);
					
					if (recalc_path)
					{
						cout << "TRYING TO RECOVER BY DEALLOCATING PATH AND CALCULATING IT AGAIN FOR UNIT " << pUnit << "..." << endl;
					}
					else
					{
						while (curnode)
						{
							if (curnode->x == pUnit->curAssociatedSquare.x &&
							    curnode->y == pUnit->curAssociatedSquare.y)
							{
#ifdef CHECKSUM_DEBUG
								Networking::checksum_output << "START PATH GOAL " << AI::currentFrame << ": " << pUnit->id << " " << curnode->x << " " << curnode->y << endl;
#endif
								pUnit->pMovementData->pCurGoalNode = curnode->pChild;
								break;
							}
							curnode = curnode->pChild;
						}

						if (!pUnit->pMovementData->pCurGoalNode)
						{
							recalc_path = true;
						}
					}

					if (recalc_path)
					{
						if (pUnit->owner->type != PLAYER_TYPE_REMOTE)
						{
#ifdef CHECKSUM_DEBUG
							Networking::checksum_output << "INIT RECALC " << AI::currentFrame << ": " << pUnit->id << endl;
#endif
							// unit positions has changed enough since the path was calculated, making it needed to
							// recalculate it. another possibility is that the path was invalidated by CheckPath()...
							if (action == AI::ACTION_FOLLOW || action == AI::ACTION_ATTACK)
							{
								ChangePath(pUnit, pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].x, 
										  pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].y,
										  pUnit->action, pUnit->pMovementData->action.goal.unit, pUnit->pMovementData->action.arg);
							}
							else
							{
								ChangePath(pUnit, pUnit->pMovementData->action.goal.pos.x, pUnit->pMovementData->action.goal.pos.y,
										  pUnit->action, pUnit->pMovementData->action.goal.unit, pUnit->pMovementData->action.arg);
							}
						}
						AI::DeallocPathfindingNodes(pUnit);
						pUnit->pMovementData->pCurGoalNode = NULL;
						should_move = false;
						pUnit->isMoving = false;
					}
					else
					{
						pUnit->pMovementData->switchedSquare = false;
						if (!pUnit->pMovementData->pCurGoalNode)
						{
#ifdef CHECKSUM_DEBUG
							Networking::checksum_output << "ONE NODE PATH " << AI::currentFrame << ": " << pUnit->id << endl;
#endif
							AI::DeallocPathfindingNodes(pUnit);
							if (pUnit->pMovementData->action.goal.unit)
							{
								if (!CanSee(pUnit, pUnit->pMovementData->action.goal.unit))
								{
#ifdef CHECKSUM_DEBUG
									Networking::checksum_output << "CANCEL CANNOTSEE " << AI::currentFrame << ": " << pUnit->id << endl;
#endif
									AI::CancelAction(pUnit);
								}
							}
							else
							{
								if (pUnit->action != AI::ACTION_BUILD)
								{
#ifdef CHECKSUM_DEBUG
									Networking::checksum_output << "COMPLETE " << AI::currentFrame << ": " << pUnit->id << endl;
#endif
									AI::CompleteAction(pUnit);
								}
								else
								{
									if (!IsWithinRangeForBuilding(pUnit))
									{
#ifdef CHECKSUM_DEBUG
										Networking::checksum_output << "CANCEL CANNOTBUILD " << AI::currentFrame << ": " << pUnit->id << endl;
#endif
										AI::CancelAction(pUnit);
									}
								}
							}
							pUnit->isMoving = false;
							pUnit->pMovementData->pCurGoalNode = NULL;
							should_move = false;
						}
					}
				}
			}
			else
			{
#ifdef CHECKSUM_DEBUG
				Networking::checksum_output << "PATHWAIT " << AI::currentFrame << ": " << pUnit->id << endl;
#endif
				should_move = false;
				pUnit->isMoving = false;
			}

			if (should_move)
			{
				if (pUnit->pMovementData->pCurGoalNode->x == pUnit->pMovementData->pGoal->x &&
				    pUnit->pMovementData->pCurGoalNode->y == pUnit->pMovementData->pGoal->y &&
				    (int) pUnit->pMovementData->action.goal.pos.x == pUnit->pMovementData->pGoal->x &&
				    (int) pUnit->pMovementData->action.goal.pos.y == pUnit->pMovementData->pGoal->y)
				{
					goto_pos = pUnit->pMovementData->action.goal.pos;
				}
				else
				{
					goto_pos.x = pUnit->pMovementData->pCurGoalNode->x + 0.5;
					goto_pos.y = pUnit->pMovementData->pCurGoalNode->y + 0.5;
				}

				distance_per_frame = pUnit->type->movementSpeed / AI::aiFps / 
				                     (GetTraversalTime(pUnit,
						                       pUnit->pMovementData->pCurGoalNode->pParent->x,
								       pUnit->pMovementData->pCurGoalNode->pParent->y,
								       pUnit->pMovementData->pCurGoalNode->x - pUnit->pMovementData->pCurGoalNode->pParent->x,
								       pUnit->pMovementData->pCurGoalNode->y - pUnit->pMovementData->pCurGoalNode->pParent->y)
				                      / 10.0);

				distance = Distance2D(goto_pos.x - pUnit->pos.x, goto_pos.y - pUnit->pos.y);

				move.set(goto_pos.x - pUnit->pos.x, 0.0, goto_pos.y - pUnit->pos.y);
				move.normalize();
				move *= distance_per_frame;
				
#ifdef CHECKSUM_DEBUG
				Networking::checksum_output << "MOVE " << AI::currentFrame << ": " << pUnit->id << " " << pUnit->pos.x << " " << pUnit->pos.y << " " << move.x << " " << move.y << " " << pUnit->type->movementSpeed << " " << AI::aiFps << " " << GetTraversalTime(pUnit, pUnit->pMovementData->pCurGoalNode->pParent->x, pUnit->pMovementData->pCurGoalNode->pParent->y, pUnit->pMovementData->pCurGoalNode->x - pUnit->pMovementData->pCurGoalNode->pParent->x, pUnit->pMovementData->pCurGoalNode->y - pUnit->pMovementData->pCurGoalNode->pParent->y) << " " << distance_per_frame << " " << distance << endl;
#endif
				
				if (distance < distance_per_frame)
				{
					move *= (distance / distance_per_frame);
				}

				if (!pUnit->pMovementData->switchedSquare &&
				    Distance2D(pUnit->pos.x + move.x - pUnit->pMovementData->pCurGoalNode->pParent->x - 0.5,
					       pUnit->pos.y + move.y - pUnit->pMovementData->pCurGoalNode->pParent->y - 0.5) > 
				    Distance2D(pUnit->pos.x + move.x - pUnit->pMovementData->pCurGoalNode->x - 0.5,
					       pUnit->pos.y + move.y - pUnit->pMovementData->pCurGoalNode->y - 0.5))
				{
#ifdef CHECKSUM_DEBUG
					Networking::checksum_output << "ATTEMPT " << AI::currentFrame << ": " << pUnit->id << " " << pUnit->pMovementData->pCurGoalNode->x << " " << pUnit->pMovementData->pCurGoalNode->y << endl;
#endif
					if (!UpdateAssociatedSquares(pUnit, pUnit->pMovementData->pCurGoalNode->x,
								            pUnit->pMovementData->pCurGoalNode->y,
								            pUnit->pMovementData->pCurGoalNode->pParent->x,
								            pUnit->pMovementData->pCurGoalNode->pParent->y))
					{
						should_move = false;
						pUnit->isMoving = false;
						if (!SquaresAreWalkable(pUnit, pUnit->pMovementData->pCurGoalNode->x, pUnit->pMovementData->pCurGoalNode->y, SIW_IGNORE_MOVING))
						{
#ifdef CHECKSUM_DEBUG
							Networking::checksum_output << "RECALC " << AI::currentFrame << ": " << pUnit->id << endl;
#endif
							AI::DeallocPathfindingNodes(pUnit);
							if (pUnit->owner->type != PLAYER_TYPE_REMOTE)
							{
								if (action == AI::ACTION_FOLLOW || action == AI::ACTION_ATTACK)
								{
									ChangePath(pUnit, pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].x, 
											  pUnit->pMovementData->action.goal.unit->lastSeenPositions[pUnit->owner->index].y,
										  pUnit->action, pUnit->pMovementData->action.goal.unit, pUnit->pMovementData->action.arg);
								}
								else
								{
									ChangePath(pUnit, pUnit->pMovementData->action.goal.pos.x, pUnit->pMovementData->action.goal.pos.y,
										  pUnit->action, pUnit->pMovementData->action.goal.unit, pUnit->pMovementData->action.arg);
								}
							}
						}
						else
						{
#ifdef CHECKSUM_DEBUG
							Networking::checksum_output << "STOP " << AI::currentFrame << ": " << pUnit->id << endl;
#endif
						}
					}
					else
					{
						pUnit->pMovementData->switchedSquare = true;
						pUnit->isMoving = true;
					}

				}

				if (should_move)
				{
					pUnit->pos.x += move.x;
					pUnit->pos.y += move.z;
					pUnit->isMoving = true;

					zero_rot.set(-1.0, 0.0, 0.0);
					move.normalize();
					pUnit->rotation = acos(zero_rot.dot(move)) * (180 / PI);
					if (move.z < 0) pUnit->rotation = 180 - pUnit->rotation + 180;

					if (distance < distance_per_frame)
					{
						if (pUnit->pMovementData->pCurGoalNode->x == pUnit->pMovementData->pGoal->x &&
						    pUnit->pMovementData->pCurGoalNode->y == pUnit->pMovementData->pGoal->y)
						{
#ifdef CHECKSUM_DEBUG
							Networking::checksum_output << "REACH " << AI::currentFrame << ": " << pUnit->id << " " << pUnit->pMovementData->pGoal->x << " " << pUnit->pMovementData->pGoal->y << endl;
#endif
							AI::DeallocPathfindingNodes(pUnit);
							if (pUnit->pMovementData->action.goal.unit)
							{
								if (!CanSee(pUnit, pUnit->pMovementData->action.goal.unit))
								{
#ifdef CHECKSUM_DEBUG
									Networking::checksum_output << "CANCEL CANNOTSEE " << AI::currentFrame << ": " << pUnit->id << endl;
#endif
									AI::CancelAction(pUnit);
								}
							}
							else
							{
								if (pUnit->action != AI::ACTION_BUILD)
								{
#ifdef CHECKSUM_DEBUG
									Networking::checksum_output << "COMPLETE " << AI::currentFrame << ": " << pUnit->id << endl;
#endif
									AI::CompleteAction(pUnit);
								}
								else
								{
									if (!IsWithinRangeForBuilding(pUnit))
#ifdef CHECKSUM_DEBUG
										Networking::checksum_output << "CANCEL CANNOTBUILD " << AI::currentFrame << ": " << pUnit->id << endl;
#endif
									{
										AI::CancelAction(pUnit);
									}
								}
							}
							pUnit->isMoving = false;
							pUnit->pMovementData->pCurGoalNode = NULL;
						}
						else
						{
							pUnit->pMovementData->pCurGoalNode = pUnit->pMovementData->pCurGoalNode->pChild;
#ifdef CHECKSUM_DEBUG
							Networking::checksum_output << "NEXT GOAL " << AI::currentFrame << ": " << pUnit->id << " " << pUnit->pMovementData->pCurGoalNode->x << " " << pUnit->pMovementData->pCurGoalNode->y << endl;
#endif
							pUnit->pMovementData->switchedSquare = false;
						}
					}
				}
			}

			return true;
		}

		// Check whether a click at (clickx, clicky) hit unit
		bool DoesHitUnit(Unit* unit, int clickx, int clicky, float& distance)
		{
			UnitType* type = unit->type;
			Model* model = type->model;
			Utilities::Vector3D near_plane, far_plane, tp1, tp2, tp3, hit_pos;
			int index, index_v;

			if (!model)
				return false;

			glPushMatrix();

				SetUnitCoordSpace(unit);

				WindowCoordToVector((GLdouble) clickx, (GLdouble) clicky, near_plane, far_plane);
				
				index = 0;

				for (int i = 0; i < model->tri_count; i++)
				{
					index_v = model->tris[index++] * 3;
					tp1.set(model->vertices[index_v], model->vertices[index_v+1], model->vertices[index_v+2]);
					index_v = model->tris[index++] * 3;
					tp2.set(model->vertices[index_v], model->vertices[index_v+1], model->vertices[index_v+2]);
					index_v = model->tris[index++] * 3;
					tp3.set(model->vertices[index_v], model->vertices[index_v+1], model->vertices[index_v+2]);
					if (CheckLineIntersectTri(tp1, tp3, tp2, near_plane, far_plane, hit_pos))
					{
						glPopMatrix();
						distance = near_plane.distance(hit_pos);
						return true;
					}
				}

			glPopMatrix();

			return false;
		}
		
		// get the screen coord of the middle (0.0, 0.0, 0.0) of unit
		Utilities::Vector3D GetUnitWindowPos(Unit* unit)
		{
//			UnitType* type = unit->type; << unused!
//			Model* model = type->model; << unused!
			Utilities::Vector3D win_vector;
//			int index, index_v; << unused!
//			float unit_y; << unused
			glPushMatrix();

				SetUnitCoordSpace(unit);

				WorldCoordToWindowCoord(Utilities::Vector3D(0.0f, 0.0f, 0.0f), win_vector);

			glPopMatrix();

			return win_vector;

		}

		// get what unit was clicked, if any
		Unit* GetUnitClicked(int clickx, int clicky, int map_x, int map_y)
		{
			Unit* unit;
			Unit* cur_unit = 0;
			float cur_dist = 1e23, dist;
			for (vector<Dimension::Unit*>::iterator it = Dimension::pWorld->vUnits.begin(); it != Dimension::pWorld->vUnits.end(); it++)
			{
				unit = *it;
				if (UnitIsRendered(unit, currentPlayerView))
				{
					if (unit->pos.x > map_x-3 && unit->pos.x < map_x+3 && unit->pos.y < map_y+3 && unit->pos.y < map_y+3)
					{
						if (Dimension::DoesHitUnit(unit, clickx, clicky, dist))
						{
							if (dist < cur_dist)
							{
								cur_unit = unit;
								cur_dist = dist;
							}
						}
					}
				}
			}
			return cur_unit;
		}

		Uint16 r_seed = 23467;
		float rotation_rand()
		{
			r_seed = (r_seed + 82473) ^ 53672;
			return (r_seed / 65535.0);
		}

		// create a unit, but don't display it
		Unit* CreateUnitNoDisplay(UnitType* type, Player* owner)
		{
			Unit* unit = new Unit;
			unit->type = type;
			unit->health = type->maxHealth;
			unit->power = type->maxPower;
			unit->owner = owner;
			unit->completeness = 100.0;
			unit->isCompleted = true;
			unit->rotation = rotation_rand() * 360;
			unit->animData.sAnimData[0] = new SingleAnimData*[10];
			unit->animData.sAnimData[1] = new SingleAnimData*[10];
			for (int i = 0; i < 10; i++)
			{
				unit->animData.sAnimData[0][i] = new SingleAnimData[10];
				unit->animData.sAnimData[1][i] = new SingleAnimData[10];
			}
			unit->pMovementData = new AI::MovementData;
			unit->aiUnitData = new AI::UnitAIData;
			unit->lastAttack = 0;
			unit->action = AI::ACTION_NONE;
			unit->lastAttacked = 0;
			unit->lastCommand = 0;
			unit->isDisplayed = false;
			unit->lastSeenPositions = new IntPosition[pWorld->vPlayers.size()];
			unit->lightState = LIGHT_ON;
			unit->isLighted = false;
			unit->isMoving = false;
			unit->hasSeen = false;
			unit->curAssociatedSquare.x = -1;
			unit->curAssociatedSquare.y = -1;
			unit->rallypoint = NULL;
			unit->id = nextID++;

			if (nextID == 0xFFFF)
				nextID = 0;

			unitByID[unit->id] = unit;
			
			for (int i = 0; i < Audio::SFX_ACT_COUNT; i++)
				unit->soundNodes[i] = NULL;

			for (unsigned int i = 0; i < pWorld->vPlayers.size(); i++)
			{
				unit->lastSeenPositions[i].x = -1000;
				unit->lastSeenPositions[i].y = -1000;
			}

			AI::InitMovementData(unit);

			return unit;
		}
		
		bool DisplayUnit(Unit* unit, float x, float y)
		{
			if (!SquaresAreWalkable_AllKnowing(unit->type, (int) x, (int) y))
			{
				return false;
			}
			unit->pos.x = x;
			unit->pos.y = y;
			SetAssociatedSquares(unit, (int)x, (int)y);
			pWorld->vUnits.push_back(unit);
			unit->owner->vUnits.push_back(unit);
			unit->isDisplayed = true;
			return true;
		}


		// create a unit
		Unit* CreateUnit(UnitType* type, Player* owner, float x, float y)
		{
			if (!SquaresAreWalkable_AllKnowing(type, (int) x, (int) y))
			{
//				cout << "buildfail" << endl;
				return NULL;
			}
			Unit* unit = CreateUnitNoDisplay(type, owner);
			DisplayUnit(unit, x, y);
			return unit;
		}

		void DeleteUnit(Unit* unit)
		{
			Unit* curUnit;
			unsigned int i, j;

			frameRemovedAt[unit->id] = AI::currentFrame;
			unitByID[unit->id] = NULL;

			for (i = 0; i < pWorld->vUnits.size(); i++)
			{
				if (pWorld->vUnits.at(i) == unit)
				{
					pWorld->vUnits.erase(pWorld->vUnits.begin() + i);
					break;
				}
			}
			for (i = 0; i < unit->owner->vUnits.size(); i++)
			{
				if (unit->owner->vUnits.at(i) == unit)
				{
					unit->owner->vUnits.erase(unit->owner->vUnits.begin() + i);
					break;
				}
			}
			for (j = 0; j < 10; j++)
			{
				for (i = 0; i < unitGroups[j].size(); i++)
				{
					if (unitGroups[j].at(i) == unit)
					{
						unitGroups[j].erase(unitGroups[j].begin() + i);
						break;
					}
				}
			}
			for (i = 0; i < unitsSelected.size(); i++)
			{
				if (unitsSelected.at(i) == unit)
				{
					unitsSelected.erase(unitsSelected.begin() + i);
					break;
				}
			}
			for (i = 0; i < pWorld->vUnits.size(); i++)
			{
				curUnit = pWorld->vUnits.at(i);
				if (curUnit->pMovementData->action.goal.unit == unit)
				{
					AI::CancelAction(curUnit);
				}
				if (curUnit->pMovementData->_action.goal.unit == unit)
				{
					SDL_LockMutex(AI::GetMutex());
					
					curUnit->pMovementData->_action.goal.unit = NULL;
					curUnit->pMovementData->_action.action = AI::ACTION_GOTO;
					
					SDL_UnlockMutex(AI::GetMutex());
				}
				for (j = 0; j < curUnit->projectiles.size(); j++)
				{
					if (curUnit->projectiles.at(j)->goalUnit == unit)
					{
						curUnit->projectiles.erase(curUnit->projectiles.begin() + j);
						j--;
					}
				}
			}
			
			DeleteAssociatedSquares(unit, unit->curAssociatedSquare.x, unit->curAssociatedSquare.y);
			
			for (i = 0; i < unit->projectiles.size(); i++)
				delete unit->projectiles.at(i);
			for (i = 0; i < unit->actionQueue.size(); i++)
				delete unit->actionQueue.at(i);
			unit->projectiles.clear();

			if (unit->rallypoint != NULL)
				delete unit->rallypoint;
			
			for (int i = 0; i < 10; i++)
			{
				delete[] unit->animData.sAnimData[0][i];
				delete[] unit->animData.sAnimData[1][i];
			}

			delete[] unit->animData.sAnimData[0];
			delete[] unit->animData.sAnimData[1];
			delete unit->aiUnitData;
			delete[] unit->lastSeenPositions;
			
			for (int i = 0; i < Audio::SFX_ACT_COUNT; i++)
			{
				if (unit->soundNodes[i] != NULL)
				{
					StopRepeatingActionSound(unit, (Audio::SoundNodeAction)i);
				}
			}

			if (AI::IsUndergoingPathCalc(unit))
			{
				AI::QuitUndergoingProc(unit);

				unit = NULL;
				return;
			}
			
			if (unit->pMovementData->pStart != NULL)
				AI::DeallocPathfindingNodes(unit);

			delete unit->pMovementData;

			delete unit;
			
			unit = NULL;
		}

		// create a projectile type
		ProjectileType* CreateProjectileType(char* model, float aoe, Utilities::Vector3D start_pos, float speed, float size)
		{
			ProjectileType* type = new ProjectileType;
			type->model = LoadModel(model);
			type->areaOfEffect = aoe;
			type->startPos = start_pos;
			type->speed = speed;
			type->size = size;
			return type;
		}

		// create a projectile
		Projectile* CreateProjectile(ProjectileType* type, Utilities::Vector3D start, Unit* goal)
		{
			Projectile* proj = new Projectile;
			proj->type = type;
			proj->pos = type->startPos;
			proj->goalUnit = goal;
			return proj;
		}

		Projectile* CreateProjectile(ProjectileType* type, Utilities::Vector3D start, Utilities::Vector3D goal)
		{
			Projectile* proj = new Projectile;
			proj->type = type;
			proj->pos = start;
			proj->goalPos = goal;
			proj->direction = goal - start;
			proj->direction.normalize();
			return proj;
		}

		Model* CreateEmptyModel(Model* template_model)
		{
			Model* model = new Model;
			model->pointCount = template_model->pointCount;
			model->texPointCount = template_model->texPointCount;
			model->vertices = new float[model->pointCount*3];
			model->normals = new float[model->pointCount*3];
			if (template_model->texCoords)
			{
				model->texCoords = new float[model->texPointCount*2];
			}
			else
			{
				model->texCoords = NULL;
			}
			return model;
		}

		MorphAnim* CreateMorphAnim(float length, int numKeyFrames, ...)
		{
			MorphAnim* morphAnim = new MorphAnim;
			va_list ap;
			va_start(ap, numKeyFrames);

			morphAnim->numKeyFrames = numKeyFrames;
			morphAnim->length = length;
			morphAnim->models = new Model*[numKeyFrames];
			morphAnim->keyFramePositions = new float[numKeyFrames];

			for (int i = 0; i < numKeyFrames; i++)
			{
				morphAnim->models[i] = LoadModel(va_arg(ap, char*));
				morphAnim->keyFramePositions[i] = va_arg(ap, double);
			}

			va_end(ap);

			if (!Game::Networking::isDedicatedServer)
				morphAnim->tempModel = CreateEmptyModel(morphAnim->models[0]);

			return morphAnim;
		}

		TransformAnim** CreateAnimChildren(int num, ...)
		{
			TransformAnim** children = new TransformAnim*[num];
			va_list ap;
			va_start(ap, num);

			for (int i = 0; i < num; i++)
			{
				children[i] = va_arg(ap, TransformAnim*);
			}

			va_end(ap);

			return children;
		}

		TransformData* CreateTransformData(Utilities::Vector3D pretrans, Utilities::Vector3D rot, Utilities::Vector3D aftertrans, Utilities::Vector3D scale)
		{
			TransformData* transData = new TransformData;
			transData->pretrans = pretrans;
			transData->rot = rot;
			transData->aftertrans = aftertrans;
			transData->scale = scale;
			return transData;
		}

		TransformAnim* CreateTransAnim(MorphAnim* morphAnim, TransformAnim** children, int numChildren, float length, int numKeyFrames, ...)
		{
			TransformAnim* transAnim = new TransformAnim;
			va_list ap;
			va_start(ap, numKeyFrames);

			transAnim->morphAnim = morphAnim;
			transAnim->numKeyFrames = numKeyFrames;
			transAnim->length = length;
			transAnim->transDatas = new TransformData*[numKeyFrames];
			transAnim->keyFramePositions = new float[numKeyFrames];
			transAnim->children = children;
			transAnim->numChildren = numChildren;
			transAnim->transDatas = new TransformData*[numKeyFrames];

			for (int i = 0; i < numKeyFrames; i++)
			{
				transAnim->transDatas[i] = va_arg(ap, TransformData*);
				transAnim->keyFramePositions[i] = va_arg(ap, double);
			}
			
			va_end(ap);

			return transAnim;
		}

		Animation* CreateAnimation(TransformAnim* transAnim)
		{
			Animation* animation = new Animation;
			animation->transAnim = new TransformAnim*;
			animation->transAnim[0] = transAnim;
			animation->num_parts = 1;
			return animation;
		}

		map<string, UnitType*> unitTypeMap;

		void InitUnits()
		{

			unitByID = new Unit*[65535];
			frameRemovedAt = new Uint32[65535];
			for (int i = 0; i < 65535; i++)
			{
				unitByID[i] = NULL;
				frameRemovedAt[i] = 0;
			}
			nextID = 0;
			
			genericTexture = Utilities::LoadGLTexture((char*) "models/textures/generic.png");

			a_seed = 23467;
			r_seed = 23467;
			
		}

		void HandleAnim(int (&a_frames)[2][4], int& animNum, float mix, Unit* pUnit, void* Anim, AnimType animtype, float (&pos_between_anim_frames)[2])
		{
			float animPos;
			float* keyFramePositions;
			int numKeyFrames;
			float animLength, frameLength;
			int i = 0;
			if (animtype == ANIM_MORPH)
			{
				keyFramePositions = ((MorphAnim*) Anim)->keyFramePositions;
				numKeyFrames = ((MorphAnim*) Anim)->numKeyFrames;
				animLength = ((MorphAnim*) Anim)->length;
			}
			else
			{
				keyFramePositions = ((TransformAnim*) Anim)->keyFramePositions;
				numKeyFrames = ((TransformAnim*) Anim)->numKeyFrames;
				animLength = ((TransformAnim*) Anim)->length;
			}
//			for (int i = 0; i < (1 + pUnit->animData.isTransition ? 1 : 0); i++)
//			{
				while (pUnit->animData.sAnimData[i][animNum]->animPos >= animLength)
					pUnit->animData.sAnimData[i][animNum]->animPos -= animLength;

				animPos = pUnit->animData.sAnimData[i][animNum]->animPos;

				a_frames[i][1] = 0;
				for (int j = 1; j < numKeyFrames; j++)
				{
					if (animPos >= keyFramePositions[j])
					{
						a_frames[i][1]++;
					}
				}

				a_frames[i][0] = a_frames[i][1]-1;
				a_frames[i][2] = a_frames[i][1]+1;
				a_frames[i][3] = a_frames[i][1]+2;

				if (a_frames[i][0] < 0)
					a_frames[i][0] = numKeyFrames-1;

				if (a_frames[i][2] >= numKeyFrames)
					a_frames[i][2] = 0;

				while (a_frames[i][3] >= numKeyFrames)
					a_frames[i][3] -= numKeyFrames;

				if (a_frames[i][1] == numKeyFrames-1)
				{
					frameLength = animLength - keyFramePositions[a_frames[i][1]];
				}
				else
				{
					frameLength = keyFramePositions[a_frames[i][2]] - keyFramePositions[a_frames[i][1]];
				}
				pos_between_anim_frames[i] = (animPos - keyFramePositions[a_frames[i][1]]) / frameLength;
				
				pUnit->animData.sAnimData[i][animNum]->animPos += Rules::time_since_last_frame;
//			}
//			mix = Utilities::InterpolateCatmullRomBounded(0.0, 0.0, 1.0, 1.0, pUnit->animData.transitionPos / pUnit->animData.transitionLength);
		}

		void RenderBuildOutline(UnitType* type, int start_x, int start_y)
		{
			Utilities::Vector3D ter[2][2];
			int end_x, end_y;
			bool outofbounds = false;
			bool suitable = IsSuitableForBuilding(type, currentPlayerView, start_x, start_y);
			GetTypeUpperLeftCorner(type, start_x, start_y, start_x, start_y);
			end_x = start_x + type->widthOnMap-1;
			end_y = start_y + type->heightOnMap-1;

			if (start_x < 0)
				start_x = 0, outofbounds = true;
			
			if (start_y < 0)
				start_y = 0, outofbounds = true;
			
			if (end_x >= pWorld->width-1)
				end_x = pWorld->width-2, outofbounds = true;

			if (end_y >= pWorld->height-1)
				end_y = pWorld->height-2, outofbounds = true;

			glDisable(GL_TEXTURE_2D);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_LIGHTING);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glBegin(GL_QUADS);
				for (int y = start_y; y <= end_y; y++)
				{
					for (int x = start_x; x <= end_x; x++)
					{
						if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
						{
							ter[0][0] = Dimension::GetTerrainCoord(x, y);
							ter[0][1] = Dimension::GetTerrainCoord(x+1, y);
							ter[1][0] = Dimension::GetTerrainCoord(x, y+1);
							ter[1][1] = Dimension::GetTerrainCoord(x+1, y+1);

							if (outofbounds || !SquareIsWalkable(type, currentPlayerView, x, y, SIW_IGNORE_OWN_MOBILE_UNITS))
							{
								glColor4f(1.0f, 0.0f, 0.0f, 1.0);
							}
							else
							{
								if (suitable)
								{
									glColor4f(0.0, 1.0, 0.0, 1.0);
								}
								else
								{
									glColor4f(1.0, 1.0, 0.0, 1.0);
								}
							}
							glNormal3f(0.0f, 1.0f, 0.0f);
							glVertex3f(ter[0][0].x, ter[0][0].y, ter[0][0].z);
							glVertex3f(ter[1][0].x, ter[1][0].y, ter[1][0].z);
							glVertex3f(ter[1][1].x, ter[1][1].y, ter[1][1].z);
							glVertex3f(ter[0][1].x, ter[0][1].y, ter[0][1].z);
						}
					}
				}
			glEnd();

			glEnable(GL_LIGHTING);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_TEXTURE_2D);
		}

		void RenderTriangles(int tri_count, int* tris, float* normals, float* vertices)
		{
			int index_v, index = 0;
			for (int j = 0; j < tri_count; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					index_v = tris[index] * 3;
					glNormal3f(normals[index_v], normals[index_v+1], normals[index_v+2]);
					glVertex3f(vertices[index_v], vertices[index_v+1], vertices[index_v+2]);
					index++;
				}
			}
		}

		void RenderTrianglesTextured(int tri_count, int* tris, int* tex_tris, float* normals, float* vertices, float* texcoords)
		{
			int index_v, index_t, index = 0;
			for (int j = 0; j < tri_count; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					index_v = tris[index] * 3;
					index_t = tex_tris[index] * 2;
					glNormal3f(normals[index_v], normals[index_v+1], normals[index_v+2]);
					glTexCoord2f(texcoords[index_t], texcoords[index_t+1]);
					glVertex3f(vertices[index_v], vertices[index_v+1], vertices[index_v+2]);
					index++;
				}
			}
		}

		void RenderProjectile(Projectile* proj)
		{
			Model* model = proj->type->model;
			glPushMatrix();

			SetProjectileCoordSpace(proj);

			glBegin(GL_TRIANGLES);

				if (model && model->texCoords)
				{
					RenderTrianglesTextured(model->tri_count, model->tris, model->tex_tris, model->normals, model->vertices, model->texCoords);
				}
				else
				{
					RenderTriangles(model->tri_count, model->tris, model->normals, model->vertices);
				}

			glEnd();

			glPopMatrix();
			
		}

		void CalculateMaterial(Unit* unit, GLfloat (&dest)[4], GLfloat source[4], GLfloat (&mod)[2][2][4])
		{
			GLfloat temp[4] = {0.0, 0.0, 0.0, 0.0};
			int num = 0;
			int start_x, start_y;
			int is_seen, is_lighted;
			int** NumUnitsSeeingSquare = currentPlayerView->NumUnitsSeeingSquare;
			GetUnitUpperLeftCorner(unit, start_x, start_y);
			for (int y = start_y; y < start_y + unit->type->heightOnMap; y++)
			{
				for (int x = start_x; x < start_x + unit->type->widthOnMap; x++)
				{
					if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
					{
						is_seen = NumUnitsSeeingSquare[y][x] > 0 ? 1 : 0;
						is_lighted = pWorld->NumLightsOnSquare[y][x] > 0 ? 1 : 0;
						for (int i = 0; i < 4; i++)
							temp[i] += source[i] * mod[is_lighted][is_seen][i];
						num++;
					}
				}
			}
			if (num)
				for (int i = 0; i < 4; i++)
					dest[i] = temp[i] / num;
		}

		void CalculateMaterial(Unit* unit, GLfloat& dest, GLfloat source, GLfloat (&mod)[2][2])
		{
			GLfloat temp = 0.0;
			int num = 0;
			int start_x, start_y;
			bool is_seen, is_lighted;
			int** NumUnitsSeeingSquare = currentPlayerView->NumUnitsSeeingSquare;
			GetUnitUpperLeftCorner(unit, start_x, start_y);
			for (int y = start_y; y < start_y + unit->type->heightOnMap; y++)
			{
				for (int x = start_x; x < start_x + unit->type->widthOnMap; x++)
				{
					if (x >= 0 && y >= 0 && x < pWorld->width && y < pWorld->height)
					{
						is_seen = NumUnitsSeeingSquare[y][x] > 0 ? 1 : 0;
						is_lighted = pWorld->NumLightsOnSquare[y][x] > 0 ? 1 : 0;
						temp += source * mod[is_lighted][is_seen];
						num++;
					}
				}
			}
			if (num)
				dest = temp / num;
		}

		void RenderMorphAnim(Unit* unit, MorphAnim* morphAnim, int& animNum)
		{
			UnitType* type;
			Model* model, *tempModel;
			Model *a_models[4];
			int a_frames[2][4];
			float pos_between_anim_frames[2];
			float fade_out = 0.0f;
			float mix = 0.0f;
			GLfloat mixcol[] = {0.66, 0.66, 0.66, 1.0};
			GLfloat ambient[4] = {0.0, 0.0, 0.0, 1.0};
			GLfloat diffuse[4] = {0.1, 0.1, 0.1, 1.0};
			GLfloat specular[4] = {0.0, 0.0, 0.0, 1.0};
			GLfloat emission[4] = {0.0, 0.0, 0.0, 1.0};
			GLfloat shininess = 10.0;

//			glEnable(GL_NORMALIZE);

			model = morphAnim->models[0];
			type = unit->type;

			glActiveTextureARB(GL_TEXTURE0_ARB);

			if(model->texture == 0)
			{
				glBindTexture(GL_TEXTURE_2D, genericTexture);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, model->texture);
			}
		
			CalculateMaterial(unit, ambient, model->Material_Ambient, unitMaterialAmbient);
			CalculateMaterial(unit, diffuse, model->Material_Diffuse, unitMaterialDiffuse);
			CalculateMaterial(unit, emission, model->Material_Emissive, unitMaterialEmission);
			CalculateMaterial(unit, specular, model->Material_Specular, unitMaterialSpecular);
			CalculateMaterial(unit, shininess, *model->Material_Shininess, unitMaterialShininess);

			if (unit->action == AI::ACTION_DIE)
			{
				fade_out = 1.0f - (AI::currentFrame - unit->lastAttacked) / 60.0;
				diffuse[3] = fade_out;
				glDisable(GL_DEPTH_TEST);
			}
			
			glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, mixcol);
			
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE1_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_CONSTANT_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, GL_SRC_COLOR);

			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, unit->owner->texture);
			
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);

			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &shininess);
			
			if (morphAnim->numKeyFrames != 1)
			{
				
				HandleAnim(a_frames, animNum, mix, unit, morphAnim, ANIM_MORPH, pos_between_anim_frames);

				tempModel = morphAnim->tempModel;

				for (int i = 0; i < 4; i++)
				{
					a_models[i] = morphAnim->models[a_frames[0][i]];
				}

				float normal_scale_factor = 0.0625*unit->type->size * unit->completeness / 100.0;

				for (int i = 0; i < tempModel->pointCount * 3; i++)
				{
					tempModel->vertices[i] = Utilities::InterpolateCatmullRomBounded(a_models[0]->vertices[i],
											      a_models[1]->vertices[i],
											      a_models[2]->vertices[i],
											      a_models[3]->vertices[i],
											      pos_between_anim_frames[0]);

					tempModel->normals[i] = Utilities::InterpolateCatmullRomBounded(a_models[0]->normals[i],
											     a_models[1]->normals[i],
											     a_models[2]->normals[i],
											     a_models[3]->normals[i],
											     pos_between_anim_frames[0]) * normal_scale_factor;
				}

				for (int i = 0; i < tempModel->texPointCount * 2; i++)
				{
					tempModel->texCoords[i] = Utilities::InterpolateCatmullRomBounded(a_models[0]->texCoords[i],
											       a_models[1]->texCoords[i],
											       a_models[2]->texCoords[i],
											       a_models[3]->texCoords[i],
											       pos_between_anim_frames[0]);
				}
				
				model = tempModel;
		
				glBegin(GL_TRIANGLES);

					if (tempModel->texCoords == NULL)
					{
						RenderTriangles(model->tri_count, model->tris, model->normals, model->vertices);
					}
					else
					{
						RenderTrianglesTextured(model->tri_count, model->tris, model->tex_tris, model->normals, model->vertices, model->texCoords);
					}

				glEnd();
			}
			else
			{

				tempModel = morphAnim->tempModel;
				
				float normal_scale_factor = 0.0625*unit->type->size * unit->completeness / 100.0;

				for (int i = 0; i < tempModel->pointCount * 3; i++)
				{
					tempModel->normals[i] = model->normals[i] * normal_scale_factor;
				}

				glBegin(GL_TRIANGLES);

				if (model->texCoords == NULL)
				{
					RenderTriangles(model->tri_count, model->tris, tempModel->normals, model->vertices);
				}
				else
				{
					RenderTrianglesTextured(model->tri_count, model->tris, model->tex_tris, tempModel->normals, model->vertices, model->texCoords);
				}

				glEnd();
			}

			if (unit->action == AI::ACTION_DIE)
			{
				glEnable(GL_DEPTH_TEST);
			}
			
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);

			animNum++;
		}

		void RenderTransAnim(Unit* unit, TransformAnim* transAnim, int& animNum)
		{
			int a_frames[2][4];
			TransformData* a_transdatas[4];
			Utilities::Vector3D pretrans, rot, aftertrans, scale;
			float pos_between_anim_frames[2];
			float mix = 0.0f; // << don't forget to define variables! :)
			glPushMatrix();

				HandleAnim(a_frames, animNum, mix, unit, transAnim, ANIM_TRANSFORM, pos_between_anim_frames);
	
				for (int i = 0; i < 4; i++)
				{
					a_transdatas[i] = transAnim->transDatas[a_frames[0][i]];
				}

				pretrans = Utilities::InterpolateCatmullRomBounded(a_transdatas[0]->pretrans, a_transdatas[1]->pretrans, a_transdatas[2]->pretrans, a_transdatas[3]->pretrans, pos_between_anim_frames[0]);
				rot = Utilities::InterpolateCatmullRomBounded(a_transdatas[0]->rot, a_transdatas[1]->rot, a_transdatas[2]->rot, a_transdatas[3]->rot, pos_between_anim_frames[0]);
				aftertrans = Utilities::InterpolateCatmullRomBounded(a_transdatas[0]->aftertrans, a_transdatas[1]->aftertrans, a_transdatas[2]->aftertrans, a_transdatas[3]->aftertrans, pos_between_anim_frames[0]);
				scale = Utilities::InterpolateCatmullRomBounded(a_transdatas[0]->scale, a_transdatas[1]->scale, a_transdatas[2]->scale, a_transdatas[3]->scale, pos_between_anim_frames[0]);

				glTranslatef(pretrans.x, pretrans.y, pretrans.z);
				glRotatef(rot.x, 1.0, 0.0, 0.0);
				glRotatef(rot.y, 0.0, 1.0, 0.0);
				glRotatef(rot.z, 0.0, 0.0, 1.0);
				glTranslatef(aftertrans.x, aftertrans.y, aftertrans.z);
				glScalef(scale.x, scale.y, scale.z);

				animNum++;

				RenderMorphAnim(unit, transAnim->morphAnim, animNum);
				
				for (int i = 0; i < transAnim->numChildren; i++)
				{
					RenderTransAnim(unit, transAnim->children[i], animNum);
				}

			glPopMatrix();
		}

		void RenderUnits()
		{
			Unit* unit;
			UnitType* type;
			unsigned int uindex;
//			TransformAnim* animations[2];
			for (uindex = 0; uindex < pWorld->vUnits.size(); uindex++)
			{
				unit = pWorld->vUnits.at(uindex);
				type = unit->type;
				int animNum = 0;

				if (UnitIsRendered(unit, currentPlayerView))
				{

					glPushMatrix();

						SetUnitCoordSpace(unit);

	//					unit->animData.anim[0] =
						if (type->animations[unit->action])
						{
							for (int i = 0; i < type->animations[unit->action]->num_parts; i++)
							{
		//						animations[0] = unit->animData.anim[0]->transAnim;
		//						animations[1] = unit->animData.anim[1]->transAnim;
								RenderTransAnim(unit, type->animations[unit->action]->transAnim[i], animNum);
							}
						}
				
					glPopMatrix();
						
				}

				for (unsigned int i = 0; i < unit->projectiles.size(); i++)
				{
					RenderProjectile(unit->projectiles.at(i));
				}
			}

		}

		void RenderHealthBars()
		{
			Unit* unit;
			UnitType* type;
			unsigned int uindex;

			glDisable(GL_TEXTURE_2D);
			glDisable(GL_LIGHTING);

			for (uindex = 0; uindex < unitsSelected.size(); uindex++)
			{
				unit = unitsSelected.at(uindex);
				type = unit->type;

				if (UnitIsRendered(unit, currentPlayerView))
				{
					glPushMatrix();

						SetUnitCoordSpace(unit, true);

						glTranslatef(0.0, 0.625 * type->height, 0.0);
						SetBillBoardCoordSystem(unit);
						
						float scale_const = 1 / pow((double) type->size, 0.75);
						glScalef(scale_const, scale_const, scale_const);
						
						// s = start
						// e = end
						// i = inner
						float x_e = unit->type->widthOnMap / 2.0;
						float x_s = -x_e;
						float y_s = 0.0;
						float y_e = 0.2;
						float progress = 0;

/*
						float y_s_i = y_s + 0.20 * (y_e - y_s);
						float y_e_i = y_s + 1.00 * (y_e - y_s);
						float x_s_i = x_s + 0.05 * (x_e - x_s);
						float x_e_i = x_s + 0.95 * (x_e - x_s);
*/

						glBegin(GL_QUADS);
						
							progress = (float) (unit->health) / type->maxHealth * (x_e - x_s);

							glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
							glVertex3f(x_s, y_s, 0.0);
							glVertex3f(x_s + progress, y_s, 0.0);
							glVertex3f(x_s + progress, y_e, 0.0);
							glVertex3f(x_s, y_e, 0.0);

							glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
							glVertex3f(x_s + progress, y_s, 0.0);
							glVertex3f(x_e, y_s, 0.0);
							glVertex3f(x_e, y_e, 0.0);
							glVertex3f(x_s + progress, y_e, 0.0);
/*
							glVertex3f(x_s_i, y_s_i, 0.0);
							glVertex3f(x_s_i + (float) (unit->health) / type->maxHealth * (x_e_i - x_s_i), y_s_i, 0.0);
							glVertex3f(x_s_i + (float) (unit->health) / type->maxHealth * (x_e_i - x_s_i), y_e_i, 0.0);
							glVertex3f(x_s_i, y_e_i, 0.0);

							glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
							glVertex3f(x_s, y_s, 0.0);
							glVertex3f(x_e, y_s, 0.0);
							glVertex3f(x_e, y_s_i, 0.0);
							glVertex3f(x_s, y_s_i, 0.0);
							
							glVertex3f(x_s, y_e_i, 0.0);
							glVertex3f(x_e, y_e_i, 0.0);
							glVertex3f(x_e, y_e, 0.0);
							glVertex3f(x_s, y_e, 0.0);

							glVertex3f(x_s, y_s_i, 0.0);
							glVertex3f(x_s_i, y_s_i, 0.0);
							glVertex3f(x_s_i, y_e_i, 0.0);
							glVertex3f(x_s, y_e_i, 0.0);

							glVertex3f(x_e_i, y_s_i, 0.0);
							glVertex3f(x_e, y_s_i, 0.0);
							glVertex3f(x_e, y_e_i, 0.0);
							glVertex3f(x_e_i, y_e_i, 0.0);
							
							glVertex3f(x_s_i + (float) (unit->health) / type->maxHealth * (x_e_i - x_s_i), y_s, 0.0);
							glVertex3f(x_e, y_s, 0.0);
							glVertex3f(x_e, y_e, 0.0);
							glVertex3f(x_s_i + (float) (unit->health) / type->maxHealth * (x_e_i - x_s_i), y_e, 0.0);
*/
							if (unit->action == AI::ACTION_BUILD)
							{
								if (unit->pMovementData->action.goal.unit != NULL)
								{
									Unit* target = unit->pMovementData->action.goal.unit;
									if (!target->isCompleted)
									{
										y_s = -0.3;
										y_e = -0.1;
										progress = target->completeness / 100.0 * (x_e - x_s);

										glColor4f(1.0f, 0.75f, 0.0f, 1.0f);
										glVertex3f(x_s, y_s, 0.0);
										glVertex3f(x_s + progress, y_s, 0.0);
										glVertex3f(x_s + progress, y_e, 0.0);
										glVertex3f(x_s, y_e, 0.0);

										glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
										glVertex3f(x_s + progress, y_s, 0.0);
										glVertex3f(x_e, y_s, 0.0);
										glVertex3f(x_e, y_e, 0.0);
										glVertex3f(x_s + progress, y_e, 0.0);
/*
										y_s_i = y_s + 0.20 * (y_e - y_s);
										y_e_i = y_s + 1.00 * (y_e - y_s);

										glColor4f(1.0f, 0.75f, 0.0f, 1.0f);
										glVertex3f(x_s_i, y_s_i, 0.0);
										glVertex3f(x_s_i + target->completeness / 100.0 * (x_e_i - x_s_i), y_s_i, 0.0);
										glVertex3f(x_s_i + target->completeness / 100.0 * (x_e_i - x_s_i), y_e_i, 0.0);
										glVertex3f(x_s_i, y_e_i, 0.0);

										glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
										glVertex3f(x_s, y_s, 0.0);
										glVertex3f(x_e, y_s, 0.0);
										glVertex3f(x_e, y_s_i, 0.0);
										glVertex3f(x_s, y_s_i, 0.0);

										glVertex3f(x_s, y_e_i, 0.0);
										glVertex3f(x_e, y_e_i, 0.0);
										glVertex3f(x_e, y_e, 0.0);
										glVertex3f(x_s, y_e, 0.0);

										glVertex3f(x_s, y_s_i, 0.0);
										glVertex3f(x_s_i, y_s_i, 0.0);
										glVertex3f(x_s_i, y_e_i, 0.0);
										glVertex3f(x_s, y_e_i, 0.0);

										glVertex3f(x_e_i, y_s_i, 0.0);
										glVertex3f(x_e, y_s_i, 0.0);
										glVertex3f(x_e, y_e_i, 0.0);
										glVertex3f(x_e_i, y_e_i, 0.0);

										glVertex3f(x_s_i + target->completeness / 100.0 * (x_e_i - x_s_i), y_s, 0.0);
										glVertex3f(x_e, y_s, 0.0);
										glVertex3f(x_e, y_e, 0.0);
										glVertex3f(x_s_i + target->completeness / 100.0 * (x_e_i - x_s_i), y_e, 0.0);
*/
									}
								}
							}
							
							if (unit->action == AI::ACTION_RESEARCH)
							{
								y_s = -0.3;
								y_e = -0.1;
								progress = unit->action_completeness / 100.0 * (x_e - x_s);

								glColor4f(1.0f, 0.75f, 0.0f, 1.0f);
								glVertex3f(x_s, y_s, 0.0);
								glVertex3f(x_s + progress, y_s, 0.0);
								glVertex3f(x_s + progress, y_e, 0.0);
								glVertex3f(x_s, y_e, 0.0);

								glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
								glVertex3f(x_s + progress, y_s, 0.0);
								glVertex3f(x_e, y_s, 0.0);
								glVertex3f(x_e, y_e, 0.0);
								glVertex3f(x_s + progress, y_e, 0.0);

							}

						glEnd();

					glPopMatrix();
				}

			}
			/*
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_LIGHTING);*/
			Utilities::RevertViewport();
			
		}

	}
}


