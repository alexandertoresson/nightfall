#include "saveandload.h"
#include "unit.h"
#include "dimension.h"
#include "utilities.h"
#include "minixml.h"
#include "unitinterface.h"
#include <string>

namespace Game
{
	namespace Dimension
	{

		void OutputVector3D(Utilities::XMLWriter &xmlfile, std::string tag, Utilities::Vector3D vector)
		{
			xmlfile.BeginTag(tag);

				xmlfile.BeginTag("x");
					xmlfile.Write(vector.x);
				xmlfile.EndTag();
				
				xmlfile.BeginTag("y");
					xmlfile.Write(vector.y);
				xmlfile.EndTag();

				xmlfile.BeginTag("z");
					xmlfile.Write(vector.z);
				xmlfile.EndTag();

			xmlfile.EndTag();
		}

		void OutputFloatPosition(Utilities::XMLWriter &xmlfile, std::string tag, float x, float y)
		{
			xmlfile.BeginTag(tag);

				xmlfile.BeginTag("x");
					xmlfile.Write(x);
				xmlfile.EndTag();
				
				xmlfile.BeginTag("y");
					xmlfile.Write(y);
				xmlfile.EndTag();

			xmlfile.EndTag();
		}

		void OutputIntPosition(Utilities::XMLWriter &xmlfile, std::string tag, int x, int y)
		{
			xmlfile.BeginTag(tag);

				xmlfile.BeginTag("x");
					xmlfile.Write(x);
				xmlfile.EndTag();
				
				xmlfile.BeginTag("y");
					xmlfile.Write(y);
				xmlfile.EndTag();

			xmlfile.EndTag();
		}

		void OutputIntPosition(Utilities::XMLWriter &xmlfile, std::string tag, IntPosition pos)
		{
			OutputIntPosition(xmlfile, tag, pos.x, pos.y);
		}

		void OutputInt(Utilities::XMLWriter &xmlfile, std::string tag, int i)
		{
			xmlfile.BeginTag(tag);
				xmlfile.Write(i);
			xmlfile.EndTag();
		}

		void OutputUint32(Utilities::XMLWriter &xmlfile, std::string tag, Uint32 i)
		{
			xmlfile.BeginTag(tag);
				xmlfile.Write(i);
			xmlfile.EndTag();
		}

		void OutputFloat(Utilities::XMLWriter &xmlfile, std::string tag, float f)
		{
			xmlfile.BeginTag(tag);
				xmlfile.Write(f);
			xmlfile.EndTag();
		}

		void OutputDouble(Utilities::XMLWriter &xmlfile, std::string tag, double d)
		{
			xmlfile.BeginTag(tag);
				xmlfile.Write(d);
			xmlfile.EndTag();
		}

		void OutputString(Utilities::XMLWriter &xmlfile, std::string tag, std::string str)
		{
			xmlfile.BeginTag(tag);
				xmlfile.Write(str);
			xmlfile.EndTag();
		}

		void OutputBool(Utilities::XMLWriter &xmlfile, std::string tag, bool b)
		{
			xmlfile.BeginTag(tag);
				xmlfile.Write(b == true ? "true" : "false");
			xmlfile.EndTag();
		}

		void OutputActionData(Utilities::XMLWriter &xmlfile, std::string tag, AI::ActionData *actionData)
		{
			xmlfile.BeginTag(tag);

				OutputInt(xmlfile, "action", actionData->action);
				OutputIntPosition(xmlfile, "startPos", actionData->startPos);

				xmlfile.BeginTag("goal");
					
					OutputIntPosition(xmlfile, "pos", actionData->goal.pos);
					
					if (actionData->goal.unit)
					{
						OutputInt(xmlfile, "unit", actionData->goal.unit->id);
					}
					
				xmlfile.EndTag();

				if (actionData->arg)
				{
					xmlfile.BeginTag("arg");
						if (UnitLuaInterface::IsValidUnitTypePointer((UnitType*) actionData->arg))
						{
							OutputString(xmlfile, "unittype", ((UnitType*) actionData->arg)->id);
						}
					xmlfile.EndTag();
				}

			xmlfile.EndTag();
		}

		void OutputPath(Utilities::XMLWriter &xmlfile, std::string tag, AI::Node *pGoal, AI::Node *pStart)
		{
			AI::Node *cur_node = pGoal;
			xmlfile.BeginTag(tag);
				while (cur_node)
				{
					OutputIntPosition(xmlfile, "node", cur_node->x, cur_node->y);
					cur_node = cur_node->pParent;
				}
			xmlfile.EndTag();
		}

