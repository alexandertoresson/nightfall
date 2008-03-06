#ifndef __VRMLPARSER_H__
#define __VRMLPARSER_H__

#ifdef DEBUG_DEP
#warning "model.h"
#endif

#include "sdlheader.h"
#include "unit.h"
#include <string>
#include <map>
#include <vector>

namespace Utilities
{
	class ModelParser
	{
		private:
			std::string mFile;
			std::vector<Game::Dimension::Model*> ModelsIndex;
			std::map<std::string, Game::Dimension::Model*> ModelsData;
			std::vector<Game::Dimension::UnitType*> UnitsIndex;
			std::map<std::string, Game::Dimension::UnitType*> UnitsData;
			std::map<std::string, bool> LoadedFiles;
			bool ValidateHeader(std::string);	
		
			enum TokenType
			{
				//Model
				Definition, //DEF
				Appearance, //Appearance
				Shape, //Shape,
				Geometry, //IndexedFaceSet
				Material, //Material
				ImageTexture, //ImageTexture
				URL, //url
				TextureCoords, //TextureCoordinate
				TextureCoordsIndex, //texCoordIndex
				Coords, //Coordinate
				CoordsIndex, //Index
				Material_Tokens, //en jäkla massa....
				Scale,
				Rotate,
				Translate,
				TRS_Tokens, //Translate, Scale, Rotate tokens
				Unknown,
				UnknownNode, //Someting [{]
				UnknownArray,
				Newline,
				Children,
				Transform,
				Group,
				Point,
				Texture,
				Material_Ambient,
				Material_Diffuse,
				Material_Specular,
				Material_Transperency,
				Material_Shininess,
				Material_Emissive,
				//Unit
				//Parameters
				MaxHealth,
				MinAttack,
				MaxAttack,
				MaxPower,
				AttackMinRange,
				AttackMaxRange,
				AttackAccuracy,
				SightRange,
				HurtByLight,
				CanAttack,
				CanAttackWhileMoving,
				IsMobile,
				MovementSpeed,
				AttackSpeed,
				CanBuild,
				CanResearch,
				HasAI,
				IsResearched,
				Size,
				BuildTime,
				ResearchTime,
				HeightOnMap,
				WidthOnMap,
				Height,
				UnitDef,
				UnitSymbol,
				ModelName,
				ProjectileTypeStruct,
				AreaOfEffect,
				Speed,
				StartPos,
				PowerIncrement,
				PowerType,
				PowerDayLight,
				PowerTwentyFourSeven,
				LightRange,
				BuildCost,
				ResearchCost,
				PowerUsage,
				LightPowerUsage,
				AttackPowerUsage,
				BuildPowerUsage,
				MovePowerUsage,

				MovementType,

				MovementTypeHuman,
				MovementTypeSea,
				MovementTypeVehicle,
				MovementTypeTank,
				MovementTypeAirborne,
				MovementTypeBuilding,
				Sound,
				// Animation
				Animation,
				//Animation Tokens
				Attack,
				Movement,
				Build,
				Collect,
				//Sub Category Tokens
				Length,
				Models,
				Time
				
			};	

			std::map<std::string, TokenType> TokenEnums;

			
			enum DefinitionType
			{
				dTransform = 0,
				dUnit = 1,
				dMaterial = 2,
				dGroup = 3,
				dViewport = 4,
				dPointlight

			};
			
			enum DataType
			{
				String,
				Float,
				Integer,
				Boolean,
				UnknownDataType,
				Coordinate2D,
				Coordinate3D
			};
			
			TokenType GetTokenType(std::vector<std::string>::iterator&, std::vector<std::string>&);
			DataType GetDataType(std::string&);

			struct VRMLDefinition
			{
				std::string name;
				DefinitionType type;
			};
			
			struct VRMLParameter
			{
				std::string name;
				DataType infoType;
				union { //Max 32 bit, if int, float and pointer is 32 bit.
					int intValue;
					float floatValue;
					std::string* stringArray;
					std::vector<float>* floatArray;
				} data;
			};
			
			struct VRMLTextureURL
			{
				std::string path;
			};

			struct VRMLClause
			{
				std::string name;
				bool finish;
			};
			enum FCType
			{
				FC_Texture,
				FC_Solid
			};

			struct VRMLCoordinates
			{
				FCType type;
				std::vector<float> points;
			};
		
			struct VRMLTRS
			{
				float value[3];
			};

			struct VRMLFaces
			{
				FCType type;
				std::vector<int> points;
				int faceCount;
			};

			struct VRMLMaterial
			{
				float diffuseColor[3];
				float ambientIntensity;
				float specularColor[3];
				float emissiveColor[3];
				float shininess;
				float transperency;
			};

			struct InnerModel
			{
				std::vector<VRMLParameter*>* Parameters;
				VRMLCoordinates* Coordinates;
				VRMLCoordinates* TextureCoordinates;
				VRMLFaces* CoordinateFaces;
				VRMLFaces* TextureFaces;
				VRMLDefinition* Name;
				VRMLMaterial* Material;
				VRMLTRS* TRS_Translate;
				VRMLTRS* TRS_Scale;
				std::string TextureURL;
			};		
			
			struct UnitProjectileType;

			struct InnerUnit
			{
				std::string*     Name;
				int         maxHealth;
				int         maxPower;
				int         minAttack;     // in hitpoints
				int         maxAttack;
				float 	    lightRange;
				float 	    powerUsage;
				float 	    lightPowerUsage;
				float 	    attackPowerUsage;
				float 	    buildPowerUsage;
				float 	    movePowerUsage;
				int 	    buildCost;
				int         researchCost;
				float 	    powerIncrement;
				TokenType   powerType;
				int         heightOnMap;   // this width and height only affect how much space the unit takes on the map.
				int         widthOnMap;

