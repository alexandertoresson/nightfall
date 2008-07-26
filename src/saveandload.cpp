#include "saveandload.h"
#include "unit.h"
#include "dimension.h"
#include "utilities.h"
#include "minixml.h"
#include "unitinterface.h"
#include "game.h"
#include "environment.h"
#include "unit.h"
#include "aibase.h"
#include "aipathfinding.h"
#include "camera.h"
#include <string>
#include <iostream>

#define CURRENT_SAVEGAME_VERSION 2

using namespace std;

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
						OutputInt(xmlfile, "unit", actionData->goal.unit->GetHandle());
					}
					
				xmlfile.EndTag();

				if (actionData->args.research)
				{
					xmlfile.BeginTag("arg");
						OutputString(xmlfile, "research", actionData->args.research->name);
					xmlfile.EndTag();
				}
				else if (actionData->args.unitType)
				{
					xmlfile.BeginTag("arg");
						OutputString(xmlfile, "unittype", actionData->args.unitType->id);
					xmlfile.EndTag();
				}
				
				OutputFloat(xmlfile, "rotation", actionData->rotation);

			xmlfile.EndTag();
		}

		void OutputPath(Utilities::XMLWriter &xmlfile, std::string tag, AI::Node *pGoal, AI::Node *pStart)
		{
			AI::Node *cur_node = pGoal;
			xmlfile.BeginTag(tag);
				while (cur_node)
				{
					OutputIntPosition(xmlfile, "elem", cur_node->x, cur_node->y);
					cur_node = cur_node->pParent;
				}
			xmlfile.EndTag();
		}

		void OutputMovementData(Utilities::XMLWriter &xmlfile, const gc_ptr<Unit>& unit)
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

		void OutputUnit(Utilities::XMLWriter &xmlfile, const gc_ptr<Unit>& unit)
		{
			xmlfile.BeginTag("unit");

				OutputInt(xmlfile, "id", unit->GetHandle());
				OutputInt(xmlfile, "owner", unit->owner->GetHandle());

				OutputString(xmlfile, "type", unit->type->id);

				OutputFloatPosition(xmlfile, "pos", unit->pos.x, unit->pos.y);
				
				OutputIntPosition(xmlfile, "curAssociatedSquare", unit->curAssociatedSquare);

				if (unit->rallypoint)
				{
					OutputIntPosition(xmlfile, "rallyPoint", *unit->rallypoint);
				}
				
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

		void OutputPlayer(Utilities::XMLWriter &xmlfile, const gc_ptr<Player>& player)
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

		void OutputProjectile(Utilities::XMLWriter &xmlfile, const gc_ptr<Player>& player, const gc_ptr<Projectile>& proj)
		{
			xmlfile.BeginTag("projectile");
				
				OutputInt(xmlfile, "player", player->GetHandle());

				if (proj->attacker && proj->attacker->isDisplayed)
				{
					OutputInt(xmlfile, "owner", proj->attacker->GetHandle());
				}

				if (proj->goalUnit)
				{
					OutputInt(xmlfile, "goalUnit", proj->goalUnit->GetHandle());
				}

				OutputVector3D(xmlfile, "pos", proj->pos);
				OutputVector3D(xmlfile, "direction", proj->direction);
				OutputVector3D(xmlfile, "goalPos", proj->goalPos);

			xmlfile.EndTag();
		}
				
		void OutputCamera(Utilities::XMLWriter &xmlfile)
		{
			Camera* camera = &Game::Dimension::Camera::instance;
			xmlfile.BeginTag("camera");
				OutputVector3D(xmlfile, "focus", camera->GetFocus());
				OutputFloat(xmlfile, "zoom", camera->GetZoom());
				OutputFloat(xmlfile, "rotation", camera->GetRotation());
			xmlfile.EndTag();
		}

		void SaveGame(std::string filename)
		{
			Utilities::XMLWriter xmlfile;
			AI::PausePathfinding();

			xmlfile.Open(filename);

			xmlfile.BeginTag("nightfall_save_file");

				OutputInt(xmlfile, "version", CURRENT_SAVEGAME_VERSION);

				OutputString(xmlfile, "level", Rules::CurrentLevel);

				Environment::FourthDimension* pDimension = Environment::FourthDimension::Instance();

				OutputFloat(xmlfile, "hour", pDimension->GetCurrentHour());

				OutputUint32(xmlfile, "currentFrame", AI::currentFrame);

				OutputUint32(xmlfile, "aiFps", AI::aiFps);

				OutputCamera(xmlfile);

				for (vector<gc_ptr<Player> >::iterator it = pWorld->vPlayers.begin(); it != pWorld->vPlayers.end(); it++)
				{
					const gc_ptr<Player>& player = *it;

					OutputPlayer(xmlfile, player);
					
				}

				for (vector<gc_ptr<Unit> >::iterator it = pWorld->vUnits.begin(); it != pWorld->vUnits.end(); it++)
				{
					const gc_ptr<Unit>& unit = *it;

					OutputUnit(xmlfile, unit);
					
				}
				
				for (vector<gc_ptr<Player> >::iterator it = pWorld->vPlayers.begin(); it != pWorld->vPlayers.end(); it++)
				{
					const gc_ptr<Player>& player = *it;

					for (vector<gc_ptr<Projectile> >::iterator it2 =player->vProjectiles.begin(); it2 != player->vProjectiles.end(); it2++)
					{
						OutputProjectile(xmlfile, player, *it2);
					}

				}
				
			xmlfile.EndTag();

			xmlfile.Close();
			AI::ResumePathfinding();
		}

		Utilities::XMLReader xmlreader;

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
		void ParseUint32(const std::string& text)
		{
			ui = (Uint32) atoi(text.c_str());
		}

		void ParseInt(const std::string& text)
		{
			i = atoi(text.c_str());
		}

		void ParseFloat(const std::string& text)
		{
			f = (float) atof(text.c_str());
		}

		void ParseDouble(const std::string& text)
		{
			d = atof(text.c_str());
		}

		void ParseString(const std::string& text)
		{
			str = text;
		}

		void ParseBool(const std::string& text)
		{
			b = text == "true" || text == "1";
		}

		// Wrappers to parse a block of a basic atomic block, ie a tag containing an int, for example
		void ParseUint32Block(Utilities::XMLElement *elem)
		{
			ui = 0;
			elem->Iterate(ParseUint32);
		}

		void ParseIntBlock(Utilities::XMLElement *elem)
		{
			i = 0;
			elem->Iterate(ParseInt);
		}

		void ParseFloatBlock(Utilities::XMLElement *elem)
		{
			f = 0.0f;
			elem->Iterate(ParseFloat);
		}

		void ParseDoubleBlock(Utilities::XMLElement *elem)
		{
			d = 0.0;
			elem->Iterate(ParseDouble);
		}

		void ParseStringBlock(Utilities::XMLElement *elem)
		{
			str = "";
			elem->Iterate(ParseString);
		}

		void ParseBoolBlock(Utilities::XMLElement *elem)
		{
			b = false;
			elem->Iterate(ParseBool);
		}

		// More advanced, composite types. A layer between the basic types and the game-specific types.
		void ParsePosition(Utilities::XMLElement *elem)
		{
			elem->Iterate("x", ParseFloatBlock);
			pos.x = f;
			elem->Iterate("y", ParseFloatBlock);
			pos.y = f;
		}

		void ParseIntPosition(Utilities::XMLElement *elem)
		{
			elem->Iterate("x", ParseIntBlock);
			pos_int.x = i;
			elem->Iterate("y", ParseIntBlock);
			pos_int.y = i;
		}

		void ParseVector3D(Utilities::XMLElement *elem)
		{
			elem->Iterate("x", ParseFloatBlock);
			vec.x = f;
			elem->Iterate("y", ParseFloatBlock);
			vec.y = f;
			elem->Iterate("z", ParseFloatBlock);
			vec.z = f;
		}

		gc_ptr<Player> player = NULL;

		unsigned sindex, pindex;

		void ParseStances(Utilities::XMLElement *elem)
		{
			if (sindex >= pWorld->vPlayers.size())
			{
				std::cout << "Invalid player index in ParseStances()" << std::endl;
				return;
			}

			elem->Iterate(ParseInt);

			player->states[sindex++] = (PlayerState) i;
		}

		void ParsePlayer(Utilities::XMLElement *elem)
		{
			if (pindex >= pWorld->vPlayers.size())
			{
				std::cout << "Invalid player index in ParsePlayer()" << std::endl;
				return;
			}

			player = pWorld->vPlayers[pindex++];

			elem->Iterate("name", ParseStringBlock);
			player->name = str;

			elem->Iterate("playerType", ParseIntBlock);
			player->type = (PlayerType) i;
			
			elem->Iterate("aiFrame", ParseIntBlock);
			player->aiFrame = i;
			
			elem->Iterate("money", ParseDoubleBlock);
			player->resources.money = d;
			
			elem->Iterate("power", ParseDoubleBlock);
			player->resources.power = d;

			sindex = 0;
			elem->Iterate("stance", ParseStances);
		}

		gc_ptr<Unit> unit;

		void ParseLastSeenPosition(Utilities::XMLElement *elem)
		{
			int player;
			elem->Iterate("player", ParseIntBlock);
			player = i;

			if ((unsigned) player >= pWorld->vPlayers.size())
			{
				std::cout << "Invalid player index in ParseLastSeenPosition()" << std::endl;
				return;
			}

			elem->Iterate("pos", ParseIntPosition);
			unit->lastSeenPositions[player] = pos_int;
		}

		void ParseUnit_Pass1(Utilities::XMLElement *elem)
		{
			bool isDisplayed, isCompleted;
			int id;
			gc_ptr<Player> owner;
			gc_ptr<Dimension::UnitType> type;

			unit = NULL;

			elem->Iterate("id", ParseIntBlock);
			
			if ((unsigned) i >= 65535)
			{
				std::cout << "Invalid unit index (out of range) in ParseUnit_Pass1()" << std::endl;
				return;
			}

			id = i;

			elem->Iterate("owner", ParseIntBlock);

			if ((unsigned) i >= pWorld->vPlayers.size())
			{
				std::cout << "Invalid unit index in ParseUnit_Pass1()" << std::endl;
				return;
			}

			owner = pWorld->vPlayers[i];
			

			elem->Iterate("type", ParseStringBlock);

			type = UnitLuaInterface::GetUnitTypeByID(owner, str);

			if (!type)
			{
				std::cout << "Invalid unit type name in ParseUnit_Pass1()" << std::endl;
				return;
			}

			elem->Iterate("isDisplayed", ParseBoolBlock);
			isDisplayed = b;

			elem->Iterate("isCompleted", ParseBoolBlock);
			isCompleted = b;

			if (isDisplayed)
			{
				elem->Iterate("curAssociatedSquare", ParseIntPosition);
				unit = CreateUnit(type->GetHandle(), owner, pos_int.x, pos_int.y, id, isCompleted);

				if (!unit)
				{
					std::cout << "Failed to create unit in ParseUnit_Pass1()" << std::endl;
					return;
				}

				Dimension::DisplayScheduledUnits();

				elem->Iterate("rotation", ParseDoubleBlock);
				unit->rotation = d;

				elem->Iterate("isMoving", ParseBoolBlock);
				unit->isMoving = b;
				
				elem->Iterate("action_completeness", ParseDoubleBlock);
				unit->action_completeness = d;

				elem->Iterate("lastSeenPosition", ParseLastSeenPosition);

				elem->Iterate("pos", ParsePosition);
				unit->pos = pos;

			}
			else
			{
				unit = CreateUnitNoDisplay(type->GetHandle(), owner, id, isCompleted);
			}
			
			elem->Iterate("health", ParseDoubleBlock);
			unit->health = d;

			elem->Iterate("power", ParseDoubleBlock);
			unit->power = d;

			elem->Iterate("completeness", ParseDoubleBlock);
			unit->completeness = d;
			
		}

		bool is_back;
		Dimension::UnitGoal goal;
		ActionArguments args;

		void ParseUnitGoal(Utilities::XMLElement *elem)
		{
			elem->Iterate("pos", ParseIntPosition);
			goal.pos = pos_int;
			
			elem->Iterate("unit", ParseIntBlock);
			goal.unit = GetUnitByID(i);
		}

		void ParseUnitArgUnitType(Utilities::XMLElement *elem)
		{
			elem->Iterate("unittype", ParseStringBlock);
			args = UnitLuaInterface::GetUnitTypeByID(unit->owner, str);
		}

		void ParseUnitArgResearch(Utilities::XMLElement *elem)
		{
			elem->Iterate("research", ParseStringBlock);
			args = UnitLuaInterface::GetResearchByID(unit->owner, str);
		}

		std::vector<IntPosition> nodes;

		void ParseNode(Utilities::XMLElement *elem)
		{
			elem->Iterate("x", ParseIntBlock);
			pos_int.x = i;
			elem->Iterate("y", ParseIntBlock);
			pos_int.y = i;

			nodes.push_back(pos_int);
		}

		void ParsePath(Utilities::XMLElement *elem)
		{
			AI::Node *pStart = NULL, *pGoal = NULL, *pCur = NULL, *pLast = NULL;

			elem->Iterate("elem", ParseNode);

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

			nodes.clear();
		}

		void ParseCurGoalNode(Utilities::XMLElement *elem)
		{
			AI::Node *pCur = unit->pMovementData->pGoal;
			elem->Iterate("x", ParseIntBlock);
			pos_int.x = i;
			elem->Iterate("y", ParseIntBlock);
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

		bool isDisplayed;

		void ParseActionData(Utilities::XMLElement *elem)
		{
			AI::UnitAction action;
			IntPosition startPos;
			float rotation;

			elem->Iterate("action", ParseIntBlock);
			action = (AI::UnitAction) i;

			elem->Iterate("goal", ParseUnitGoal);
			
			if (action == Game::AI::ACTION_RESEARCH)
			{
				elem->Iterate("arg", ParseUnitArgResearch);
			}
			else
			{
				elem->Iterate("arg", ParseUnitArgUnitType);
			}

			elem->Iterate("startPos", ParseIntPosition);
			startPos = pos_int;

			elem->Iterate("rotation", ParseFloatBlock);
			rotation = f;

			if (action == AI::ACTION_ATTACK || action == AI::ACTION_FOLLOW || action == AI::ACTION_MOVE_ATTACK_UNIT)
			{
				if (!goal.unit)
				{
					std::cout << "ParseActionData() detected an action that needed a unit goal, but had none" << std::endl;
					return;
				}
			}

			if (action == AI::ACTION_BUILD)
			{
				if (!goal.unit && !args.unitType)
				{
					std::cout << "ParseActionData() detected an action that needed a unit goal or an argument, but had neither" << std::endl;
					return;
				}
			}
			
			if (action == AI::ACTION_RESEARCH)
			{
				if (!args.research)
				{
					std::cout << "ParseActionData() detected an action that needed a an argument, but had none" << std::endl;
					return;
				}
			}

			if (action != AI::ACTION_NONE)
			{
				if (isDisplayed)
				{
					if (is_back)
					{
						if (!unit->owner->isRemote)
						{
							AI::CommandPathfinding(unit, startPos.x, startPos.y, goal.pos.x, goal.pos.y, action, goal.unit, args);
						}
					}
					else
					{
						AI::ApplyAction(unit, action, goal.pos.x, goal.pos.y, goal.unit, args, rotation);
					}
				}
				else
				{
					std::cout << "ParseActionData() detected a non-displayed unit with an action" << std::endl;
				}
			}
		}

		void ParseMovementData(Utilities::XMLElement *elem)
		{
			elem->Iterate("path", ParsePath);
			elem->Iterate("curGoalNode", ParseCurGoalNode);

			is_back = false;
			elem->Iterate("actionData", ParseActionData);
			
			is_back = true;
			elem->Iterate("_actionData", ParseActionData);
		}

		void ParseUnit_Pass2(Utilities::XMLElement *elem)
		{
			unit = NULL;

			elem->Iterate("id", ParseIntBlock);

			unit = GetUnitByID(i);

			elem->Iterate("isDisplayed", ParseBoolBlock);
			isDisplayed = b;

			if (!unit)
			{
				std::cout << "Invalid unit index in ParseUnit_Pass2()" << std::endl;
				return;
			}

			elem->Iterate("movementData", ParseMovementData);
			
		}

		void ParseProjectile(Utilities::XMLElement *elem)
		{
			gc_ptr<Projectile> proj;
			Utilities::Vector3D pos, direction, goalPos;

			elem->Iterate("owner", ParseIntBlock);
			const gc_ptr<Unit>& unit = GetUnitByID(i);

			elem->Iterate("player", ParseIntBlock);
			const gc_ptr<Player>& player = HandleManager<Player>::InterpretHandle(i);

			elem->Iterate("goalUnit", ParseIntBlock);
			const gc_ptr<Unit>& goalUnit = GetUnitByID(i);
			
			elem->Iterate("pos", ParseVector3D);
			pos = vec;
			
			elem->Iterate("direction", ParseVector3D);
			direction = vec;
			
			elem->Iterate("goalPos", ParseVector3D);
			goalPos = vec;
			
			proj = CreateProjectile(unit->type->projectileType, pos, goalPos, unit);
			proj->goalUnit = goalUnit;
			proj->direction = direction;
			player->vProjectiles.push_back(proj);

		}

		void ParseCamera(Utilities::XMLElement *elem)
		{
			Utilities::Vector3D focus;
			float zoom, rotation;
			
			elem->Iterate("focus", ParseVector3D);
			focus = vec;
			
			elem->Iterate("zoom", ParseFloatBlock);
			zoom = f;
			
			elem->Iterate("rotation", ParseFloatBlock);
			rotation = f;

			Game::Dimension::Camera::instance.SetCamera(focus, zoom, rotation);
		}

		void ParseMain(Utilities::XMLElement *elem)
		{
			Environment::FourthDimension* pDimension = Environment::FourthDimension::Instance();
			elem->Iterate("hour", ParseFloatBlock);
			pDimension->SetCurrentHour(f);

			elem->Iterate("currentFrame", ParseUint32Block);
			AI::currentFrame = ui;
			
			elem->Iterate("camera", ParseCamera);
			
			elem->Iterate("aiFps", ParseIntBlock);
			AI::aiFps = i;

			pindex = 0;
			elem->Iterate("player", ParsePlayer);

			elem->Iterate("unit", ParseUnit_Pass1);
			elem->Iterate("unit", ParseUnit_Pass2);

			elem->Iterate("projectile", ParseProjectile);
		}

		void ParseLevel(Utilities::XMLElement *elem)
		{
			elem->Iterate("version", ParseIntBlock);
			if (i != CURRENT_SAVEGAME_VERSION)
			{
				std::cout << "Invalid version on save file! This build of Nightfall only loads savegames of version " << CURRENT_SAVEGAME_VERSION << ", while the version of this savegame is " << i << std::endl;
				Rules::CurrentLevel = "ERROR";
				return;
			}
			elem->Iterate("level", ParseStringBlock);
			Rules::CurrentLevel = str;
		}

		void LoadGameSaveFile(std::string filename)
		{
			std::cout << "Parsing save game file " << filename << "..." << std::endl;
			xmlreader.Read(filename);
			xmlreader.root->Iterate("nightfall_save_file", ParseLevel);
		}

		void LoadGame_PostLoad()
		{
			xmlreader.root->Iterate("nightfall_save_file", ParseMain);
#ifndef WIN32
			xmlreader.Deallocate();
#endif	
			AI::SendScheduledUnitEvents();
			UnitLuaInterface::ApplyScheduledActions();
		}
		
	}
}