		void OutputMovementData(Utilities::XMLWriter &xmlfile, Unit* unit)
		{
			xmlfile.BeginTag("movementData");
				OutputActionData(xmlfile, "actionData", &unit->pMovementData->action);
				if (unit->pMovementData->pCurGoalNode)
				{
					OutputIntPosition(xmlfile, "curGoalNode", unit->pMovementData->pCurGoalNode->x, unit->pMovementData->pCurGoalNode->y);
				}
				if (unit->pMovementData->pStart)
				{
					OutputPath(xmlfile, "path", unit->pMovementData->pGoal, unit->pMovementData->pStart);
				}
				if (unit->pMovementData->_currentState != AI::INTTHRSTATE_NONE)
				{
					if (unit->pMovementData->_popFromQueue && unit->pMovementData->_reason == AI::POP_NEW_GOAL)
					{
						OutputActionData(xmlfile, "_actionData", &unit->pMovementData->_newAction);
					}
					else
					{
						OutputActionData(xmlfile, "_actionData", &unit->pMovementData->_action);
					}
				}
			xmlfile.EndTag();
		}

		void OutputUnit(Utilities::XMLWriter &xmlfile, Unit* unit)
		{
			xmlfile.BeginTag("unit");

				OutputInt(xmlfile, "id", unit->id);
				OutputInt(xmlfile, "owner", unit->owner->index);

				OutputString(xmlfile, "type", unit->type->id);

				OutputFloatPosition(xmlfile, "pos", unit->pos.x, unit->pos.y);
				
				OutputIntPosition(xmlfile, "curAssociatedSquare", unit->curAssociatedSquare);

				if (unit->rallypoint)
				{
					OutputIntPosition(xmlfile, "rallyPoint", *unit->rallypoint);
				}
				
				OutputInt(xmlfile, "action", unit->action);
				OutputInt(xmlfile, "faceTarget", unit->faceTarget);
				
				OutputFloat(xmlfile, "health", unit->health);
				OutputFloat(xmlfile, "power", unit->power);
				OutputUint32(xmlfile, "lastAttack", unit->lastAttack);
				OutputUint32(xmlfile, "lastAttacked", unit->lastAttacked);
				OutputUint32(xmlfile, "lastCommand", unit->lastCommand);
				
				OutputInt(xmlfile, "aiFrame", unit->aiFrame);
				
				OutputFloat(xmlfile, "completeness", unit->completeness);
				OutputFloat(xmlfile, "actionCompleteness", unit->action_completeness);

				OutputBool(xmlfile, "isCompleted", unit->isCompleted);
				OutputBool(xmlfile, "isDisplayed", unit->isDisplayed);
				OutputBool(xmlfile, "isMoving", unit->isMoving);
				OutputBool(xmlfile, "isLighted", unit->isLighted);
				OutputBool(xmlfile, "hasSeen", unit->hasSeen);
				
				OutputFloat(xmlfile, "rotation", unit->rotation);

				for (unsigned i = 0; i < pWorld->vPlayers.size(); i++)
				{
					if (unit->lastSeenPositions[i].x != -1000)
					{
						xmlfile.BeginTag("lastSeenPosition");

							OutputInt(xmlfile, "player", i);

							OutputIntPosition(xmlfile, "pos", unit->lastSeenPositions[i]);

						xmlfile.EndTag();
					}
				}


				OutputMovementData(xmlfile, unit);

			xmlfile.EndTag();
		}

		void OutputPlayer(Utilities::XMLWriter &xmlfile, Player *player)
		{
			xmlfile.BeginTag("player");
				
				OutputString(xmlfile, "name", player->name);
				OutputInt(xmlfile, "playerType", player->type);

				for (unsigned i = 0; i < pWorld->vPlayers.size(); i++)
				{
					OutputInt(xmlfile, "stance", player->states[i]);
				}

				OutputDouble(xmlfile, "money", player->resources.money);
				OutputDouble(xmlfile, "power", player->resources.power);
				OutputInt(xmlfile, "aiFrame", player->aiFrame);

			xmlfile.EndTag();
		}

		void OutputProjectile(Utilities::XMLWriter &xmlfile, Unit* unit, Projectile *proj)
		{
			xmlfile.BeginTag("projectile");
				
				if (unit)
				{
					OutputInt(xmlfile, "owner", unit->id);
				}

				if (proj->goalUnit)
				{
					OutputInt(xmlfile, "goalUnit", proj->goalUnit->id);
				}

				OutputVector3D(xmlfile, "pos", proj->pos);
				OutputVector3D(xmlfile, "direction", proj->direction);
				OutputVector3D(xmlfile, "goalPos", proj->goalPos);

			xmlfile.EndTag();
		}
				