				bool        isMobile;      // whether the unit is moveable
				bool        hasAI;         // whether the unit has an AI
				bool        isResearched;
				bool        hurtByLight;
				bool        canAttack;
				bool        canAttackWhileMoving;

				float       attackMinRange;// the minimum range of the unit's attack
				float       attackMaxRange;// the maximum range of the unit's attack
				float       attackAccuracy;
				float       sightRange;
				float       movementSpeed; // in squares per second
				float       attackSpeed;   // in times per second
				float       size;          // size of unit -- how to scale it
				float       height;
				int       buildTime;     // seconds to build
				int       researchTime;     // seconds to research

				UnitProjectileType *projectileType;
				
				std::vector<std::string>*		canBuild;
				std::vector<std::string>*		canResearch;
				std::vector<std::string>*		sound;
				std::string*     Model;
				std::string*	    Symbol;

				TokenType movementType;	
			};

			struct VRMLData
			{
				TokenType type;
				union {
					VRMLDefinition* Definition;
					VRMLCoordinates* Coordinates;
					VRMLFaces* Faces;
					VRMLParameter* Parameter;
					VRMLMaterial* Material;
					VRMLTextureURL* URL;
					VRMLTRS* TRS;
					bool finish; //Clause begin or finish
				} param;
				VRMLData *children;
				VRMLData *nxt;
			};

			enum DataGroupType
			{
				Model,
				Unit
			};
			
			//Most data is single value paramaters.
			struct UnitData
			{
				TokenType type;
				union
				{
					int integer;
					bool boolean;
					float floating;
					TokenType enums;
					std::string* str;
					std::vector<float>* floatArray;
					std::vector<std::string>* stringArray;
					UnitProjectileType *projectileType;
				} param;
				UnitData *children; //For example Animation grouping.
				UnitData *nxt;
			};
			
			struct UnitProjectileType
			{
				std::string* model;
				float size;
				float areaOfEffect;
				float* startPos;
				float speed;
			};

			struct UnitAnimation;

			struct DataGroup
			{
				DataGroupType type;
				union
				{
					VRMLData *VRML_Begin;
					UnitData *Unit_Begin;
				} param;
			};

			enum HandlingType
			{
				tModel,
				tUnit
			};

			struct innerHandling
			{
				HandlingType type;
				union {
					InnerModel* typeModel;
					InnerUnit* typeUnit;
				} data;
			};
			
			enum SearchTypes
			{
				Number,
				SpecifiedChar
			};

			bool CheckAhead(std::vector<std::string>::iterator&, std::vector<std::string>&,int);
			bool SearchAhead(std::vector<std::string>::iterator& CurrentPosition, std::vector<std::string>::iterator EndPosition,SearchTypes Find,char specifiedchar);
			void CreateVRMLData(VRMLData*&);
			void CreateInnerModel(InnerModel*&);

			void DeleteDataGroup(DataGroup*&);

			void DeleteVRMLData(VRMLData*&);
			void DeleteOnlyAndContinue(VRMLData*&);
			void DeleteAndContinue(VRMLData*&);
			void DeleteAll(VRMLData*&);
			void DeleteInnerModel(InnerModel*&);
			
			void CreateInnerUnit(InnerUnit*&);
			void CreateUnitData(UnitData*&);
			void DeleteAll(UnitData*&);
			void DeleteOnlyAndContinue(UnitData*&);
			void DeleteAndContinue(UnitData*&);
			void DeleteUnitData(UnitData*&);
			void DeleteInnerUnit(InnerUnit*&);
			
			typedef union
			{
				int integer;
				float floating;
				std::string* str;
				float* floatArray;
				bool boolean;
			} DataParameter;
			
			bool ReadParameter(DataType dTyp, DataParameter&, std::vector<std::string>::iterator& Position, std::vector<std::string>& tokens);
			bool ReadUnitParameter(TokenType tTyp, DataType dTyp, UnitData*& current, std::vector<std::string>::iterator& Postion, std::vector<std::string>& tokens);
			bool ReadUnitArray(TokenType tTyp, DataType dTyp, UnitData*& Current, std::vector<std::string>::iterator& Position, std::vector<std::string>& tokens);

			// Validates Number
			bool IsCoordinate(std::string&);
			bool GetNextType(std::vector<std::string>::iterator& position, std::vector<std::string>& tokens, ModelParser::DataType Type);

			void InitEnum();

			int ProcessFile(std::string mFile, std::vector<std::string>& tokens);
			int ProcessTokens(std::vector<DataGroup*>& blocks, std::vector<std::string>& tokens);
			int ProcessGroups(std::vector<DataGroup*>& blocks, std::vector<innerHandling*>& ObjectBlocks);
			int PrepareForGameCore(std::vector<innerHandling*>& ObjectBlocks, std::string& mFile);
		public:				
			ModelParser(std::string);
			ModelParser();
			~ModelParser();
		
			int Parse(void);
			int Parse(std::string);

			//void NormalizeModel(Game::Dimension::Model*);		
			Game::Dimension::Model* GetModel(std::string);
			Game::Dimension::Model* GetModel(int);
			Game::Dimension::UnitType* GetUnit(std::string);
			Game::Dimension::UnitType* GetUnit(int);
			int GetModelCount();
			int GetUnitCount();
	};
}

#ifdef DEBUG_DEP
#warning "model.h-end"
#endif

#endif
