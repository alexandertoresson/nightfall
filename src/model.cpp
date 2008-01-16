#include "model.h"

#include "game.h"
#include "utilities.h"
#include "errors.h"
#include "dimension.h"
#include "paths.h"
#include <iostream>
#include <fstream>
#include <stack>

#define NEWLINE "\n"
#define PRINTTOKENS 0
#define PRINTOUT
//#define INVERTFACES
//#define INVERTTEXFACES
//#define NORMALDEBUG
//#define DATADEBUG

#define SET_UNITTYPE_ENUM(case_, type, newtype) \
	case case_: \
	{ \
		pUnitType->type = newtype; \
		break; \
	}

#ifdef DATADEBUG
#define FETCH_UNIT_VALUE(case_, dest, source) \
	case case_: \
	{ \
		console << #case_ << Console::nl; \
		Unit->dest = currentUnit->param.source; \
		DeleteOnlyAndContinue(currentUnit); \
		break; \
	}

#else
#define FETCH_UNIT_VALUE(case_, dest, source) \
	case case_: \
	{ \
		Unit->dest = currentUnit->param.source; \
		DeleteOnlyAndContinue(currentUnit); \
		break; \
	}
#endif

#ifdef DATADEBUG
#define SET_UNITTYPE_VALUE(name) \
	pUnitType->name = pUnit->name; \
	console << #name << " == " << pUnitType->name << Console::nl;
#else
#define SET_UNITTYPE_VALUE(name) \
	pUnitType->name = pUnit->name;
#endif
using namespace std;
using namespace Game;
using namespace Game::Dimension;
namespace  Utilities
{
	ModelParser::ModelParser(string filepath)
	{
		mFile = filepath;
		InitEnum();
	}
	
	ModelParser::ModelParser()
	{
		mFile = "";
		InitEnum();
	}

	bool ModelParser::ValidateHeader(string line)
	{
		string header = "#VRML V2.0 utf8";
		string header2 = "#RTS V1.0 utf8";

		if(line.compare(0, header.length(), header) == 0)
			return true;
		else if(line.compare(0, header2.length(), header2) == 0)
			return true;
		else
			return false;
	}
	
	ModelParser::~ModelParser()
	{
		//Clean the vector with all data, deallocate all.
		for(vector<Game::Dimension::Model*>::iterator iterator = ModelsIndex.begin(); iterator != ModelsIndex.end(); iterator++)
		{
			Game::Dimension::Model* tmpModel = *iterator;
			if(tmpModel->normals != NULL)
				delete [] tmpModel->normals;
			if(tmpModel->vertices != NULL)
				delete [] tmpModel->vertices;
			if(tmpModel->tris != NULL)
				delete [] tmpModel->tris;
			if(tmpModel->texCoords != NULL)
				delete [] tmpModel->texCoords;
			delete tmpModel;
		}

		for(vector<Game::Dimension::UnitType*>::iterator iterator = UnitsIndex.begin(); iterator != UnitsIndex.end(); iterator++)
		{
			Game::Dimension::UnitType* tmpUnit = *iterator;
			for (int i = 0; i < Audio::SFX_ACT_COUNT; i++)
			{
				if (tmpUnit->actionSounds[i] != NULL)
				{
					delete tmpUnit->actionSounds[i];
					tmpUnit->actionSounds[i] = NULL;
				}
			}
			delete tmpUnit;
		}
	}

	inline void ModelParser::CreateVRMLData(VRMLData*& current)
	{
		current->nxt = new VRMLData;
		current = current->nxt;
		current->nxt = NULL;
	}
	

	inline void ModelParser::DeleteDataGroup(DataGroup*& target)
	{
		if(target->type == Model)
		{
			if(target->param.VRML_Begin == NULL)
				delete target;
			else
			{
				DeleteAll(target->param.VRML_Begin);
			}

		}
		else if(target->type == Unit)
		{
			DeleteAll(target->param.Unit_Begin);
		}
		delete target;
		target = NULL;		
	}

	inline void ModelParser::DeleteAll(VRMLData*& target)
	{
		while(target != NULL)
		{
			DeleteAndContinue(target);
		}		
	}

	void ModelParser::DeleteVRMLData(VRMLData*& current)
	{	
		switch(current->type)
		{
			case Definition:
			{
				delete current->param.Definition;
				break;
			}
			case Coords: //Dealloc Coordinates
			{
				delete current->param.Coordinates;
				break;
			}
			case TextureCoords: //Dealloc Coordinates
			{
				delete current->param.Coordinates;
				break;
			}
			case TextureCoordsIndex: //Dealloc Faces
			{
				delete current->param.Faces;
				break;
			}
			case CoordsIndex: //Dealloc Faces
			{
				delete current->param.Faces;
				break;
			}
			case URL: //Dealloc URL
			{
				delete current->param.URL;
				break;
			}
			default:
				break;
		}
		
		delete current;
		current = NULL;		
	}
	
	void ModelParser::CreateInnerModel(InnerModel*& Model)
	{
		Model = new InnerModel;
		Model->Parameters = NULL;
		Model->CoordinateFaces = NULL;
		Model->Coordinates = NULL;
		Model->TextureCoordinates = NULL;
		Model->TextureFaces = NULL;
		Model->Material = NULL;
		Model->Name = NULL;
	}
	
	void ModelParser::DeleteInnerModel(InnerModel*& Model)
	{
		if(Model == NULL)
			return;
		if(Model->Parameters != NULL)
			delete Model->Parameters;
		if(Model->Coordinates != NULL)
			delete Model->Coordinates;
		if(Model->TextureCoordinates != NULL)
			delete Model->TextureCoordinates;
		if(Model->CoordinateFaces != NULL)
			delete Model->CoordinateFaces;
		if(Model->TextureFaces != NULL)
			delete Model->TextureFaces;
		if(Model->Material != NULL)
			delete Model->Material;
		if(Model->Name != NULL)
			delete Model->Name;
		if(Model->TRS_Scale != NULL)
			delete Model->TRS_Scale;
		if(Model->TRS_Translate != NULL)
			delete Model->TRS_Translate;
		delete Model;
		Model = NULL;
	}

	inline void ModelParser::DeleteOnlyAndContinue(VRMLData*& current)
	{
		VRMLData* tmp = current;
		current = current->nxt;
		delete tmp;
	}
	
	inline void ModelParser::DeleteAndContinue(VRMLData*& current)
	{
		VRMLData* tmp = current;
		current = current->nxt;
		DeleteVRMLData(tmp);		
	}


	void ModelParser::CreateInnerUnit(InnerUnit*& Unit)
	{
		Unit = new InnerUnit;
		Unit->maxHealth = 0;
		Unit->maxPower = 0;
		Unit->minAttack = 0;
		Unit->maxAttack = 0;
		Unit->heightOnMap = 0;
		Unit->widthOnMap = 0;
		Unit->isMobile = false;
		Unit->hasAI = false;
		Unit->isResearched = false;
		Unit->hurtByLight = false;
		Unit->canAttack = false;
		Unit->canAttackWhileMoving = false;
		Unit->attackMinRange = 0.0f;
		Unit->attackMaxRange = 0.0f;
		Unit->attackAccuracy = 0.0f;
		Unit->sightRange = 0.0f;
		Unit->movementSpeed = 0.0f;
		Unit->movementType = MovementTypeVehicle;
		Unit->projectileType = NULL;
		Unit->attackSpeed = 0.0f;
		Unit->size = 0.0f;
		Unit->height = 0.0f;
		Unit->buildTime = 0;
		Unit->researchTime = 0;
		Unit->Model = NULL;
		Unit->canBuild = NULL;
		Unit->canResearch = NULL;
		Unit->Model = NULL;
		Unit->Symbol = NULL;
		Unit->powerIncrement = 0;
		Unit->powerUsage = 0;
		Unit->lightPowerUsage = 0;
		Unit->attackPowerUsage = 0;
		Unit->buildPowerUsage = 0;
		Unit->movePowerUsage = 0;
		Unit->lightRange = 0.0f;
		Unit->buildCost = 0;
		Unit->researchCost = 0;
		Unit->powerType = PowerTwentyFourSeven;
		Unit->sound = NULL;
	}

	void ModelParser::DeleteInnerUnit(InnerUnit*& Unit)
	{
		if(Unit != NULL)
			delete Unit;
		Unit = NULL;
	}
	// Härifrån?
	inline void ModelParser::CreateUnitData(UnitData*& current)
	{
		current->nxt = new UnitData;
		current = current->nxt;
		current->nxt = NULL;
	}


	inline void ModelParser::DeleteAll(UnitData*& target)
	{
		while(target != NULL)
		{
			DeleteAndContinue(target);
		}		
	}

	void ModelParser::DeleteUnitData(UnitData*& current)
	{	
		switch(current->type)
		{
			case Definition:
			{
				delete current->param.str;
				break;
			}
			default:
				break;
		}
		
		delete current;
		current = NULL;
	}

	inline void ModelParser::DeleteOnlyAndContinue(UnitData*& current)
	{
		UnitData* tmp = current;
		current = current->nxt;
		delete tmp;
	}
	
	inline void ModelParser::DeleteAndContinue(UnitData*& current)
	{
		UnitData* tmp = current;
		current = current->nxt;
		DeleteUnitData(tmp);		
	}

	float CalculateNormal(float *array)
	{
		//9 byte array
		return 0;
	}

	bool ModelParser::ReadParameter(ModelParser::DataType dTyp, ModelParser::DataParameter& store, vector<string>::iterator& Position, vector<string>& tokens)
	{
		switch(dTyp)
		{
			case Float:
			case Integer:
			case String:
			case Boolean:
			{
				if(GetNextType(Position, tokens, dTyp) != true)
					return false;
	
				switch(dTyp)
				{
					case Float: //Convert to float.
					{
						store.floating = (float) atof((*Position).c_str());
						break;
					}
					case Integer: //Convert to integer
					{
						store.integer = atoi((*Position).c_str());
						break;
					}
					case String: //Save as string...
					{
						store.str = new string();
						*store.str = (*Position).substr(1,(*Position).size() - 2);
						break;
					}
					case Boolean: //Convert int 0 / 1 to false / true
					{
						int val = atoi((*Position).c_str());
						if(val == 0)
							store.boolean = false;
						else if(val == 1)
							store.boolean = true;
						else
							return false;
						break;
					}
					default:
						return false;
				}
				return true;
				break;
			}
			case Coordinate3D:
			{
				//The next 3 floats.
				float *values = new float[3];
				int counter = 0;
				while(Position != tokens.end() && counter < 3)
				{
					if(!SearchAhead(Position, tokens.end(), Number, '\0'))
					{
						return false;
					}
					values[counter] = (float) atof((*Position).c_str());
					counter++;
				}
				if(Position == tokens.end())
				{
					return false;
				}
				store.floatArray = values;
				return true;
				break;
			}
			default:
			{
				return false;
			}
		}
	}