		void SaveGame(std::string filename)
		{
			Utilities::XMLWriter xmlfile;
			SDL_LockMutex(AI::GetMutex());

			xmlfile.Open(filename);

			xmlfile.BeginTag("nightfall_save_file");

				OutputUint32(xmlfile, "currentFrame", AI::currentFrame);

				OutputUint32(xmlfile, "aiFps", AI::aiFps);

				for (vector<Player*>::iterator it = pWorld->vPlayers.begin(); it != pWorld->vPlayers.end(); it++)
				{
					Player* player = *it;

					OutputPlayer(xmlfile, player);
					
				}

				for (vector<Unit*>::iterator it = pWorld->vUnits.begin(); it != pWorld->vUnits.end(); it++)
				{
					Unit* unit = *it;

					OutputUnit(xmlfile, unit);
					
				}
				
				for (vector<Unit*>::iterator it = pWorld->vUnits.begin(); it != pWorld->vUnits.end(); it++)
				{
					Unit* unit = *it;

					for (vector<Projectile*>::iterator it2 = unit->projectiles.begin(); it2 != unit->projectiles.end(); it2++)
					{
						OutputProjectile(xmlfile, unit, *it2);
					}

				}
				
			xmlfile.EndTag();

			xmlfile.Close();
			SDL_UnlockMutex(AI::GetMutex());
		}

		Utilities::XMLReader xmlfile;

		// Data returned through the Parse* functions
		std::string str;
		int i;
		Uint32 ui;
		float f;
		double d;
		Position pos;
		IntPosition pos_int;
		Utilities::Vector3D vec;
		bool b;

		// Basic atomic types
		void ParseUint32(std::string text)
		{
			ui = (Uint32) atoi(text.c_str());
		}

		void ParseInt(std::string text)
		{
			i = atoi(text.c_str());
		}

		void ParseFloat(std::string text)
		{
			f = (float) atof(text.c_str());
		}

		void ParseDouble(std::string text)
		{
			d = atof(text.c_str());
		}

		void ParseString(std::string text)
		{
			str = text;
		}

		void ParseBool(std::string text)
		{
			b = text == "true" || text == "1";
		}

		// Wrappers to parse a block of a basic atomic block, ie a tag containing an int, for example
		void ParseUint32Block(Utilities::XMLData *data)
		{
			ui = 0;
			xmlfile.Iterate(data, ParseUint32);
		}

		void ParseIntBlock(Utilities::XMLData *data)
		{
			i = 0;
			xmlfile.Iterate(data, ParseInt);
		}

		void ParseFloatBlock(Utilities::XMLData *data)
		{
			f = 0.0f;
			xmlfile.Iterate(data, ParseFloat);
		}

		void ParseDoubleBlock(Utilities::XMLData *data)
		{
			d = 0.0;
			xmlfile.Iterate(data, ParseDouble);
		}

		void ParseStringBlock(Utilities::XMLData *data)
		{
			str = "";
			xmlfile.Iterate(data, ParseString);
		}

		void ParseBoolBlock(Utilities::XMLData *data)
		{
			b = false;
			xmlfile.Iterate(data, ParseBool);
		}

		// More advanced, composite types. A layer between the basic types and the game-specific types.
		void ParsePosition(Utilities::XMLData *data)
		{
			xmlfile.Iterate(data, "x", ParseFloatBlock);
			pos.x = f;
			xmlfile.Iterate(data, "y", ParseFloatBlock);
			pos.y = f;
		}

		void ParseIntPosition(Utilities::XMLData *data)
		{
			xmlfile.Iterate(data, "x", ParseIntBlock);
			pos_int.x = i;
			xmlfile.Iterate(data, "y", ParseIntBlock);
			pos_int.y = i;
		}

		void ParseVector3D(Utilities::XMLData *data)
		{
			xmlfile.Iterate(data, "x", ParseFloatBlock);
			vec.x = f;
			xmlfile.Iterate(data, "y", ParseFloatBlock);
			vec.y = f;
			xmlfile.Iterate(data, "z", ParseFloatBlock);
			vec.z = f;
		}

		Player *player = NULL;

		void ParseStances(Utilities::XMLData *data)
		{
			if (data->index > pWorld->vPlayers.size())
			{
				return;
			}

			xmlfile.Iterate(data, ParseInt);

			player->states[data->index] = (PlayerState) i;
		}

