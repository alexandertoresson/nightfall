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
#ifndef EFFECT_H
#define EFFECT_H

#ifdef DEBUG_DEP
#warning "effect.h"
#endif

#include "effect-pre.h"
#include "scenegraph.h"

#include "sdlheader.h"
#include "randomgenerator.h"
#include "utilities-pre.h"
#include "vector3d.h"
#include "unit-pre.h"


namespace Game
{
	namespace FX
	{
		extern ParticleSystemHandler *pParticleSystems;

		class ParticleSystemHandler
		{
			private:
				ParticleSystem **particleSystems;
				EffectType *particleEffect;
				ParticleStats *particleStats;

				MTRand *randomgenerator;
				int start;

				int systemCount;
				int particleDetail;

				GLuint blurpic;
			protected:
				struct ExplosionInit
				{
					GLuint center;
					GLuint blob;
					GLuint blob2;
					GLuint splinter;
				};

				ExplosionInit ExplosionPics;

			public:
				ParticleSystemHandler(int systemCount, int particleDetail);
				void InitEffect(float x, float y, float z, float size, EffectType type);
				//void InitEffect(float x, float y, float z, EffecType type, GLfloat[3] normal);
				void Iterate(float time);
				void Render();
				~ParticleSystemHandler();

			friend class Explosion;
		};

		class ParticleSystem
		{
			protected:
				struct Emitter
				{
					float x; //Emitter location
					float y;
					float z;

					float t; //Time to emit
					float d; //Duration to emit
					int a; //Amount

					void *pEmitterParam;
				};

				struct Particle
				{
					float x;
					float y;
					float z;
					float dx;
					float dy;
					float dz;
					float ddx;
					float ddy;
					float ddz;
					float rot;
					float drot;
					float ddrot;
					float dds;

					float ddsize;
					float dsize;
					float size;

					GLuint Texture;

					float currentLife;
					float totalLife;					

					float d_fluc;
					float drot_fluc;

					bool dead;
					void *pParam;
				};

				struct AreaMotionParameters
				{
					//rf = Random Factor, maa = Maximum Vector, mia = Minimum Vector
					Particle init;

					float xyz_rf;

					float d_max;
					float d_min;

					float dd_max;
					float dd_min;

					float rot_max;
					float rot_min;

					float drot_min;
					float drot_max;

					float ddrot_min;
					float ddrot_max;

					float life_min;
					float life_max;
					
					float s_min;
					float s_max;

					float dsize_min;
					float dsize_max;

					float ddsize_min;
					float ddsize_max;
				};

				struct DirectionMotionParameters
				{
					//rf = Random Factor, mia = Minimum Vector
					Particle init;

					float nx;
					float ny;
					float nz;

					float u_rf;
					float v_rf;

					float ddu_rf;
					float ddv_rf;

					float d_rf;
					float d_mia;

					float dd_rf;
					float dd_mia;
				};

				struct CircularMotionParameters
				{
					Particle init;
					
					float d_acc_x;
					float d_acc_y;
					float d_acc_z;

					float r;
					float d_min;
					float d_max;
				};

				Particle **pParticles;
				MTRand *random;

				int count;
				int deadCount;

				float centX;
				float centY;
				float centZ;
				float scale;
				GLfloat matrix[16];

				Utilities::Vector3D normal;

				float GetRandomRange(float min, float max);
				void InitAreaMotionParticle(Particle **start, int indexstart, int count, AreaMotionParameters params);
				void InitDirectionalMotionParticle(Particle **start, int count, DirectionMotionParameters params);
				void InitCircularMotionParticle(Particle **start, int count, CircularMotionParameters params);
				void ClearParticles();
				void StartBillboarding();
				void TranslateToLocation();
				void StartEffectRender();
				void EndEffectRender();
				void GetBillboardCoords();

				void IterateMotion(Particle **start, int count, float time);
				GLuint blur;

			public:
				ParticleSystem()
				{
					pParticles = NULL;
					count = 0;
				}
				void SetBlurPicture(GLuint pic) { blur = pic; this->scale = 1.0f; }
				void SetRandomGenerator(MTRand *randomgenerator) {random = randomgenerator;}
				virtual void Init(float x, float y, float z, float size) {};
				virtual void Iterate(float time) {};
				virtual ParticleStats Render() {return DONE;};
				void SetParticleCount(int count);
				virtual ~ParticleSystem() { ClearParticles(); }
				friend class ParticleSystemHandler;
		};

		class Explosion : public ParticleSystem
		{
			protected:
				void InitSpecial(float x, float y, float z, float size, ParticleSystemHandler::ExplosionInit data);
			public:
				void Init(float x, float y, float z, float size);
				void Iterate(float time);
				ParticleStats Render();
			friend class ParticleSystemHandler;
		};

		class DirectionalExplosion : public ParticleSystem
		{
			public:
				void Init(float x, float y, float z, float size);
				void Iterate(float time);
				ParticleStats Render();
		};

		class CircularParticles : public ParticleSystem
		{
			private:
				float r;
				void Emit(int index);
				void IterateCircularMotion(Particle **start, int count, float time);
			public:
				void Init(float x, float y, float z, float size);
				void Iterate(float time);
				ParticleStats Render();
		};
		
		class ParticleNode : public Scene::Graph::Node
		{
			private:
				ParticleNode();
			protected:
				virtual void Render();
			public:
				static ParticleNode instance;
		};

	}
}

#ifdef DEBUG_DEP
#warning "effect.h-end"
#endif

#endif