	bool ModelParser::ReadUnitParameter(ModelParser::TokenType tTyp, ModelParser::DataType dTyp, ModelParser::UnitData*& current, vector<string>::iterator& Position, vector<string>& tokens)
	{
		if(GetNextType(Position, tokens, dTyp) != true)
			return false;

		switch(dTyp)
		{
			case Float: //Convert to float.
			{
				CreateUnitData(current);
				current->type = tTyp;
				current->param.floating = (float) atof((*Position).c_str());
				break;
			}
			case Integer: //Convert to integer
			{
				CreateUnitData(current);
				current->type = tTyp;
				current->param.integer = atoi((*Position).c_str());
				break;
			}
			case String: //Save as string...
			{
				CreateUnitData(current);
				current->type = tTyp;
				current->param.str = new string();
				*current->param.str = (*Position).substr(1,(*Position).size() - 2);
				break;
			}
			case Boolean: //Convert int 0 / 1 to false / true
			{
				CreateUnitData(current);
				current->type = tTyp;
				int val = atoi((*Position).c_str());
				if(val == 0)
					current->param.boolean = false;
				else if(val == 1)
					current->param.boolean = true;
				else
					return false;
				break;
			}
			default:
				return false;
		}
		return true;
	}
	//Find [, find each typ until ] is found.
	bool ModelParser::ReadUnitArray(ModelParser::TokenType tTyp, ModelParser::DataType dTyp, ModelParser::UnitData*& current, vector<string>::iterator& Position, vector<string>& tokens)
	{
		//find [
		if(SearchAhead(Position, tokens.end(), SpecifiedChar, '[') != true)
			return false;

		switch(dTyp)
		{
			case Float: //Convert to float.
			{
				CreateUnitData(current);
				current->type = tTyp;
				current->param.floatArray = new vector<float>();
				
				while(Position != tokens.end())
				{
					if(GetDataType(*Position) == Float)
						current->param.floatArray->push_back((float) atof((*Position).c_str()));

					if((*Position).at(0) == ']')
						break;

					Position++;
				}
				if(Position == tokens.end())
					return false;

				return true;
				break;
			}
			case String: //Just save the string.
			{
				CreateUnitData(current);
				current->type = tTyp;
				current->param.stringArray = new vector<string>();
				
				while(Position != tokens.end())
				{
					if(GetDataType(*Position) == String)
						current->param.stringArray->push_back((*Position).substr(1,(*Position).size() - 2));

					if((*Position).at(0) == ']')
						break;

					Position++;
				}
				if(Position == tokens.end())
					return false;

				return true;
				break;
			}
			default:
				return false;
		}
		return true;
	}
	void ModelParser::InitEnum()
	{
		//VRML Material
		TokenEnums["emissiveColor"] = Material_Emissive;
		TokenEnums["specularColor"] = Material_Specular;
		TokenEnums["transparency"] = Material_Transperency;
		TokenEnums["diffuseColor"] = Material_Diffuse;
		TokenEnums["shininess"] = Material_Shininess;
		TokenEnums["ambientIntensity"] = Material_Ambient;
		TokenEnums["material"] = Material;

		//VRML token
		TokenEnums["DEF"] = Definition;
		TokenEnums["Appearance"] = Appearance;
		TokenEnums["Shape"] = Shape;
		TokenEnums["url"] = URL;
		TokenEnums["scale"] = Scale;
		TokenEnums["Coordinate"] = Coords;
		TokenEnums["Transform"] = Transform;
		TokenEnums["TextureCoordinate"] = TextureCoords;
		TokenEnums["translation"] = Translate;
		TokenEnums["texCoordIndex"] = TextureCoordsIndex;
		TokenEnums["texture"] = Texture;
		TokenEnums["IndexedFaceSet"] = Geometry;
		TokenEnums["ImageTexture"] = ImageTexture;
		TokenEnums["rotate"] = TRS_Tokens;
		TokenEnums["point"] = Point;
		TokenEnums["Group"] = Group;
		TokenEnums["coordIndex"] = CoordsIndex;
		TokenEnums["children"] = Children;
		TokenEnums["coord"] = Coords;

		//Unit token
		TokenEnums["\n"] = Newline;
		TokenEnums["maxHealth"] = MaxHealth;
		TokenEnums["minAttack"] = MinAttack;
		TokenEnums["maxAttack"] = MaxAttack;
		TokenEnums["movementSpeed"] = MovementSpeed;
		TokenEnums["movement"] = Movement;
		TokenEnums["canBuild"] = CanBuild;
		TokenEnums["canResearch"] = CanResearch;
		TokenEnums["Models"] = Models;
		TokenEnums["maxPower"] = MaxPower;
		TokenEnums["Animation"] = Animation;
		TokenEnums["Time"] = Time;
		TokenEnums["isMobile"] = IsMobile;
		TokenEnums["attackMinRange"] = AttackMinRange;
		TokenEnums["attackMaxRange"] = AttackMaxRange;
		TokenEnums["attackAccuracy"] = AttackAccuracy;
		TokenEnums["attackSpeed"] = AttackSpeed;
		TokenEnums["sightRange"] = SightRange;
		TokenEnums["canAttack"] = CanAttack;
		TokenEnums["canAttackWhileMoving"] = CanAttackWhileMoving;
		TokenEnums["buildTime"] = BuildTime;
		TokenEnums["researchTime"] = ResearchTime;
		TokenEnums["hasAI"] = HasAI;
		TokenEnums["isResearched"] = IsResearched;
		TokenEnums["hurtByLight"] = HurtByLight;
		TokenEnums["Length"] = Length;
		TokenEnums["size"] = Size;
		TokenEnums["heightOnMap"] = HeightOnMap;
		TokenEnums["widthOnMap"] = WidthOnMap;
		TokenEnums["height"] = Height;
		TokenEnums["Unit"] = UnitDef;
		TokenEnums["symbol"] = UnitSymbol;
		TokenEnums["Model"] = ModelName;
		TokenEnums["powerIncrement"] = PowerIncrement;
		TokenEnums["powerUsage"] = PowerUsage;
		TokenEnums["lightPowerUsage"] = LightPowerUsage;
		TokenEnums["attackPowerUsage"] = AttackPowerUsage;
		TokenEnums["buildPowerUsage"] = BuildPowerUsage;
		TokenEnums["movePowerUsage"] = MovePowerUsage;
		TokenEnums["powerType"] = PowerType;
		TokenEnums["buildCost"] = BuildCost;
		TokenEnums["researchCost"] = ResearchCost;

		TokenEnums["DayLight"] = PowerDayLight;
		TokenEnums["TwentyFourSeven"] = PowerTwentyFourSeven;

		TokenEnums["lightRange"] = LightRange;

		TokenEnums["ProjectileType"] = ProjectileTypeStruct;
		TokenEnums["areaOfEffect"] = AreaOfEffect;
		TokenEnums["startPos"] = StartPos;
		TokenEnums["speed"] = Speed;

		TokenEnums["movementType"] = MovementType;
		TokenEnums["Vehicle"] = MovementTypeVehicle;
		TokenEnums["Sea"] = MovementTypeSea;
		TokenEnums["Building"] = MovementTypeBuilding;
		TokenEnums["Airborne"] = MovementTypeAirborne;
		TokenEnums["Tank"] = MovementTypeTank;
		TokenEnums["Human"] = MovementTypeHuman;
		TokenEnums["sound"] = Sound;
	}
	ModelParser::TokenType ModelParser::GetTokenType(vector<string>::iterator &token, vector<string> &tokens)
	{

		map<string, TokenType>::iterator TokenTypeIter = TokenEnums.find(*token);
		if(TokenTypeIter != TokenEnums.end())
			return (*TokenTypeIter).second;

		vector<string>::iterator tmp = token;
		tmp++;
		if(tmp != tokens.end())
		{
			if((*tmp).at(0) == '{')
			{
				return UnknownNode;
			}
			else if((*tmp).at(0) == '[')
			{
				return UnknownArray;
			}
		}
		return Unknown;
	}
	
	bool ModelParser::SearchAhead(vector<string>::iterator& CurrentPosition, vector<string>::iterator EndPosition,SearchTypes Find, char specifiedchar)
	{
		switch(Find)
		{
			case SpecifiedChar:
			{
				for(;CurrentPosition != EndPosition; CurrentPosition++)
				{
					if((*CurrentPosition).at(0) == specifiedchar)
					{
						if((*CurrentPosition).length() == 1)
							return true;
					}
				}
				return false;
			}
			case Number:
			{
				for(;CurrentPosition != EndPosition; CurrentPosition++)
				{
					DataType type = GetDataType(*CurrentPosition);
					if(type == Integer || type == Float)
					{
						return true;
					}
				}
				return false;				
			}
		}
		return false;
	}

	ModelParser::DataType ModelParser::GetDataType(string& token)
	{
		//Integer Number, only 0 - 9
		//Float number, only 0-9 and '-', '.'
		//String begins with '"'
		if(token.at(0) == '"' && token.at(token.length() - 1) == '"')
			return String;
		
		bool possibleFloat = false;
	
		// Check so that only allowed charachters exist in the token.
		string::iterator tokensIterator;
		for( tokensIterator = token.begin(); tokensIterator != token.end(); tokensIterator++ ) 
		{
			if((char)((*tokensIterator) >= '0' && (char)(*tokensIterator) <= '9') || (char)(*tokensIterator) == '-')
			{
				
			}
			else if((char)(*tokensIterator) == '.')
			{
				possibleFloat = true;
			}
			else
			{
				return UnknownDataType;
			}
		}
		
		if(possibleFloat == true)
		{
			return Float;
		}
		else
		{
			return Integer;
		}
	}
	
	inline bool ModelParser::GetNextType(vector<string>::iterator& position, vector<string>& tokens, ModelParser::DataType Type)
	{
		while(position != tokens.end())
		{
			if(GetDataType(*position) == Type)
				return true;
			position++;
		}
		if(position == tokens.end())
			return false;

		return false;
	}

	inline bool ModelParser::CheckAhead(vector<string>::iterator& position, vector<string>& tokens, int i)
	{
		int c = 0;
		vector<string>::iterator tmp = position;
		for(c = 0;c < i; c++)
		{
			tmp++;
			if(tmp == tokens.end())
				return false;
		}
		return true;
	}
	
	inline bool ModelParser::IsCoordinate(string& target)
	{
		char chr = target.at(0);
		if((chr >= '0' && chr <= '9') || chr == '.' || chr == '-')
			return true;
		else
			return false;
	}
	
	int ModelParser::Parse()
	{
		return Parse(mFile);
	}
	
