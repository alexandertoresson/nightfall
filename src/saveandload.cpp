#include "saveandload.h"
#include "unit.h"
#include "dimension.h"
#include "utilities.h"
#include "minixml.h"
#include <string>

namespace Game
{
	namespace Dimension
	{

		void OutputPosition(Utilities::XMLWriter &xmlfile, std::string tag, float x, float y)
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
			OutputPosition(xmlfile, tag, pos.x, pos.y);
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

		void SaveGame(std::string filename)
		{
			Utilities::XMLWriter xmlfile;
			xmlfile.Open(filename);

			xmlfile.BeginTag("nightfall_save_file");
			for (vector<Unit*>::iterator it = pWorld->vUnits.begin(); it != pWorld->vUnits.end(); it++)
			{
				Unit* unit = *it;
				
				xmlfile.BeginTag("unit");

					OutputInt(xmlfile, "id", unit->id);
					OutputInt(xmlfile, "owner", unit->owner->index);

					OutputString(xmlfile, "type", unit->type->id);

					OutputPosition(xmlfile, "pos", unit->pos.x, unit->pos.y);
					
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

				xmlfile.EndTag();
			}
			xmlfile.EndTag();

			xmlfile.Close();
		}
	}
}