		void ParsePlayer(Utilities::XMLData *data)
		{
			if (data->index > pWorld->vPlayers.size())
			{
				return;
			}

			player = pWorld->vPlayers[data->index];

			xmlfile.Iterate(data, "name", ParseStringBlock);
			player->name = str;

			xmlfile.Iterate(data, "playerType", ParseIntBlock);
			player->type = (PlayerType) i;
			
			xmlfile.Iterate(data, "aiFrame", ParseIntBlock);
			player->aiFrame = i;
			
			xmlfile.Iterate(data, "money", ParseDoubleBlock);
			player->resources.money = d;
			
			xmlfile.Iterate(data, "power", ParseDoubleBlock);
			player->resources.power = d;

			xmlfile.Iterate(data, "stance", ParseStances);
		}

		Unit* unit;

		void ParseLastSeenPosition(Utilities::XMLData *data)
		{
			int player;
			xmlfile.Iterate(data, "player", ParseIntBlock);
			player = i;

			if ((unsigned) player >= pWorld->vPlayers.size())
			{
				return;
			}

			xmlfile.Iterate(data, "pos", ParseIntPosition);
			unit->lastSeenPositions[player] = pos_int;
		}

		void ParseUnit_Pass1(Utilities::XMLData *data)
		{
			bool isDisplayed;
			int id;
			Player *owner;
			UnitType *type;

			unit = NULL;

			xmlfile.Iterate(data, "id", ParseIntBlock);
			
			if ((unsigned) i >= 65535)
			{
				return;
			}

			id = i;

			xmlfile.Iterate(data, "owner", ParseIntBlock);

			if ((unsigned) i >= pWorld->vPlayers.size())
			{
				return;
			}

			owner = pWorld->vPlayers[i];
			

			xmlfile.Iterate(data, "type", ParseStringBlock);

			type = UnitLuaInterface::GetUnitTypeByID(str);

			if (!type)
			{
				return;
			}

			xmlfile.Iterate(data, "isDisplayed", ParseBoolBlock);
			isDisplayed = b;

			if (isDisplayed)
			{
				xmlfile.Iterate(data, "curAssociatedSquare", ParseIntPosition);
				unit = CreateUnit(type, owner, pos_int.x, pos_int.y, id);

				if (!unit)
				{
					return;
				}

				xmlfile.Iterate(data, "pos", ParsePosition);
				unit->pos = pos;

				xmlfile.Iterate(data, "rotation", ParseDoubleBlock);
				unit->rotation = d;

				xmlfile.Iterate(data, "isCompleted", ParseBoolBlock);
				unit->isCompleted = b;

				xmlfile.Iterate(data, "isMoving", ParseBoolBlock);
				unit->isMoving = b;
				
				xmlfile.Iterate(data, "action_completeness", ParseDoubleBlock);
				unit->action_completeness = b;

				xmlfile.Iterate(data, "lastSeenPosition", ParseLastSeenPosition);

			}
			else
			{
				unit = CreateUnitNoDisplay(type, owner, id);
			}
			
			xmlfile.Iterate(data, "health", ParseDoubleBlock);
			unit->health = d;

			xmlfile.Iterate(data, "power", ParseDoubleBlock);
			unit->power = d;

			xmlfile.Iterate(data, "completeness", ParseDoubleBlock);
			unit->completeness = d;

		}

		bool is_back;
		AI::UnitGoal goal;
		void *arg;

		void ParseUnitGoal(Utilities::XMLData *data)
		{
			xmlfile.Iterate(data, "pos", ParseIntPosition);
			goal.pos = pos_int;
			
			xmlfile.Iterate(data, "unit", ParseIntBlock);
			goal.unit = GetUnitByID(i);
		}

		void ParseUnitArg(Utilities::XMLData *data)
		{
			xmlfile.Iterate(data, "unittype", ParseStringBlock);
			arg = UnitLuaInterface::GetUnitTypeByID(str);
		}

		std::vector<IntPosition> nodes;

		void ParseNode(Utilities::XMLData *data)
		{
			xmlfile.Iterate(data, "x", ParseIntBlock);
			pos_int.x = i;
			xmlfile.Iterate(data, "y", ParseIntBlock);
			pos_int.y = i;

			nodes.push_back(pos_int);
		}