	// Validates file header and extracts all tokens from a file into a string array.
	// It also counts brackets and array brackets to check for possible errors.
	// Remarks: Ignores all whitespace.
	int ModelParser::ProcessFile(string mFile, vector<string>& tokens)
	{
#ifdef PRINTOUT
		console << "Loading file '" << mFile << "'..." << Console::nl;
#endif
		// Load the file
		ifstream file;
		string buffer;
		string line;
		string filename = Utilities::GetDataFile(mFile);

		if (!filename.length())
			return MODEL_ERROR_FILE_NOT_FOUND;

		file.open(filename.c_str());
				
		if(file.good() == false)
			return MODEL_ERROR_FILE_NOT_FOUND;
		
//		getline(file, buffer);
		Utilities::ReadLineFromFile(file, buffer);
		StringTrim(buffer, line);

		if(ValidateHeader(line) == true)
		{
			buffer.clear();
			line.clear();
			int brackets = 0;
			int arrayc = 0;

			// Step 1 - Tokenize the entire file, Read all tokens, check so that every {,},[,] are finished in the proper order.
			while(file.eof() == false)
			{				
				Utilities::ReadLineFromFile(file, buffer);
				if(buffer.length() == 0)
					continue;
				
				StringTrim(buffer, line);
				
				if(line.at(0) == '#')
					continue;
				
				string token;
				
				int start = 0;
				int length = line.length();

				//Loop to the end of line or end of tokens
				while(start != length)
				{
					start = StringGetToken(line, token, start);
					switch((char)token.at(0))
					{
						case '{':
						{
							brackets++;
							if(brackets < 0)
							{
								file.close();
								return MODEL_ERROR_PARSE;
							}
							break;
						}
						case '}':
						{
							brackets--;
							if(brackets < 0)
							{
								file.close();
								return MODEL_ERROR_PARSE;
							}
						}
						case '[':
						{
							arrayc++;
							if(arrayc < 0)
							{
								file.close();
								return MODEL_ERROR_PARSE;
							}
							break;
						}
						case ']':
						{
							arrayc--;
							if(arrayc < 0)
							{
								file.close();
								return MODEL_ERROR_PARSE;
							}
							break;
						}
						case '\r':
						{
							continue;
							break;
						}
					}
					tokens.push_back(token);
#if PRINTTOKENS == 1
					console << token << Console::nl;
#endif
				}
				//Clean up.
				buffer.clear();
				line.clear();
			}
			file.close();
			if(brackets != 0)
			{
				return MODEL_ERROR_PARSE;
			}
			return SUCCESS;
		}
		return MODEL_ERROR_INVALID_FORMAT;
	}

