/*
 * Nightfall - Real-time strategy game
 *
 * Copyright (c) 2008 Marcus Klang, Alexander Toresson and Leonard Wickmark
 * 
 * This file is part of Nightfall.
 * 
 * Nightfall is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nightfall is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nightfall.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#ifdef DEBUG_DEP
#warning "environment.h"
#endif

#include "environment-pre.h"

#define DEFAULT_ENVIRONMENT_FPS 20
#define ENVIRONMENT_ENUM_DEACTIVATE -1.0f

namespace Game
{
	namespace Dimension
	{
		namespace Environment
		{
/*
			class Vector4D
			{
			public:
				GLfloat m1;
				GLfloat m2;
				GLfloat m3;
				GLfloat m4;

				GLfloat* arr;

				Vector4D(GLfloat m1 = 0.0f, GLfloat m2 = 0.0f, GLfloat m3 = 0.0f, GLfloat m4 = 0.0f);
				~Vector4D(void);
				void BuildFromGLArray(GLfloat*);
				void BuildFromVAList(...);
				GLfloat* ToGLArray(void);
				const char* ToString(void);
				void DeleteGLArray(void);
			};
*/

			struct SkyBox
			{
				GLuint texture;
				//GLUquadricObj *sphere;
				float r; // << radius
			};

			struct EnvironmentalCondition
			{
				float    hourBegin;
				float    hourEnd;
				GLfloat  fogColor[4];
				GLfloat  fogIntensity;
				GLfloat  fogBegin;
				GLfloat  fogEnd;
				GLfloat  ambient[4];
				GLfloat  diffuse[4];
				GLfloat  sunPos[4];
/*				GLfloat  terrainMaterialAmbientDiffuse[2][2][4];
				GLfloat  waterMaterialAmbientDiffuse[2][2][4];*/
				int      skybox;
				bool     isNight;
				bool     isDay;
				bool     isDawn;
				bool     isDusk;
				std::string   musicListTag;

				EnvironmentalCondition* _next; // Private! Do not set!
				EnvironmentalCondition* _last; // Private! Do not set!

				void (*fptrOnBegin) (const EnvironmentalCondition*);
				void (*fptrOnEnd) (const EnvironmentalCondition*);

				EnvironmentalCondition(void)
				{
					this->fogIntensity = 0.0f;
					this->fogBegin = 0.0f;
					this->fogEnd = 0.0f;
					this->hourBegin = 0.0f;
					this->hourEnd = 0.0f;
					this->isNight = false;
					this->isDay = false;
					this->isDawn = true;
					this->isDusk = true;
					this->skybox = 0;
					this->_next = this;
					this->_last = this;

					for (int i = 0; i < 4; i++)
					{
						this->fogColor[i]  = 0;
						this->ambient[i]   = 0;
						this->diffuse[i]   = 0;
						this->sunPos[i]    = 0;
					}
					
/*					for (int i = 0; i < 2; i++)
					{
						for (int j = 0; j < 2; j++)
						{
							for (int k = 0; k < 4; k++)
							{
								this->terrainMaterialAmbientDiffuse[i][j][k] = 0;
								this->waterMaterialAmbientDiffuse[i][j][k] = 0;
							}
						}
					}

					// Use presets
					this->waterMaterialAmbientDiffuse[0][0][0] = -1.0f; 
					this->terrainMaterialAmbientDiffuse[0][0][0] = -1.0f;*/

					this->fptrOnBegin = NULL;
					this->fptrOnEnd = NULL;
				}
			};

			class FourthDimension
			{
				private:
					static FourthDimension* m_pInstance;

					EnvironmentalCondition* m_envFirst;
					EnvironmentalCondition* m_envLast;

					EnvironmentalCondition* m_curEnvCond;
					EnvironmentalCondition* m_nextEnvCond;
					int m_frames;

					int m_conditionsCount;
					bool m_prepared;

					SkyBox** m_skyboxes;
					int m_skyboxesCount;
					int m_skyboxCurIndex;
					GLfloat m_skyboxOpacity;
					float m_skyboxRotation;
					std::map<std::string, int> m_skyboxesGLContainer;
					
					float m_hourLength;
					int m_dayLength;
					float m_currentHour;
					float m_progress;
					
					float *sphere_coords;
					float *sphere_texcoords;
					int *sphere_faces;
					int sphere_face_count;
/*
					GLfloat m_ambientDt[4];
					GLfloat m_diffuseDt[4];
					GLfloat m_sunPosDt[4];
					GLfloat m_fogColorDt[4];
					GLfloat m_fogIntensityDt;
					GLfloat m_fogBeginDt;
					GLfloat m_fogEndDt;
					GLfloat m_skyboxOpacityDt;
*/

					GLfloat m_curAmbient[4];
					GLfloat m_curDiffuse[4];
					GLfloat m_curSunPos[4];
					GLfloat m_curFogColor[4];
		
					GLfloat m_curFogIntensity;
					GLfloat m_curFogBegin;
					GLfloat m_curFogEnd;
					GLfloat m_curAlpha;

					bool CheckEnvironmentalConditions(void);
					void CalculateChange(void);
					bool ApplyEnvironmentalConditions(EnvironmentalCondition* const = NULL);

					void RenderSkyBox(const SkyBox *sky1, const SkyBox *sky2, float value, float rotation);
					void RenderSkyBox(const SkyBox* sky, float rotation);

					void PrepareFog(void) const;
					void SetFog(bool isEnabled);

					float CalculateLength(EnvironmentalCondition* cond, float from  = 0, float to = 0);

				public:
					static FourthDimension* Instance(void);
					static void Destroy(void);
					static FourthDimension* ReallocInstance(void);

					FourthDimension(void) : 
								m_envFirst(NULL), m_envLast(NULL), m_curEnvCond(NULL), m_nextEnvCond(NULL),
								m_frames(0), m_conditionsCount(0), m_prepared(false),
								m_skyboxes(NULL), m_skyboxesCount(0), m_skyboxCurIndex(0), m_skyboxOpacity(0),
								m_skyboxRotation(0),m_hourLength(0), m_dayLength(24), m_currentHour(0.0),
								sphere_coords(NULL), sphere_texcoords(NULL), sphere_faces(NULL),
								m_curFogIntensity(0), m_curAlpha(0.0f)
					{
						PrepareFog();
					};
					~FourthDimension(void);

					void SetHourLength(const float);
					float GetHourLength(void) const;
					void SetDayLength(const int);
					int GetDayLength(void) const;
					void SetCurrentHour(const float);
					float GetCurrentHour(void) const;

					void SetDefaultSun(void) const;
					void RotateWorld(float lastPass);

					void ApplyLight(void);
					void AllocSkyboxContainer(const int);
					bool LoadSkyboxes(std::string config_file = "skyboxes.txt");
					int GetSkybox(const char* tag);
					int LoadSkyBox(std::string path);
					void InitSkyBox(int detail, int hdetail);
					void RenderSkyBox();

					void AddCondition(EnvironmentalCondition* const);
					bool ValidateConditions(void);
			};
			
			class EnvironmentNode : public Scene::Graph::Node
			{
				private:
					EnvironmentNode();
				protected:
					virtual void Render();
				public:
					static EnvironmentNode instance;
			};

			class SkyboxNode : public Scene::Graph::Node
			{
				private:
					SkyboxNode();
				protected:
					virtual void Render();
				public:
					static SkyboxNode instance;
			};

		}
	}
}

#ifdef DEBUG_DEP
#warning "environment.h-end"
#endif

#endif