		void ParsePath(Utilities::XMLData *data)
		{
			AI::Node *pStart = NULL, *pGoal = NULL, *pCur, *pLast = NULL;

			xmlfile.Iterate(data, "node", ParseNode);

			AI::Node *pNodes = new AI::Node[nodes.size()];

			int i = 0;
			for (std::vector<IntPosition>::iterator it = nodes.begin(); it != nodes.end(); it++, i++)
			{
				pCur = &pNodes[i];

				if (!pGoal)
				{
					pGoal = pCur;
				}

				pCur->pChild = pLast;

				pCur->x = it->x;
				pCur->y = it->y;

				if (pLast)
				{
					pLast->pParent = pCur;
				}

				pLast = pCur;
			}
			pStart = pCur;

			unit->pMovementData->pGoal = pGoal;
			unit->pMovementData->pStart = pStart;
		}

		void ParseCurGoalNode(Utilities::XMLData *data)
		{
			AI::Node *pCur = unit->pMovementData->pGoal;
			xmlfile.Iterate(data, "x", ParseIntBlock);
			pos_int.x = i;
			xmlfile.Iterate(data, "y", ParseIntBlock);
			pos_int.y = i;

			while (pCur)
			{
				if (pos_int.x == pCur->x && pos_int.y == pCur->y)
				{
					unit->pMovementData->pCurGoalNode = pCur;
					break;
				}
				pCur = pCur->pParent;
			}
		}

		void ParseActionData(Utilities::XMLData *data)
		{
			AI::UnitAction action;
			IntPosition startPos;

			xmlfile.Iterate(data, "action", ParseIntBlock);
			action = (AI::UnitAction) i;

			xmlfile.Iterate(data, "goal", ParseUnitGoal);
			
			xmlfile.Iterate(data, "arg", ParseUnitArg);

			xmlfile.Iterate(data, "startPos", ParseIntPosition);
			startPos = pos_int;

			if (is_back)
			{
				if (unit->owner->type != PLAYER_TYPE_REMOTE)
				{
					AI::CommandPathfinding(unit, startPos.x, startPos.y, goal.pos.x, goal.pos.y, action, goal.unit, arg);
				}
			}
			else
			{
				AI::ApplyAction(unit, action, goal.pos.x, goal.pos.y, goal.unit, arg);
			}
		}

		void ParseMovementData(Utilities::XMLData *data)
		{
			xmlfile.Iterate(data, "path", ParsePath);
			xmlfile.Iterate(data, "curGoalNode", ParseCurGoalNode);

			is_back = false;
			xmlfile.Iterate(data, "actionData", ParseActionData);
			
			is_back = true;
			xmlfile.Iterate(data, "_actionData", ParseActionData);
		}

		void ParseUnit_Pass2(Utilities::XMLData *data)
		{
			unit = NULL;

			xmlfile.Iterate(data, "id", ParseIntBlock);

			unit = GetUnitByID(i);

			if (!unit)
			{
				return;
			}

			xmlfile.Iterate(data, "movementData", ParseMovementData);
			
			xmlfile.Iterate(data, "action", ParseIntBlock);
			unit->action = (AI::UnitAction) i;
			
		}

		void ParseProjectile(Utilities::XMLData *data)
		{
			Unit *owner, *goalUnit;
			Projectile *proj;
			Utilities::Vector3D pos, direction, goalPos;
			xmlfile.Iterate(data, "owner", ParseIntBlock);
			owner = GetUnitByID(i);

			if (!owner)
			{
				return;
			}

			xmlfile.Iterate(data, "goalUnit", ParseIntBlock);
			goalUnit = GetUnitByID(i);
			
			xmlfile.Iterate(data, "pos", ParseVector3D);
			pos = vec;
			
			xmlfile.Iterate(data, "direction", ParseVector3D);
			direction = vec;
			
			xmlfile.Iterate(data, "goalPos", ParseVector3D);
			goalPos = vec;
			
			proj = CreateProjectile(owner->type->projectileType, pos, goalPos);
			proj->goalUnit = goalUnit;
			proj->direction = direction;
			owner->projectiles.push_back(proj);

		}

		void ParseMain(Utilities::XMLData *data)
		{
			xmlfile.Iterate(data, "currentFrame", ParseUint32Block);
			AI::currentFrame = ui;
			
			xmlfile.Iterate(data, "aiFps", ParseIntBlock);
			AI::aiFps = i;

			xmlfile.Iterate(data, "player", ParsePlayer);

			xmlfile.Iterate(data, "unit", ParseUnit_Pass1);
			xmlfile.Iterate(data, "unit", ParseUnit_Pass2);

			xmlfile.Iterate(data, "projectile", ParseProjectile);
		}

		void LoadGame(std::string filename)
		{
			xmlfile.Read(filename);

			xmlfile.Iterate("nightfall_save_file", ParseMain);
		}
		
	}
}