	int ModelParser::ProcessTokens(vector<DataGroup*>& blocks, vector<string>& tokens)
	{
		DataGroup* currentDataGroup = NULL;
		VRMLData* current = NULL;
		UnitData* currentUnit = NULL;
		bool currentDefintion = false;
		bool saveDefinition = false;

		// Step 2 - Pick out brackets and interpret the data needed.
		bool ignore = false;
		bool ignorearray = false;

		int bracketc = 0;
		int arrayc = 0;
		int savec = 0;

		vector<string>::iterator tokensIterator;
		for( tokensIterator = tokens.begin(); tokensIterator != tokens.end(); tokensIterator++ ) 
		{
			switch((*tokensIterator)[0])
			{
				case '{':
				{
					if(ignore == true)
						bracketc++;
					else
						savec++;
					break;
				}
				case '}':
				{
					if(ignore == true)
					{
						bracketc--;
						ignore = false;
					}
					else
						savec--;

					if(savec == 0 && currentDefintion == true)
					{
						if(saveDefinition == true)
						{
							switch(currentDataGroup->type)
							{
								case Model:
								{
#ifdef DATADEBUG						
									console << "Save VRMLDataGroup" << Console::nl;
#endif							
									saveDefinition = false;
									blocks.push_back(currentDataGroup);
									currentDataGroup = NULL;
									current = NULL;
									break;
								}
								case Unit:
								{
#ifdef DATADEBUG						
									console << "Save UnitDataGroup" << Console::nl;
#endif							
									saveDefinition = false;
									blocks.push_back(currentDataGroup);
									currentDataGroup = NULL;
									currentUnit = NULL;
									break;
								}
							}
						}
						else
						{
							DeleteDataGroup(currentDataGroup);
						}
					}
					break;
				}
				case '[':
				{
					if(ignorearray == true)
					{
						arrayc++;
					}
					break;
				}
				case ']':
				{
					if(ignorearray == true)
					{
						arrayc--;
					}
					break;
				}
				default:
				{
					if(ignore == false)
					{
						if(currentDataGroup != NULL)
						{
							if(currentDataGroup->type == Model)
							{
								//Model tokens only
								ModelParser::TokenType token = GetTokenType(tokensIterator, tokens);
								switch(token)
								{
									case UnknownNode:
									case UnknownArray:
									{
										ignorearray = true;
										break;
									}
									case Unknown:
									{
										continue;
										break;
									}
									case Shape:
									{
										saveDefinition = true;
										continue;
									}
									case Geometry:
									{
										continue;
									}
									case Scale:
									case Translate:
									{
										//Read three floats
										VRMLTRS *obj = new VRMLTRS;
										tokensIterator++;
										int c = 0;
										while(tokensIterator != tokens.end())
										{
											DataType typ = GetDataType(*tokensIterator);
											if(typ == Float)
											{
												obj->value[c] = (float) atof((*tokensIterator).c_str());
												c++;
												if(c == 3)
													break;
											}
											tokensIterator++;
										}
										if(tokensIterator == tokens.end())
											return MODEL_ERROR_PARSE;

										CreateVRMLData(current);
										current->type = token;
										current->param.TRS = obj;
										break;
									}
									case Coords:
									{
										VRMLCoordinates * coordStruct = new VRMLCoordinates;
										coordStruct->type = FC_Solid;

										//First find Coordinate
										//Then find points
										bool foundCoordinate = false;
										bool foundPoint = false;
										tokensIterator++;
										while(tokensIterator != tokens.end())
										{
											TokenType currToken = GetTokenType(tokensIterator, tokens);
											if(currToken == Coords)
											{
												foundCoordinate = true;
											}
											else if(currToken == Point)
											{
												foundPoint = true;
												break;
											}
											tokensIterator++;
										}

										if(foundCoordinate == true && foundPoint == true)
										{
											bool foundClammer = false;
											bool foundEnd = false;
											while(tokensIterator != tokens.end())
											{
												if(foundClammer == true)
												{
													for(;;)
													{
														if((*tokensIterator).at(0) == ']')
														{
															foundEnd = true;
															break;
														}
														bool is = IsCoordinate(*tokensIterator);
														if(is == true)
															break;

														tokensIterator++;
														if(tokensIterator == tokens.end())
														{
															console << Console::err << "Parse error in Coordinate extraction" << Console::nl;
															return MODEL_ERROR_PARSE;
														}
													}

													if(foundEnd == true)
														break;

													if(CheckAhead(tokensIterator, tokens, 3) == true)
													{
														for(int i = 0; i < 3; i++)
														{
															float coord = (float) atof((*tokensIterator).c_str());
															coordStruct->points.push_back(coord);
															tokensIterator++;	
														}
													}
													else
													{
														console << Console::err << "Parse error in Coordinate extraction" << Console::nl;
														return MODEL_ERROR_PARSE;
													}
												}
												else
												{
													if((*tokensIterator).at(0) == '[')
													{
														foundClammer = true;
													}
													tokensIterator++;
												}
											}
											if(foundClammer == true && foundEnd == true)
											{
												//Search for }
												bool foundIt = false;
												while(tokensIterator != tokens.end())
												{
													if((*tokensIterator).at(0) == '}')
													{
														foundIt = true;
														break;
													}
													tokensIterator++;
												}
												if(foundIt == false)
												{
													console << Console::err << "Parse error in Coordinate extraction" << Console::nl;
													return MODEL_ERROR_PARSE;
												}
		#ifdef DATADEBUG									
												console << "* Coordinates..." << coordStruct->points.size() << "p" << Console::nl;
		#endif
												if(current == NULL || currentDataGroup == NULL)
													return MODEL_ERROR_UNEXPECTED_ERROR;
												CreateVRMLData(current);
												current->type = Coords;
												current->param.Coordinates = coordStruct;
											}
											else
											{
												console << Console::err << "Parse error in Coordinate extraction" << Console::nl;
												return MODEL_ERROR_PARSE;
											}

										}
										else
										{
											console << Console::err << "Parse error in Coordinate extraction" << Console::nl;
											return MODEL_ERROR_PARSE;
										}
										break;
									}
									case TextureCoords:
									{
										//Find Point
										//Find [
										//Find floats, iterate until ] is found
										while(tokensIterator != tokens.end())
										{
											if(GetTokenType(tokensIterator,tokens) == Point)
												break;
											tokensIterator++;
										}
										if(tokensIterator == tokens.end())
										{
											console << Console::err << "Parse error in Texture Coordinate extraction" << Console::nl;
											return MODEL_ERROR_PARSE;
										}

										if(SearchAhead(tokensIterator, tokens.end(), SpecifiedChar, '[') == false)
										{
											console << Console::err << "Parse error in Texture Coordinate extraction" << Console::nl;
											return MODEL_ERROR_PARSE;
										}
										VRMLCoordinates* coords = new VRMLCoordinates;
										coords->type = FC_Texture;
										
										while((*tokensIterator).at(0) != ']')
										{
											DataType dataType = GetDataType(*tokensIterator);
											if(dataType == Float)
											{
												float nb = (float) atof((*tokensIterator).c_str());
												
												coords->points.push_back(nb);
											}
											tokensIterator++;
											if(tokensIterator == tokens.end())
											{
												console << Console::err << "Parse error in Texture Coordinate extraction" << Console::nl;
												return MODEL_ERROR_PARSE;
											}
										}
		#ifdef DATADEBUG
										console << "* Texture Coordinates..." << coords->points.size() << "p" << Console::nl;
		#endif
										CreateVRMLData(current);
										current->type = TextureCoords;
										current->param.Coordinates = coords;
										break;
									}
									case CoordsIndex:
									{
										//Find [
										//Find number, loop to -1, find a number, loop to -1
										//until ] is found.
										if(SearchAhead(tokensIterator, tokens.end(), SpecifiedChar, '[') == false)
										{
											console << Console::err << "Parse error in Face extraction" << Console::nl;
											return MODEL_ERROR_PARSE;
										}
										if(SearchAhead(tokensIterator, tokens.end(), Number, 0) == false)
										{
											console << Console::err << "Parse error in Face extraction" << Console::nl;
											return MODEL_ERROR_PARSE;
										}

										VRMLFaces *faces = new VRMLFaces;
										faces->type = FC_Solid;
										
										int faceCount = 0;
		#ifdef INVERTFACES								
										int points = 0;
		#endif								
										//Numbers, 0 1 2 3 -1, ['\n']
										while((*tokensIterator).at(0) != ']')
										{
											DataType dataType = GetDataType(*tokensIterator);
											if(dataType == Integer)
											{
												int nb = atoi((*tokensIterator).c_str());
		#ifdef INVERTFACES										
												points++;
		#endif										
												faces->points.push_back(nb);
												if(nb == -1)
												{
													faceCount++;
		#ifdef INVERTFACES											
													points--;
													//ReOrderPoints.
													int *tmp = new int[points];
													vector<int>::reverse_iterator iter = faces->points.rbegin();
													iter += 1; //1 for end, 1 for -1
													for(int i = 0; i < points; i++, iter++)
													{
														tmp[i] = (*iter);
													}
													*iter--;
													for(int i = 0; i < points; i++, iter--)
													{
														(*iter) = tmp[i];
													}
													delete [] tmp;
													points = 0;
		#endif
												}
											}
											tokensIterator++;
											if(tokensIterator == tokens.end())
											{
												console << Console::err << "Parse error in Face extraction" << Console::nl;
												return MODEL_ERROR_PARSE;
											}
										}
		#ifdef DATADEBUG								
										console << "* Faces..." << faceCount << "f/" << faces->points.size() << "p" << Console::nl;
		#endif
										if(faces->points.size() == 0)
											return MODEL_ERROR_UNEXPECTED_ERROR;
										faces->faceCount = faceCount;
										CreateVRMLData(current);
										current->type = CoordsIndex;
										current->param.Faces = faces;
										break;
									}
									case TextureCoordsIndex:
									{
										//As above
										//Find [
										//Find number, loop to -1, find a number, loop to -1
										//until ] is found.
										if(SearchAhead(tokensIterator, tokens.end(), SpecifiedChar, '[') == false)
										{
											console << Console::err << "Parse error in Texture Face extraction" << Console::nl;
											return MODEL_ERROR_PARSE;
										}
										if(SearchAhead(tokensIterator, tokens.end(), Number, 0) == false)
										{
											console << Console::err << "Parse error in Texture Face extraction" << Console::nl;
											return MODEL_ERROR_PARSE;
										}

										VRMLFaces *faces = new VRMLFaces;
										faces->type = FC_Texture;
										
										int faceCount = 0;
		#ifdef INVERTTEXFACES
										int points = 0;
		#endif
										//Numbers, 0 1 2 3 -1, ['\n']
										while((*tokensIterator).at(0) != ']')
										{
											DataType dataType = GetDataType(*tokensIterator);
											if(dataType == Integer)
											{
												int nb = atoi((*tokensIterator).c_str());
		#ifdef INVERTTEXFACES
												points++;
		#endif
												faces->points.push_back(nb);
												if(nb == -1)
												{
													faceCount++;
		#ifdef INVERTTEXFACES											
													points--;
													//ReOrderPoints.
													int *tmp = new int[points];
													vector<int>::reverse_iterator iter = faces->points.rbegin();
													iter += 1; //1 for end, 1 for -1
													for(int i = 0; i < points; i++, iter++)
													{
														tmp[i] = (*iter);
													}
													*iter--;
													for(int i = 0; i < points; i++, iter--)
													{
														(*iter) = tmp[i];
													}
													delete [] tmp;
													points = 0;
		#endif											
												}
											}
											tokensIterator++;
											if(tokensIterator == tokens.end())
											{
												console << Console::err << "Parse error in Texture Face extraction" << Console::nl;
												return MODEL_ERROR_PARSE;
											}
										}
		#ifdef DATADEBUG								
										console << "* Texture Faces..." << faceCount << "f/" << faces->points.size() << "p" << Console::nl;
		#endif
										if(faces->points.size() == 0)
											return MODEL_ERROR_UNEXPECTED_ERROR;
										faces->faceCount = faceCount;
										CreateVRMLData(current);
										current->type = TextureCoordsIndex;
										current->param.Faces = faces;
										break;
									}
									case Appearance: //Material extraction
									{
										//Find Material, Extract all, find }
										while(tokensIterator != tokens.end())
										{
											if(GetTokenType(tokensIterator, tokens) == Material)
											{
												//Find {
												while(tokensIterator != tokens.end())
												{
													if((*tokensIterator).at(0) == '{')
														break;
													tokensIterator++;
												}
												if(tokensIterator == tokens.end())
													return MODEL_ERROR_PARSE;

												break;
											}
											tokensIterator++;
										}
										
										VRMLMaterial *mat = new VRMLMaterial;
										mat->ambientIntensity = 0.0f;
										mat->diffuseColor[0] = 0.0f;
										mat->diffuseColor[1] = 0.0f;
										mat->diffuseColor[2] = 0.0f;
										mat->specularColor[0] = 0.0f;
										mat->specularColor[1] = 0.0f;
										mat->specularColor[2] = 0.0f;
										mat->emissiveColor[0] = 0.0f;
										mat->emissiveColor[1] = 0.0f;
										mat->emissiveColor[2] = 0.0f;
										mat->shininess = 0.0f;
										mat->transperency = 0.0f;

										//Walk troungh material parameters
										while(tokensIterator != tokens.end())
										{
											//loop until } is found.
											if((*tokensIterator).at(0) == '}')
											{
												break;
											}
											TokenType tok = GetTokenType(tokensIterator, tokens);
											switch(tok)
											{
												case Material_Ambient: //1 float
												{
													if(GetNextType(tokensIterator, tokens, Float) == true)
														mat->ambientIntensity = (float) atof((*tokensIterator).c_str());
													else
														return MODEL_ERROR_PARSE;
													break;
												}
												case Material_Diffuse: //3 float
												{
													for(int i = 0; i < 3; i++)
													{
														if(GetNextType(tokensIterator, tokens, Float) == true)
															mat->diffuseColor[i] = (float) atof((*tokensIterator).c_str());
														else
															return MODEL_ERROR_PARSE;
														tokensIterator++;
													}
													break;
												}
												case Material_Specular: //3 float
												{
													for(int i = 0; i < 3; i++)
													{
														if(GetNextType(tokensIterator, tokens, Float) == true)
															mat->specularColor[i] = (float) atof((*tokensIterator).c_str());
														else
															return MODEL_ERROR_PARSE;
														tokensIterator++;
													}
														break;
												}
												case Material_Transperency: //1 float
												{
													if(GetNextType(tokensIterator, tokens, Float) == true)
														mat->transperency = (float) atof((*tokensIterator).c_str());
													else
														return MODEL_ERROR_PARSE;
														break;
												}
												case Material_Shininess: //1 float
												{
													if(GetNextType(tokensIterator, tokens, Float) == true)
														mat->shininess = (float) atof((*tokensIterator).c_str());
													else
														return MODEL_ERROR_PARSE;
												
													break;
												}
												default:
												{
													tokensIterator++;
													break;
												}
											}
										}

										if(tokensIterator == tokens.end())
											return MODEL_ERROR_PARSE;

										CreateVRMLData(current);
										current->type = Material;
										current->param.Material = mat;
										break;
									}
									case Texture:
									{
										//Find ImageTexture, then { and then url and }.
										tokensIterator++;
										while(tokensIterator != tokens.end())
										{
											//Find ImageTexture
											TokenType tok = GetTokenType(tokensIterator, tokens);
											if(tok == ImageTexture)
											{
												tokensIterator++;
												if((*tokensIterator).at(0) != '{')
												{
													return MODEL_ERROR_PARSE;
												}
												break;
											}
											tokensIterator++;
										}
										if(tokensIterator == tokens.end())
											return MODEL_ERROR_PARSE;

										//Find url
										while(tokensIterator != tokens.end())
										{
											TokenType tok = GetTokenType(tokensIterator, tokens);
											if(tok == URL)
											{
												break;
											}
											tokensIterator++;
										}
										if(tokensIterator == tokens.end())
											return MODEL_ERROR_PARSE;

										//The next token SHOULD contain the url.
										tokensIterator++;
										if((*tokensIterator).at(0) == '"')
										{
											//Extract the name.
											string Path = (*tokensIterator).substr(1,(*tokensIterator).size() - 2);
											VRMLTextureURL *url = new VRMLTextureURL;
											url->path = Path;
											CreateVRMLData(current);
											current->type = URL;
											current->param.URL = url;
										}
										else
										{
											return MODEL_ERROR_PARSE;
										}
										break;
									}
									default:
										break;
								}
							}
							else if(currentDataGroup->type == Unit)
							{
								//Unit tokens only
								ModelParser::TokenType token = GetTokenType(tokensIterator, tokens);
								switch(token)
								{
									case UnknownNode:
									case UnknownArray:
									{
										ignorearray = true;
										break;
									}
									case Unknown:
									{
										continue;
										break;
									}
									case MaxHealth:
									case MinAttack:
									case MaxPower:
									case MaxAttack:
									case HeightOnMap:
									case WidthOnMap:
									case IsMobile:
									case HasAI:
									case IsResearched:
									case HurtByLight:
									case CanAttack:
									case BuildCost:
									case ResearchCost:
									case CanAttackWhileMoving:
									{
										//Parse an int.
										if(!ReadUnitParameter(token, Integer, currentUnit, tokensIterator, tokens))
										{
											console << Console::err << "Int Parse Error" << Console::nl;
											return MODEL_ERROR_PARSE;
										}
										break;
									}
									case MovementType:
									case PowerType:
									{
										CreateUnitData(currentUnit);
										currentUnit->type = token;
										tokensIterator++;
										ModelParser::TokenType tmpToken = GetTokenType(tokensIterator, tokens);
										currentUnit->param.enums = tmpToken;
										break;
									}
									case ProjectileTypeStruct:
									{
										//Find {
										while(tokensIterator != tokens.end())
										{
											if((*tokensIterator).at(0) == '{')
												break;
											tokensIterator++;
										}
										if(tokensIterator == tokens.end())
											return MODEL_ERROR_PARSE;
										
										UnitProjectileType *projectileTypeData = new UnitProjectileType;
										projectileTypeData->model = NULL;
										projectileTypeData->size = 0.0f;
										projectileTypeData->areaOfEffect = 0.0f;
										projectileTypeData->speed = 0.0f;
										projectileTypeData->startPos = NULL;

										while(tokensIterator != tokens.end())
										{
											if((*tokensIterator).at(0) == '}')
												break;
											TokenType tmp = GetTokenType(tokensIterator, tokens);
											switch(tmp)
											{
												case ModelName:
												{
													DataParameter tmp;
													if(!ReadParameter(String, tmp, tokensIterator, tokens))
													{
														return MODEL_ERROR_PARSE;
													}
													projectileTypeData->model = tmp.str;

												}
												case Size:
												{
													DataParameter tmp;
													if(!ReadParameter(Float, tmp, tokensIterator, tokens))
													{
														return MODEL_ERROR_PARSE;
													}
													projectileTypeData->size = tmp.floating;
													break;
												}
												case AreaOfEffect:
												{
													DataParameter tmp;
													if(!ReadParameter(Float, tmp, tokensIterator, tokens))
													{
														return MODEL_ERROR_PARSE;
													}
													projectileTypeData->areaOfEffect = tmp.floating;
													break;
												}
												case Speed:
												{
													DataParameter tmp;
													if(!ReadParameter(Float, tmp, tokensIterator, tokens))
													{
														return MODEL_ERROR_PARSE;
													}
													projectileTypeData->speed = tmp.floating;
													break;
												}
												case StartPos:
												{
													DataParameter tmp;
													if(!ReadParameter(Coordinate3D, tmp, tokensIterator, tokens))
													{
														return MODEL_ERROR_PARSE;
													}
													projectileTypeData->startPos = tmp.floatArray;
													break;
												}
												default:
													break;
											}
											tokensIterator++;
										}
										CreateUnitData(currentUnit);
										currentUnit->type = ProjectileTypeStruct;
										currentUnit->param.projectileType = projectileTypeData;
										break;
									}
									case AttackMinRange:
									case AttackMaxRange:
									case AttackAccuracy:
									case SightRange:
									case AttackSpeed:
									case MovementSpeed:
									case Size:
									case Height:
									case PowerIncrement:
									case PowerUsage:
									case LightPowerUsage:
									case AttackPowerUsage:
									case BuildPowerUsage:
									case MovePowerUsage:
									case LightRange:
									case BuildTime:
									case ResearchTime:
									{
										//Parse an float.
										if(!ReadUnitParameter(token, Float, currentUnit, tokensIterator, tokens))
										{
											console << Console::err << "Float Parse Error" << Console::nl;
											return MODEL_ERROR_PARSE;
										}
										break;	
									}
									case Sound:
									case CanBuild:
									case CanResearch:
									{
										if(!ReadUnitArray(token, String, currentUnit, tokensIterator, tokens))
										{
											console << Console::err << "String Array Error" << Console::nl;
											return MODEL_ERROR_PARSE;
										}

										break;
									}
									case UnitSymbol:
									case ModelName:
									{
										if(!ReadUnitParameter(token, String, currentUnit, tokensIterator, tokens))
											return MODEL_ERROR_PARSE;
										break;
									}
		/*
									case movement:
									case Models:
									case Animation:
									case Time:
									case Length:
		*/

									default:
										break;
								}
							}
						}
						else
						{
							//Check for definition only
							ModelParser::TokenType token = GetTokenType(tokensIterator, tokens);
							switch(token)
							{
								case UnknownNode:
								case UnknownArray:
								{
									ignorearray = true;
									break;
								}
								case Unknown:
								{
									continue;
									break;
								}
								case Definition:
								{
									vector<string>::iterator pointer;
									pointer = tokensIterator;
									if(CheckAhead(pointer, tokens, 2) == true)
									{
										pointer++;
										std::string defName = *pointer;
										if(defName.at(0) == '"') //Strip it
										{
											defName = defName.substr(1, defName.size() - 2);
										}
										pointer++;
										TokenType defType = GetTokenType(pointer,tokens);
										switch(defType)
										{
											case Transform:
											{
												currentDefintion = true;
												currentDataGroup = new DataGroup;
												currentDataGroup->type = Model;
												current =  new VRMLData;
												current->nxt = NULL;
												currentDataGroup->param.VRML_Begin = current;
												current->type = Definition;
												current->param.Definition = new VRMLDefinition;
												current->param.Definition->name = defName;
												current->param.Definition->type = dTransform;
												ignore = false;
												break;
											}
											case Group:
											{
												if(currentDataGroup == NULL || current == NULL)
													return MODEL_ERROR_UNEXPECTED_ERROR;
	#ifdef DATADEBUG
												if(current->type == Definition)
													console << "DEF Model " << current->param.Definition->name << Console::nl;
	#endif
												CreateVRMLData(current);
												current->type = Definition;
												current->param.Definition = new VRMLDefinition;
												current->param.Definition->name = defName;
												current->param.Definition->type = dGroup;
												ignore = false;
												break;
											}
											case UnitDef:
											{
	#ifdef DATADEBUG
												console << "DEF Unit '" << defName << "'" << Console::nl;
	#endif
												currentDefintion = true;
												currentDataGroup = new DataGroup;
												currentDataGroup->type = Unit;
												currentUnit =  new UnitData;
												currentUnit->nxt = NULL;
												currentDataGroup->param.Unit_Begin = currentUnit;
												currentUnit->type = Definition;
												currentUnit->param.str = new string();
												*currentUnit->param.str = defName;
												saveDefinition = true;
												ignore = false;
												break;
											}
											default:
											{
												ignore = true;
												continue;
												break;
											}
										}
									}
									else
									{
										return MODEL_ERROR_PARSE;
									}
									break;
								}
								default:
									break;
							}
						}
					}
					break;
				}
			}
		}
		return SUCCESS;
	}

	// Assembles raw data into usable inner handling structures and disposes all unneeded raw data.
	int ModelParser::ProcessGroups(vector<DataGroup*>& blocks, vector<innerHandling*>& ObjectBlocks)
	{
		VRMLData *current = NULL;
		UnitData *currentUnit = NULL;
		//Step 3 - Put it all together, deallocate what is not needed and put together and push back in the ObjectBlocks the rest.
		int blockCounter = 0;
		for(vector<DataGroup*>::iterator blocksIterator = blocks.begin(); blocksIterator != blocks.end(); blocksIterator++)
		{
			if((*blocksIterator)->type == Model)
			{
				InnerModel* Model;
				CreateInnerModel(Model);

				current = (*blocksIterator)->param.VRML_Begin;
				while(current != NULL)
				{
					switch(current->type)
					{
						 case Coords:
						 {
#ifdef PRINTOUT						 
							console << "* Coordinates" << Console::nl;
#endif							
						 	Model->Coordinates = current->param.Coordinates;
							DeleteOnlyAndContinue(current);
						 	break;
						 }
						 case TextureCoords:
						 {
#ifdef PRINTOUT						 
							 console << "* Texture Coordinates" << Console::nl;
#endif							
						 	Model->TextureCoordinates = current->param.Coordinates;
							DeleteOnlyAndContinue(current);
						 	break;
						 }
						 case CoordsIndex:
						 {
#ifdef PRINTOUT						 
							 console << "* Faces" << Console::nl;
#endif							
						 	Model->CoordinateFaces = current->param.Faces;
							DeleteOnlyAndContinue(current);
						 	break;
						 }
						 case TextureCoordsIndex:
						 {
#ifdef PRINTOUT						 
							console << "* Texture Faces" << Console::nl;
#endif							
						 	Model->TextureFaces = current->param.Faces;
							DeleteOnlyAndContinue(current);
							break;
						 }
						 case Definition:
						 {
						 	if(current->param.Definition->type == dTransform)
							{
#ifdef PRINTOUT							
								console << "Model: " << current->param.Definition->name << Console::nl;
#endif								
								Model->Name = current->param.Definition;
								DeleteOnlyAndContinue(current);
							}
							else
							{
								DeleteAndContinue(current);
							}
							break;
						 }
						 case Material:
						 {
						 	Model->Material = current->param.Material;
#ifdef PRINTOUT
							console << "* Materials" << Console::nl;
#endif
							DeleteOnlyAndContinue(current);
							break;
						 }
						 case URL:
						 {
						 	Model->TextureURL = current->param.URL->path;
#ifdef PRINTOUT
							console << "* Texture URL: " << Model->TextureURL << Console::nl;
#endif
							DeleteAndContinue(current);
						 	break;
						 }
						 case Scale:
						 {
						 	Model->TRS_Scale = current->param.TRS;
							DeleteOnlyAndContinue(current);
						 	break;
						 }
						 case Translate:
						 {
						 	Model->TRS_Translate = current->param.TRS;
							DeleteOnlyAndContinue(current);
						 	break;
						 }
						 default:
						 {
						 	DeleteAndContinue(current);
						 	break;
						 }
					}
				}
				innerHandling* ModelHandler = new innerHandling;
				ModelHandler->type = tModel;
				ModelHandler->data.typeModel = Model;

				//Save it
				ObjectBlocks.push_back(ModelHandler);
				blockCounter++;
			}
			else if((*blocksIterator)->type == Unit)
			{
				InnerUnit* Unit;
				CreateInnerUnit(Unit);

				currentUnit = (*blocksIterator)->param.Unit_Begin;
				while(currentUnit != NULL)
				{
					switch(currentUnit->type)
					{
						FETCH_UNIT_VALUE(MovementType, movementType, enums)
						FETCH_UNIT_VALUE(PowerType, powerType, enums)
						FETCH_UNIT_VALUE(MaxHealth, maxHealth, integer)
						FETCH_UNIT_VALUE(MinAttack, minAttack, integer)
						FETCH_UNIT_VALUE(MaxPower, maxPower, integer)
						FETCH_UNIT_VALUE(MaxAttack, maxAttack, integer)
						FETCH_UNIT_VALUE(HeightOnMap, heightOnMap, integer)
						FETCH_UNIT_VALUE(WidthOnMap, widthOnMap, integer)
						FETCH_UNIT_VALUE(BuildCost, buildCost, integer)
						FETCH_UNIT_VALUE(ResearchCost, researchCost, integer)
						FETCH_UNIT_VALUE(BuildTime, buildTime, integer)
						FETCH_UNIT_VALUE(ResearchTime, researchTime, integer)
						case IsMobile:
						{
							if(currentUnit->param.integer == 0)
								Unit->isMobile = false;
							else
								Unit->isMobile = true;
							DeleteOnlyAndContinue(currentUnit);
							break;
						}
						case HasAI:
						{
							if(currentUnit->param.integer == 0)
								Unit->hasAI = false;
							else
								Unit->hasAI = true;
							DeleteOnlyAndContinue(currentUnit);
							break;
						}
						case IsResearched:
						{
							if(currentUnit->param.integer == 0)
								Unit->isResearched = false;
							else
								Unit->isResearched = true;
							DeleteOnlyAndContinue(currentUnit);
							break;
						}
						case HurtByLight:
						{
							if(currentUnit->param.integer == 0)
								Unit->hurtByLight = false;
							else
								Unit->hurtByLight = true;
							DeleteOnlyAndContinue(currentUnit);
							break;
						}
						case CanAttack:
						{
							if(currentUnit->param.integer == 0)
								Unit->canAttack = false;
							else
								Unit->canAttack = true;
							DeleteOnlyAndContinue(currentUnit);								
							break;
						}
						case CanAttackWhileMoving:
						{
							if(currentUnit->param.integer == 0)
								Unit->canAttackWhileMoving = false;
							else
								Unit->canAttackWhileMoving = true;
							DeleteOnlyAndContinue(currentUnit);								
							break;
						}
						case Definition:
						{
							console << "Unit: " << *(currentUnit->param.str) << Console::nl;
							Unit->Name = currentUnit->param.str;
							DeleteOnlyAndContinue(currentUnit);
							break;
						}
						FETCH_UNIT_VALUE(ProjectileTypeStruct, projectileType, projectileType)
						FETCH_UNIT_VALUE(ModelName, Model, str)
						FETCH_UNIT_VALUE(UnitSymbol, Symbol, str)
						FETCH_UNIT_VALUE(CanBuild, canBuild, stringArray)
						FETCH_UNIT_VALUE(CanResearch, canResearch, stringArray)
						FETCH_UNIT_VALUE(Sound, sound, stringArray)
						FETCH_UNIT_VALUE(AttackSpeed, attackSpeed, floating)
						FETCH_UNIT_VALUE(AttackMinRange, attackMinRange, floating)
						FETCH_UNIT_VALUE(AttackMaxRange, attackMaxRange, floating)
						FETCH_UNIT_VALUE(AttackAccuracy, attackAccuracy, floating)
						FETCH_UNIT_VALUE(SightRange, sightRange, floating)
						FETCH_UNIT_VALUE(MovementSpeed, movementSpeed, floating)
						FETCH_UNIT_VALUE(Size, size, floating)
						FETCH_UNIT_VALUE(Height, height, floating)
						FETCH_UNIT_VALUE(LightRange, lightRange, floating)	
						FETCH_UNIT_VALUE(PowerIncrement, powerIncrement, floating)
						FETCH_UNIT_VALUE(PowerUsage, powerUsage, floating)
						FETCH_UNIT_VALUE(LightPowerUsage, lightPowerUsage, floating)
						FETCH_UNIT_VALUE(AttackPowerUsage, attackPowerUsage, floating)
						FETCH_UNIT_VALUE(BuildPowerUsage, buildPowerUsage, floating)
						FETCH_UNIT_VALUE(MovePowerUsage, movePowerUsage, floating)
						default:
						{
							DeleteAndContinue(currentUnit);
							break;
						}
					}	
				}
				innerHandling* ModelHandler = new innerHandling;
				ModelHandler->type = tUnit;
				ModelHandler->data.typeUnit = Unit;
				//Save it
				ObjectBlocks.push_back(ModelHandler);
				blockCounter++;				
			}
			delete *blocksIterator;
		}
		if(blockCounter == 0)
		{
#ifdef PRINTOUT
			console << Console::imp << "No Objects!" << Console::nl;
#endif			
		}
		return SUCCESS;
	}
	
	int ModelParser::PrepareForGameCore(vector<innerHandling*>& ObjectBlocks, string& mFile)
	{
		//Step 4 - For Model: Calculate normals and process the points. For UnitType: Process parameters
		for(vector<innerHandling*>::iterator handleIter = ObjectBlocks.begin(); handleIter != ObjectBlocks.end(); handleIter++)
		{
			if((*handleIter)->type == tModel)
			{
				InnerModel* Model = (*handleIter)->data.typeModel;
				
				//Initate vectors for performance.
				Vector3D vertex_one(0,0,0);
				Vector3D vertex_two(0,0,0);
				Vector3D vertex_three(0,0,0);				
				Vector3D normal(0,0,0);

				//Calculate Triangle Normals
				vector<int>* AssociatedWith = new vector<int>[Model->Coordinates->points.size() / 3];

				//Initate
				float *Normals = new float[Model->CoordinateFaces->faceCount * 3 + 3];

				//Foreach face calculate normals of the first three coordinates
				vector<int> triangles;
				vector<int> tex_triangles;
				int n = 0;
//				int tricount = 0; << unused!

				float Verticies[9];
					
				//Process all faces and generate triangles
				vector<int>::iterator facesIter;
				vector<int>::iterator texfacesIter;
				bool with_texcoords = false;

				//Determine with or without textures
				if(Model->TextureCoordinates == NULL)
				{
					with_texcoords = false;
					facesIter = Model->CoordinateFaces->points.begin();
				}
				else
				{
					with_texcoords = true;
					facesIter = Model->CoordinateFaces->points.begin();
					texfacesIter = Model->TextureFaces->points.begin();
				}

				while(1)
				{
					//Determine verticle count
					int tri_count = 0;
					for(;facesIter != Model->CoordinateFaces->points.end(); facesIter++)
					{
						if(*facesIter == -1)
							break;
						else
							tri_count++;
					}
					facesIter -= tri_count;
	
					switch(tri_count)
					{
						case 3: //Triangle
						{
							//Add three coordinates
							if(with_texcoords == false)
							{
								for(int i = 0; (i < 3) && (facesIter != Model->CoordinateFaces->points.end()); i++, facesIter++)
								{
									int c = (*facesIter) * 3;
									//Get Three points, calculate normal
									Verticies[i * 3] = Model->Coordinates->points[c];
									Verticies[i * 3 + 1] = Model->Coordinates->points[c + 1];
									Verticies[i * 3 + 2] = Model->Coordinates->points[c + 2];
									triangles.push_back(*facesIter);
									AssociatedWith[(*facesIter)].push_back(n);
								}
							}
							else
							{
								for(int i = 0; (i < 3) && ((facesIter != Model->CoordinateFaces->points.end()) && (texfacesIter != Model->TextureFaces->points.end())); i++, texfacesIter++, facesIter++)
								{
									int c = (*facesIter) * 3;
									//Get Three points, calculate normal
									Verticies[i * 3] = Model->Coordinates->points[c];
									Verticies[i * 3 + 1] = Model->Coordinates->points[c + 1];
									Verticies[i * 3 + 2] = Model->Coordinates->points[c + 2];
									triangles.push_back(*facesIter);
									tex_triangles.push_back(*texfacesIter);
	
									AssociatedWith[(*facesIter)].push_back(n);					
								}
							}
							//Calculate face normal
							vertex_one.x = Verticies[0];
							vertex_one.y = Verticies[1];
							vertex_one.z = Verticies[2];
							vertex_two.x = Verticies[3];
							vertex_two.y = Verticies[4];
							vertex_two.z = Verticies[5];
							vertex_three.x = Verticies[6];
							vertex_three.y = Verticies[7];
							vertex_three.z = Verticies[8];
					
							normal.set(0,0,0);
		
							//FaceVector = (v1 - v0) X (v0 - v2)
							normal = vertex_one - vertex_three;
							normal.cross(vertex_two - vertex_one);
							normal.normalize();
							Normals[n * 3] = normal.x;
							Normals[n * 3 + 1] = normal.y;
							Normals[n * 3 + 2] = normal.z;
							n++;
							break;
						}
						case 4: //Quad
						{
							//Add three coordinates
							if(with_texcoords == false)
							{
								int val = (*facesIter); //First verticle

								//First triangle
								for(int i = 0; (i < 3) && (facesIter != Model->CoordinateFaces->points.end()); i++, facesIter++)
								{
									int c = (*facesIter) * 3;
									//Get Three points, calculate normal
									Verticies[i * 3] = Model->Coordinates->points[c];
									Verticies[i * 3 + 1] = Model->Coordinates->points[c + 1];
									Verticies[i * 3 + 2] = Model->Coordinates->points[c + 2];
									triangles.push_back(*facesIter);
									AssociatedWith[(*facesIter)].push_back(n);
								}
								facesIter--;

								//Second triangle
								for(int i = 0; (i < 2) && (facesIter != Model->CoordinateFaces->points.end()); i++, facesIter++)
								{
									triangles.push_back(*facesIter);
									AssociatedWith[(*facesIter)].push_back(n + 1);
								}
								triangles.push_back(val);
								AssociatedWith[val].push_back(n + 1);
							}
							else
							{
								int val = (*facesIter);
								int val_tex = (*texfacesIter);

								for(int i = 0; (i < 3) && ((facesIter != Model->CoordinateFaces->points.end()) && (texfacesIter != Model->TextureFaces->points.end())); i++, texfacesIter++, facesIter++)
								{
									int c = (*facesIter) * 3;
									//Get Three points, calculate normal
									Verticies[i * 3] = Model->Coordinates->points[c];
									Verticies[i * 3 + 1] = Model->Coordinates->points[c + 1];
									Verticies[i * 3 + 2] = Model->Coordinates->points[c + 2];
									triangles.push_back(*facesIter);
									tex_triangles.push_back(*texfacesIter);
									AssociatedWith[(*facesIter)].push_back(n);					
								}
								facesIter--;
								texfacesIter--;
								
								//Second triangle
								for(int i = 0; (i < 2) && (facesIter != Model->CoordinateFaces->points.end()); i++, facesIter++)
								{
									triangles.push_back(*facesIter);
									tex_triangles.push_back(*texfacesIter);
									AssociatedWith[(*facesIter)].push_back(n);
								}
								triangles.push_back(val);
								tex_triangles.push_back(val_tex);
								AssociatedWith[val].push_back(n);
							}
							//Calculate face normal
							vertex_one.x = Verticies[0];
							vertex_one.y = Verticies[1];
							vertex_one.z = Verticies[2];
							vertex_two.x = Verticies[3];
							vertex_two.y = Verticies[4];
							vertex_two.z = Verticies[5];
							vertex_three.x = Verticies[6];
							vertex_three.y = Verticies[7];
							vertex_three.z = Verticies[8];
					
							normal.set(0,0,0);
		
							//FaceVector = (v1 - v0) X (v0 - v2)
							normal = vertex_one - vertex_three;
							normal.cross(vertex_two - vertex_one);
							normal.normalize();
							Normals[n * 3] = normal.x;
							Normals[n * 3 + 1] = normal.y;
							Normals[n * 3 + 2] = normal.z;
							
							n++;
/*							Normals[n * 3] = normal.x;
							Normals[n * 3 + 1] = normal.y;
							Normals[n * 3 + 2] = normal.z;*/
							break;
						}
						default: //Unsupported length
						{
#ifdef PRINTOUT					
							// Don't bother ask me why I choose 'points in cloud'... i don't know...
							console << Console::err << "Polygon Mismatch, counted " << tri_count << " points in cloud" << Console::nl;
#endif							
							facesIter += tri_count;
							break;
						}
					}
					//for(facesIter = Model->CoordinateFaces->points.begin(); facesIter != Model->CoordinateFaces->points.end(); facesIter++, n++)
					
					if(with_texcoords == true)
					{
						if(facesIter == Model->CoordinateFaces->points.end() || texfacesIter == Model->TextureFaces->points.end())
							break;
						else
						{
							facesIter++;
							texfacesIter++;
						}
						if(facesIter == Model->CoordinateFaces->points.end() || texfacesIter == Model->TextureFaces->points.end())
							break;
					}
					else
					{
						if(facesIter == Model->CoordinateFaces->points.end())
							break;
						else
						{
							facesIter++;
						}
						if(facesIter == Model->CoordinateFaces->points.end())
							break;
					}

				}
////END
				//Initiate vertex normals
				//All coordinates has been grouped, calculate vertex normals.
				float* VertexNormals = new float[Model->Coordinates->points.size()];
				for(unsigned int i = 0; i < (Model->Coordinates->points.size() / 3); i++)
				{
					//Add all normals for the faces which uses the vertex coordinate and then normalize.
					normal.set(0,0,0);
					for(vector<int>::iterator dataIter = AssociatedWith[i].begin(); dataIter != AssociatedWith[i].end(); dataIter++)
					{
						vertex_one.x = Normals[*dataIter * 3];
						vertex_one.y = Normals[*dataIter * 3 + 1];
						vertex_one.z = Normals[*dataIter * 3 + 2];
						normal += vertex_one;
					}

					normal.normalize();
					VertexNormals[i * 3] = normal.x;
					VertexNormals[i * 3 + 1] = normal.y;
					VertexNormals[i * 3 + 2] = normal.z;
#ifdef NORMALDEBUG
					console << "["<< i << "] Normal: { " << normal.x << ", " << normal.y << ", " << normal.z << " }" << Console::nl;
#endif
				}

				//face normals are not needed anymore freeup the memory
				delete [] Normals;
				//Coordinate grouping data is note needed anymore.
				delete [] AssociatedWith;
				//All normals has been calculated, faces, coordinates are finished...
				//Vertex Normals, Triangle index, coordinates
				
				Game::Dimension::Model* ModelData = new Game::Dimension::Model;
				ModelData->normals = NULL;
				ModelData->tris = NULL;
				ModelData->texCoords = NULL;
				ModelData->vertices = NULL;

				ModelData->Material_Ambient = NULL;
				ModelData->Material_Diffuse = NULL;
				ModelData->Material_Specular = NULL;
				ModelData->Material_Emissive = NULL;
				ModelData->Material_Shininess = NULL;

				if(Model->Material != NULL)
				{
					//Do the calculations and save it...
					Model->Material->transperency = 1.0f - Model->Material->transperency;
					ModelData->Material_Diffuse = new GLfloat[4];
					ModelData->Material_Diffuse[0] = Model->Material->diffuseColor[0];
					ModelData->Material_Diffuse[1] = Model->Material->diffuseColor[1];
					ModelData->Material_Diffuse[2] = Model->Material->diffuseColor[2];
#ifdef DATADEBUG
					console << "Diffuse: { " << Model->Material->diffuseColor[0] << ", " << Model->Material->diffuseColor[1] << ", " << Model->Material->diffuseColor[2] << "}" << Console::nl;
#endif
					ModelData->Material_Diffuse[3] = Model->Material->transperency;
					ModelData->Material_Shininess = new GLfloat[1];
					ModelData->Material_Shininess[0] = Model->Material->shininess;
					ModelData->Material_Specular = new GLfloat[4];
					ModelData->Material_Specular[0] = Model->Material->specularColor[0];
					ModelData->Material_Specular[1] = Model->Material->specularColor[1];
					ModelData->Material_Specular[2] = Model->Material->specularColor[2];
					ModelData->Material_Specular[3] = Model->Material->transperency;
					ModelData->Material_Ambient = new GLfloat[4];
					ModelData->Material_Ambient[0] = Model->Material->diffuseColor[0] * Model->Material->ambientIntensity;
					ModelData->Material_Ambient[1] = Model->Material->diffuseColor[1] * Model->Material->ambientIntensity;
					ModelData->Material_Ambient[2] = Model->Material->diffuseColor[2] * Model->Material->ambientIntensity;
#ifdef DATADEBUG
					console << "Ambient: {" << ModelData->Material_Ambient[0] << ", " << ModelData->Material_Ambient[1] << ", " << ModelData->Material_Ambient[2] << "}" << Console::nl;
#endif
					ModelData->Material_Ambient[3] = Model->Material->transperency;
					ModelData->Material_Emissive = new GLfloat[4];
					ModelData->Material_Emissive[0] = Model->Material->emissiveColor[0];
					ModelData->Material_Emissive[1] = Model->Material->emissiveColor[1];
					ModelData->Material_Emissive[2] = Model->Material->emissiveColor[2];
					ModelData->Material_Emissive[3] = Model->Material->transperency;
				}
				ModelData->tri_count = triangles.size() / 3;
				ModelData->tris = new int[ModelData->tri_count * 3];

				ModelData->pointCount = Model->Coordinates->points.size() / 3;

				//Copy the triangles to the triangle array.
				int c = 0;
				for(vector<int>::iterator faces = triangles.begin(); faces != triangles.end(); faces++, c++)
					ModelData->tris[c] = *faces;
				
				if(Model->TextureFaces)
				{
					ModelData->tex_tris = new int[ModelData->tri_count * 3];
					c = 0;
					//Copy the texture triangles to the texture triangle array
					for(vector<int>::iterator faces = tex_triangles.begin(); faces != tex_triangles.end(); faces++, c++)
						ModelData->tex_tris[c] = *faces;
				}

				ModelData->vertices = new float[Model->Coordinates->points.size()];
				
				//Copy all vertices to the model
				c = 0;
				for(vector<float>::iterator coords = Model->Coordinates->points.begin(); coords != Model->Coordinates->points.end(); coords++, c++)
					ModelData->vertices[c] = *coords / 8;
				
				if(Model->TextureCoordinates)
				{
					c = 0;
					//Copy all texture coords to the model
					ModelData->texCoords = new float[Model->TextureCoordinates->points.size()];
					for(vector<float>::iterator tex_coords = Model->TextureCoordinates->points.begin(); tex_coords != Model->TextureCoordinates->points.end(); tex_coords++, c++)
						ModelData->texCoords[c] = *tex_coords;
					ModelData->texPointCount = Model->TextureCoordinates->points.size() / 2;
				}
			
				//Apply Translatation
				//Apply Scale
				if(Model->TRS_Scale && Model->TRS_Translate)
				{
#ifdef DATADEBUG
					console << "Scale {" << Model->TRS_Scale->value[0] << ", " << Model->TRS_Scale->value[1] << ", " << Model->TRS_Scale->value[2] << "}" << Console::nl;
					console << "Translate {" << Model->TRS_Translate->value[0] << ", " << Model->TRS_Translate->value[1] << ", " << Model->TRS_Translate->value[2] << "}" << Console::nl;
#endif
					for(unsigned int i = 0; i < Model->Coordinates->points.size(); i += 3)
					{
						ModelData->vertices[i] *= Model->TRS_Scale->value[0];
						ModelData->vertices[i + 1] *= Model->TRS_Scale->value[1];
						ModelData->vertices[i + 2] *= Model->TRS_Scale->value[2];

						ModelData->vertices[i] += Model->TRS_Translate->value[0];
						ModelData->vertices[i + 1] += Model->TRS_Translate->value[1];
						ModelData->vertices[i + 2] += Model->TRS_Translate->value[2];
					}
				}
				
				float lowest_x = ModelData->vertices[0], lowest_y = ModelData->vertices[1], lowest_z = ModelData->vertices[2];
				float highest_x = lowest_x, highest_y = lowest_y, highest_z = lowest_z;

				for(unsigned int i = 0; i < Model->Coordinates->points.size(); i+=3)
				{
					if (ModelData->vertices[i] > highest_x)
						highest_x = ModelData->vertices[i];
					if (ModelData->vertices[i+1] > highest_y)
						highest_y = ModelData->vertices[i+1];
					if (ModelData->vertices[i+2] > highest_z)
						highest_z = ModelData->vertices[i+2];

					if (ModelData->vertices[i] < lowest_x)
						lowest_x = ModelData->vertices[i];
					if (ModelData->vertices[i+1] < lowest_y)
						lowest_y = ModelData->vertices[i+1];
					if (ModelData->vertices[i+2] < lowest_z)
						lowest_z = ModelData->vertices[i+2];
				}

				float scale;

				if ((highest_x - lowest_x) > (highest_z - lowest_z))
				{
					scale = 2 / (highest_x - lowest_x);
				}
				else
				{
					scale = 2 / (highest_z - lowest_z);
				}

#ifdef DATADEBUG
				console << "Lowest x: " << lowest_x << Console::nl;
				console << "Lowest y: " << lowest_y << Console::nl;
				console << "Lowest z: " << lowest_z << Console::nl;
				console << "Highest x: " << highest_x << Console::nl;
				console << "Highest y: " << highest_y << Console::nl;
				console << "Highest z: " << highest_z << Console::nl;
				console << "Builtin Scaling: " << scale << Console::nl;
#endif
				for(unsigned int i = 0; i < Model->Coordinates->points.size(); i+=3)
				{
					ModelData->vertices[i] -= (highest_x + lowest_x) / 2;
//					ModelData->vertices[i+1] += lowest_y / scale - lowest_y;
					ModelData->vertices[i+1] += (-(1 / scale) - lowest_y);
					ModelData->vertices[i+2] -= (highest_z + lowest_z) / 2;
				}
				
				for(unsigned int i = 0; i < Model->Coordinates->points.size(); i++)
					ModelData->vertices[i] *= scale;
				
				lowest_x = ModelData->vertices[0]; lowest_y = ModelData->vertices[1]; lowest_z = ModelData->vertices[2];
				highest_x = lowest_x; highest_y = lowest_y; highest_z = lowest_z;

				for(unsigned int i = 0; i < Model->Coordinates->points.size(); i+=3)
				{
					if (ModelData->vertices[i] > highest_x)
						highest_x = ModelData->vertices[i];
					if (ModelData->vertices[i+1] > highest_y)
						highest_y = ModelData->vertices[i+1];
					if (ModelData->vertices[i+2] > highest_z)
						highest_z = ModelData->vertices[i+2];

					if (ModelData->vertices[i] < lowest_x)
						lowest_x = ModelData->vertices[i];
					if (ModelData->vertices[i+1] < lowest_y)
						lowest_y = ModelData->vertices[i+1];
					if (ModelData->vertices[i+2] < lowest_z)
						lowest_z = ModelData->vertices[i+2];
				}
				
#ifdef DATADEBUG
				console << "Lowest x: " << lowest_x << Console::nl;
				console << "Lowest y: " << lowest_y << Console::nl;
				console << "Lowest z: " << lowest_z << Console::nl;
				console << "Highest x: " << highest_x << Console::nl;
				console << "Highest y: " << highest_y << Console::nl;
				console << "Highest z: " << highest_z << Console::nl;
#endif

				ModelData->normals = VertexNormals;

				if(Model->TextureURL.size())
				{
					size_t location = mFile.find_last_of("/", mFile.size() - 1);
					if(location == string::npos)
					{
						console << mFile << Console::nl;
						return MODEL_ERROR_PARSE;
					}
					string path = mFile.substr(0, location) + "/textures/" + Model->TextureURL;
					ModelData->texture = LoadTexture(path);
				}
				else
				{
					ModelData->texture = 0;
				}

				ModelsIndex.push_back(ModelData);
				

				//Check if a model by the same name exists.
				if(ModelsData.find(Model->Name->name) != ModelsData.end())
					return MODEL_ERROR_NAME_CONFLICT;
				else
					ModelsData[Model->Name->name] = ModelData;
#ifdef DATADEBUG
				console << "Model '" << Model->Name->name << "' Added." << Console::nl;
#endif

				//Dealloc model in memory...
				DeleteInnerModel(Model);
			}
			else if((*handleIter)->type == tUnit)
			{
				InnerUnit *pUnit = (*handleIter)->data.typeUnit;
				Game::Dimension::UnitType *pUnitType = new Game::Dimension::UnitType;
				SET_UNITTYPE_VALUE(maxHealth)
				SET_UNITTYPE_VALUE(maxPower)
				SET_UNITTYPE_VALUE(minAttack)
				SET_UNITTYPE_VALUE(maxAttack)
				SET_UNITTYPE_VALUE(heightOnMap)
				SET_UNITTYPE_VALUE(widthOnMap)
				SET_UNITTYPE_VALUE(isMobile)
				SET_UNITTYPE_VALUE(hasAI)
				SET_UNITTYPE_VALUE(hurtByLight)
				SET_UNITTYPE_VALUE(canAttack)
				SET_UNITTYPE_VALUE(canAttackWhileMoving)
				SET_UNITTYPE_VALUE(attackMinRange)
				SET_UNITTYPE_VALUE(attackMaxRange)
				SET_UNITTYPE_VALUE(attackAccuracy)
				SET_UNITTYPE_VALUE(sightRange)
				SET_UNITTYPE_VALUE(movementSpeed)
				SET_UNITTYPE_VALUE(attackSpeed)
				SET_UNITTYPE_VALUE(size)
				SET_UNITTYPE_VALUE(height)
				SET_UNITTYPE_VALUE(buildTime)
				SET_UNITTYPE_VALUE(researchTime)
				SET_UNITTYPE_VALUE(powerIncrement)
				SET_UNITTYPE_VALUE(powerUsage)
				SET_UNITTYPE_VALUE(lightPowerUsage)
				SET_UNITTYPE_VALUE(attackPowerUsage)
				SET_UNITTYPE_VALUE(buildPowerUsage)
				SET_UNITTYPE_VALUE(movePowerUsage)
				SET_UNITTYPE_VALUE(buildCost)
				SET_UNITTYPE_VALUE(researchCost)
				SET_UNITTYPE_VALUE(lightRange)
				
				for (int i = 0; i < AI::ACTION_NUM; i++)
				{
					pUnitType->animations[i] = NULL;
				}

				pUnitType->isResearched = new bool[Dimension::pWorld->vPlayers.size()];
				pUnitType->isBeingResearchedBy = new Dimension::Unit*[Dimension::pWorld->vPlayers.size()];
				for (unsigned int i = 0; i < Dimension::pWorld->vPlayers.size(); i++)
				{
					pUnitType->isResearched[i] = pUnit->isResearched;
					pUnitType->isBeingResearchedBy[i] = NULL;
				}
				
				switch(pUnit->movementType)
				{
					SET_UNITTYPE_ENUM(MovementTypeVehicle, movementType, MOVEMENT_MEDIUMVEHICLE)
					SET_UNITTYPE_ENUM(MovementTypeSea, movementType, MOVEMENT_SEA)
					SET_UNITTYPE_ENUM(MovementTypeBuilding, movementType, MOVEMENT_BUILDING)
					SET_UNITTYPE_ENUM(MovementTypeTank, movementType, MOVEMENT_LARGEVEHICLE)
					SET_UNITTYPE_ENUM(MovementTypeAirborne, movementType, MOVEMENT_AIRBORNE)
					SET_UNITTYPE_ENUM(MovementTypeHuman, movementType, MOVEMENT_HUMAN)
					default:
						break;
				}

				switch(pUnit->powerType)
				{
					SET_UNITTYPE_ENUM(PowerDayLight, powerType, POWERTYPE_DAYLIGHT)
					SET_UNITTYPE_ENUM(PowerTwentyFourSeven, powerType, POWERTYPE_TWENTYFOURSEVEN)
					default:
						break;
				}

				pUnitType->name = (char*)pUnit->Name->c_str();				
#ifdef DATADEBUG
				console << "Name == " << *pUnit->Name << Console::nl;
				if(pUnit->Model != NULL)
					console << "ModelName == " << *pUnit->Model << Console::nl;
#endif
				if(pUnit->Symbol != NULL)
				{
#ifdef DATADEBUG				
					console << "Symbol == " << *pUnit->Symbol << Console::nl;
#endif
					//Load the image and make texture
					pUnitType->Symbol = LoadTexture(*(pUnit->Symbol));
				}
				else
				{
					pUnitType->Symbol = 0;
				}

				pUnitType->projectileType = NULL;
				pUnitType->model = NULL;
				pUnitType->name = (char*) pUnit->Name->c_str();
				
				//Check if a unittype by the same name exists.
				if(UnitsData.find(*(pUnit->Name)) != UnitsData.end())
					return MODEL_ERROR_NAME_CONFLICT;
				else
					UnitsData[*(pUnit->Name)] = pUnitType;
#ifdef DATADEBUG
				console << "Unit '" << *(pUnit->Name) << "' Added." << Console::nl;
#endif
				UnitsIndex.push_back(pUnitType);

				unitTypeMap[*pUnit->Name] = pUnitType;

				if(pUnit->Model != NULL)
				{
					pUnitType->model = LoadModel(pUnit->Model->c_str());
					if(pUnitType->model == NULL)
					{
#ifdef PRINTOUT					
						console << "Model '" << *(pUnit->Model) << "' in " << pUnitType->name << " couldn't be found!" << Console::nl;
#endif
					}
					else
					{
						for (int i = 0; i < AI::ACTION_NUM; i++)
						{
							pUnitType->animations[i] = CreateAnimation(CreateTransAnim(CreateMorphAnim(1.0, 1, pUnit->Model->c_str(), 0.0), NULL, 0, 1.0, 1, CreateTransformData(Utilities::Vector3D(0.0, 0.0, 0.0), Utilities::Vector3D(0.0, 0.0, 0.0), Utilities::Vector3D(0.0, 0.0, 0.0), Utilities::Vector3D(1.0, 1.0, 1.0)), 0.0));
						}
					}
				}
				
				if(pUnit->projectileType != NULL)
				{
					if(pUnit->projectileType->model != NULL)
					{
						ProjectileType *projectile = new ProjectileType;
						projectile->model = LoadModel(pUnit->projectileType->model->c_str());
						if(projectile->model == NULL)
						{
#ifdef PRINTOUT
							console << "Projectile Model '" << *(pUnit->Model) << "' in " << pUnitType->name << " couldn't be found!" << Console::nl;
#endif
						}
						if(pUnit->projectileType->startPos != NULL)
						{
							Utilities::Vector3D v3(pUnit->projectileType->startPos[0], pUnit->projectileType->startPos[1],pUnit->projectileType->startPos[2]);
							projectile->startPos = v3;
						}
						projectile->areaOfEffect = pUnit->projectileType->areaOfEffect;
						projectile->speed = pUnit->projectileType->speed;
						projectile->size = pUnit->projectileType->size;
						pUnitType->projectileType = projectile;						
					}
					else
					{
						return MODEL_ERROR_UNEXPECTED_ERROR;
					}
				}

				if(pUnit->canBuild != NULL)
				{
					int count = 0;
					for(vector<string>::iterator Iterator = pUnit->canBuild->begin(); Iterator != pUnit->canBuild->end(); Iterator++)
					{
						Game::Dimension::UnitType *typ = LoadUnitType(Iterator->c_str());
						if(typ != NULL)
						{
							pUnitType->canBuild.push_back(typ);
							count++;
						}
					}
				}

				if(pUnit->canResearch != NULL)
				{
					int count = 0;
					for(vector<string>::iterator Iterator = pUnit->canResearch->begin(); Iterator != pUnit->canResearch->end(); Iterator++)
					{
						Game::Dimension::UnitType *typ = LoadUnitType(Iterator->c_str());
						if(typ != NULL)
						{
							pUnitType->canResearch.push_back(typ);
							count++;
						}
					}
				}

				for (int i = 0; i < Audio::SFX_ACT_COUNT; i++)
					pUnitType->actionSounds[i] = NULL;

				if(pUnit->sound != NULL)
				{
					//
					// Format
					// strength=%d&sound=%s
					//
					// action keys defined in actionStrings.rtf
					//
					const char* actionIndexes[8] = {"fire", "death", "move", "moveDone", "idle", "focus"};
					for (int i = 0; i < Audio::SFX_ACT_COUNT; i++)
						pUnitType->actionSounds[i] = NULL;

					for (vector<string>::iterator Iterator = pUnit->sound->begin(); 
						 pUnit->sound->end() != Iterator; 
						 Iterator++)
					{
						float strength = 0.0f;
						char  sound[255];
						char  action[16];
						int index = -1;
						
						char* ptr = (char*)Iterator->c_str();
						char buffer[255];
						int count = 0;
						
						for (int i = 0; i < 255; i++)
							buffer[i] = 0;
						
						while (true)
						{
							if (count >= 255)
								return MODEL_ERROR_INVALID_SOUND_FORMAT;
							
							if (*ptr == ',' || *ptr == 0)
							{						
								char* argptr = buffer;
								char token[16];
								char value[255];
								count = 0;
								
								for (int i = 0; i < 16; i++)
									token[i] = 0;
								for (int i = 0; i < 255; i++)
									value[i] = 0;
								
								do
								{
									if (count >= 64)
										return MODEL_ERROR_INVALID_SOUND_FORMAT;
										
									token[count++] = *argptr;
								} while (*(argptr++ + 1) != '=');
								
								argptr++;
								
								count = 0;
								
								do
								{
									if (count >= 255)
										return MODEL_ERROR_INVALID_SOUND_FORMAT;
										
									value[count++] = *argptr;
								} while (*(argptr++ + 1) != 0);
								
								if (strcmp("action", token) == 0)
								{
									if (count > 16)
										return MODEL_ERROR_INVALID_FORMAT;
										
									strcpy(action, value);
								}
								else if (strcmp("strength", token) == 0)
								{
									strength = (float) atof(value);
									
									if (strength > 255.0f)
										return MODEL_ERROR_INVALID_FORMAT;
								}
								else if (strcmp("sound", token) == 0)
								{
									strcpy(sound, value);
								}
								else if (strcmp("index", token) == 0)
								{
									index = atoi(value);
									
									if (index > 1 >> 16)
										return MODEL_ERROR_INVALID_FORMAT;
								}
								
								if (*ptr == 0)
									break;
									
								for (int i = 0; i < 255; i++)
									buffer[i] = 0;
								
								count = 0;
								
								ptr++;
								continue;
							}
							
							buffer[count] = *ptr;
							
							ptr++;
							count++;
						}

						if (strength > 0.0f && index > -1)
						{
							Audio::Sound* soundFx = Audio::GetSound(sound, index);
							if (soundFx != NULL)
							{
								Audio::AudioFXInfo* info = new Audio::AudioFXInfo;
								info->channel = 0;
								info->pSound  = soundFx;
								info->strength = strength;

								int i = 0;
								for ( ; i < Audio::SFX_ACT_COUNT; i++)
								{
									if (pUnitType->actionSounds[i] != NULL)
										continue;

									if (strcmp(actionIndexes[i], action) == 0)
									{
										pUnitType->actionSounds[i] = info;
										break;
									}
								}

								if (i == Audio::SFX_ACT_COUNT)
								{
									delete info;
									info = NULL;
								}
							}
						}
					}
					delete pUnit->sound;
				}

				DeleteInnerUnit(pUnit);
			}
			delete *handleIter;
		}
		return SUCCESS;
	}

	//Parse a file.
	int ModelParser::Parse(string mFile)
	{
		map<string, bool>::iterator iter = LoadedFiles.find(mFile);
		if(iter != LoadedFiles.end())
			return SUCCESS;
		else
			LoadedFiles[mFile] = true;

		int errCode = 0;
		
		vector<string> tokens;
		vector<DataGroup*> blocks;
		vector<innerHandling*> ObjectBlocks;
		
		//Filedata to string token conversion.
		errCode = ProcessFile(mFile, tokens);
		if(errCode != SUCCESS)
		{
#ifdef DATADEBUG
			console << Console::err << "File Error" << Console::nl;
#endif
			return errCode;
		}

		//Token & Data interpreting.
		errCode = ProcessTokens(blocks, tokens);
		if(errCode != SUCCESS)
		{
#ifdef DATADEBUG
			console << Console::err << "Token Error" << Console::nl;
#endif
			return errCode;
		}
		
		//Clear all tokens from memory, they are not needed anymore.
		tokens.clear();

		errCode = ProcessGroups(blocks, ObjectBlocks);
		if(errCode != SUCCESS)
		{
#ifdef DATADEBUG
			console << Console::err << "Group Error" << Console::nl;
#endif
			return errCode;
		}
		
		//Clears groups, not needed anymore.
		blocks.clear();

		errCode = PrepareForGameCore(ObjectBlocks, mFile);
		if(errCode != SUCCESS)
		{
#ifdef DATADEBUG
			console << Console::err << "GameCore Error" << Console::nl;
#endif
			return errCode;
		}
		
		return SUCCESS;
	}	

	Game::Dimension::Model* ModelParser::GetModel(int id)
	{
		return ModelsIndex[id];
	}

	Game::Dimension::Model* ModelParser::GetModel(string id)
	{
		map<string, Game::Dimension::Model*>::iterator ModelIter = ModelsData.find(id); 
		if(ModelIter != ModelsData.end())
		{
			return (*ModelIter).second;
		}
		else
			return NULL;
	}
	Game::Dimension::UnitType* ModelParser::GetUnit(int id)
	{
		return UnitsIndex[id];
	}

	Game::Dimension::UnitType* ModelParser::GetUnit(string id)
	{
		map<string, Game::Dimension::UnitType*>::iterator UnitIter = UnitsData.find(id); 
		if(UnitIter != UnitsData.end())
		{
			unitTypeMap[id] = (*UnitIter).second;
			return (*UnitIter).second;
		}
		else
			return NULL;
	}

	vector<Game::Dimension::UnitType*> ModelParser::GetBuildUnits(void)
	{
		vector<Game::Dimension::UnitType*> units;
		for(vector<Game::Dimension::UnitType*>::iterator iter = pWorld->vUnitTypes.begin(); iter != pWorld->vUnitTypes.end(); iter++)
		{
			Game::Dimension::UnitType *unit = (*iter);
			if(unit->canBuild.size() != 0)
			{
				units.push_back(unit);
			}
		}
		return units;
	}

	int ModelParser::GetModelCount()
	{
		return ModelsIndex.size();
	}
	int ModelParser::GetUnitCount()
	{
		return UnitsIndex.size();
	}
}

