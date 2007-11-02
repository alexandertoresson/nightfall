#include "effect.h"

#include "game.h"
#include <cmath>

#undef PI
#define PI 3.1415926535897932384626433832795

using namespace std;
namespace Game
{
	namespace FX
	{
		ParticleSystemHandler *pParticleSystems = NULL;

		ParticleSystemHandler::ParticleSystemHandler(int systemCount, int particleDetail)
		{
			this->systemCount = systemCount;
			this->particleDetail = particleDetail;
			this->particleSystems = new ParticleSystem*[this->systemCount];
			this->particleEffect = new EffectType[this->systemCount];
			this->particleStats = new ParticleStats[this->systemCount];

			blurpic = Utilities::LoadTexture("textures/blur.png");
			ExplosionPics.blob = Utilities::LoadTexture("textures/blob.png");
			ExplosionPics.blob2 = Utilities::LoadTexture("textures/blob2.png");
			ExplosionPics.splinter = Utilities::LoadTexture("textures/dots.png");
			ExplosionPics.center = Utilities::LoadTexture("textures/cloud.png");

			for(int i = 0; i < this->systemCount; i++)
			{
				 this->particleSystems[i] = NULL;
				 this->particleEffect[i] = NOT_INITED;
				 this->particleStats[i] = DONE;
			}

			this->start = 0;
			this->randomgenerator = new MTRand();
		}

		void ParticleSystemHandler::InitEffect(float x, float y, float z, float size, EffectType type)
		{
			if(type == PARTICLE_SPHERICAL_EXPLOSION)
			{
				Explosion *particleExplosion;
				if(particleEffect[start] == type)
				{
					particleExplosion = (Explosion*)particleSystems[start];
				}
				else if(this->particleSystems[start] != NULL)
				{
					delete particleSystems[start];
					particleSystems[start] = particleExplosion = new Explosion();
					particleExplosion->SetRandomGenerator(randomgenerator);
					particleExplosion->SetParticleCount(particleDetail);
					particleExplosion->SetBlurPicture(blurpic);
				}
				else
				{
					particleSystems[start] = particleExplosion = new Explosion();
					particleExplosion->SetRandomGenerator(randomgenerator);
					particleExplosion->SetParticleCount(particleDetail);
					particleExplosion->SetBlurPicture(blurpic);
				}

				this->particleStats[start] = RUNNING;
				this->particleEffect[start] = PARTICLE_SPHERICAL_EXPLOSION;
				particleExplosion->InitSpecial(x, y, z, size, ExplosionPics);
			}
			
			start++;
			if(start == this->systemCount)
				start = 0;
		}

		void ParticleSystemHandler::Iterate(float time)
		{
			for(int i = 0; i < this->systemCount; i++)
			{
				if(particleStats[i] == DONE)
					continue;

				if(particleSystems[i] != NULL)
					particleSystems[i]->Iterate(time);
			}
		}

		void ParticleSystemHandler::Render()
		{
			for(int i = 0; i < this->systemCount; i++)
			{
				if(particleStats[i] == DONE)
					continue;

				if(particleSystems[i] != NULL)
				{
					glPushMatrix();
					if (particleSystems[i]->Render() == DONE)
					{
						particleStats[i] = DONE;
					}

					glPopMatrix();
				}
			}
		}

		ParticleSystemHandler::~ParticleSystemHandler()
		{
			for(int i = 0; i < this->systemCount; i++)
			{
				if(particleSystems == NULL)
					continue;

				delete particleSystems[i];
			}
			glDeleteTextures(1, &blurpic);
		}

		float ParticleSystem::GetRandomRange(float min, float abs)
		{
			float val = (random->rand(2.0) - 1.0) * (abs - min);
			if(val < 0)
				val -= min;
			else
				val += min;

			return val;
		}

		void ParticleSystem::InitAreaMotionParticle(Game::FX::ParticleSystem::Particle **start, int indexstart, int count, Game::FX::ParticleSystem::AreaMotionParameters params)
		{
			int endindex = indexstart + count;
			for(int i = indexstart; i < endindex; i++)
			{
				Particle *pCurrent = start[i];
				pCurrent->x = params.init.x + (random->rand(2.0) - 1.0) * params.xyz_rf;
				pCurrent->y = params.init.y + (random->rand(2.0) - 1.0) * params.xyz_rf;
				pCurrent->z = params.init.z + (random->rand(2.0) - 1.0) * params.xyz_rf;

				double vector = GetRandomRange(params.d_min, params.d_max);
				double randu = random->rand(PI);
				double randv = random->rand(PI * 2.0);

				pCurrent->dx = params.init.dx + vector*sin(randu)*cos(randv);
				pCurrent->dy = params.init.dy + vector*sin(randu)*sin(randv);
				pCurrent->dz = params.init.dz + vector*cos(randu);

				pCurrent->ddx = params.init.ddx;
				pCurrent->ddy = params.init.ddy;
				pCurrent->ddz = params.init.ddz;

				pCurrent->ddrot = params.init.ddrot + GetRandomRange(params.ddrot_min, params.ddrot_max);
				pCurrent->drot = params.init.drot + GetRandomRange(params.drot_min, params.drot_max);
				pCurrent->rot = params.init.rot + GetRandomRange(params.rot_min, params.rot_max);

				pCurrent->ddsize = params.ddsize_min + random->rand() * (params.ddsize_max - params.ddsize_min);
				pCurrent->dsize = params.dsize_min + random->rand() * (params.dsize_max - params.dsize_min);

				pCurrent->dead = false;
				pCurrent->totalLife = pCurrent->currentLife = params.life_min + random->rand() * (params.life_max - params.life_min);
				pCurrent->size = params.s_min + random->rand() * (params.s_max - params.s_min);

				pCurrent->drot_fluc = params.init.drot_fluc;
				pCurrent->d_fluc = params.init.d_fluc;
				
				pCurrent->Texture = params.init.Texture;

				start[i] = pCurrent;
			}
		}

		void ParticleSystem::InitDirectionalMotionParticle(Game::FX::ParticleSystem::Particle **start, int count, Game::FX::ParticleSystem::DirectionMotionParameters params)
		{
			double v = acos(params.nz / sqrt(params.nx * params.nx + params.ny * params.ny + params.nz * params.nz));
			double u = atan(params.ny / params.nz);

			for (int i = 0; i < count; i++)
			{
				Particle *pCurrent = start[i];
				
				double randu = u + random->rand(params.v_rf * 2.0) - params.v_rf;
				double randv = v + random->rand(params.u_rf * 2.0) - params.u_rf;
				double vector = GetRandomRange(params.d_mia, params.d_rf);

				pCurrent->x = params.init.x;
				pCurrent->y = params.init.y;
				pCurrent->z = params.init.z;

				pCurrent->dx = params.init.dx + vector*sin(randu)*cos(randv);
				pCurrent->dy = params.init.dy + vector*sin(randu)*sin(randv);
				pCurrent->dz = params.init.dz + vector*cos(randu);

				pCurrent->ddx = params.init.ddx;
				pCurrent->ddy = params.init.ddy;
				pCurrent->ddz = params.init.ddz;

				pCurrent->dead = false;
				pCurrent->currentLife = pCurrent->totalLife = random->rand(2.0f) + 1.0f;

				pCurrent->size = 1.0f;

				pCurrent->drot = 0.0f;
				pCurrent->drot_fluc = 0.0f;
				pCurrent->rot = 0.0f;

				pCurrent->Texture = blur;
			}

		}

		void ParticleSystem::ClearParticles()
		{
			for(int i = 0; i < count; i++)
			{
				Particle *tmp = pParticles[i];
//				if(tmp->pParam != NULL)
//					delete tmp->pParam;    // deleting void* gives undefined results
				delete tmp;
			}
			delete pParticles;
		}

		void ParticleSystem::IterateMotion(FX::ParticleSystem::Particle **start, int count, float time)
		{
			for(int i = 0; i < count; i++)
			{
				Particle *pCurrent = start[i];
				if(pCurrent->dead == true)
					continue;

				pCurrent->dx += pCurrent->ddx * time + (this->random->rand(2.0) - 1.0) * pCurrent->d_fluc * time;
				pCurrent->dy += pCurrent->ddy * time + (this->random->rand(2.0) - 1.0) * pCurrent->d_fluc * time;
				pCurrent->dz += pCurrent->ddz * time + (this->random->rand(2.0) - 1.0) * pCurrent->d_fluc * time;
				
				pCurrent->x += pCurrent->dx * time;
				pCurrent->y += pCurrent->dy * time;
				pCurrent->z += pCurrent->dz * time;

				pCurrent->dsize += pCurrent->ddsize * time;
				pCurrent->size += pCurrent->dsize * time;

				pCurrent->drot += pCurrent->ddrot * time;
				pCurrent->rot += pCurrent->drot * time;

				if(pCurrent->currentLife != 0.0f)
				{
					pCurrent->currentLife -= time;
					if(pCurrent->currentLife <= 0.0f)
					{
						pCurrent->currentLife = 0.0f;
						pCurrent->dead = true;
						deadCount++;
					}
				}

				pCurrent->x += pCurrent->dx * time;
				pCurrent->y += pCurrent->dy * time;
				pCurrent->z += pCurrent->dz * time;
			}
		}

		void Explosion::Iterate(float time)
		{
			IterateMotion(pParticles, count, time);
		}

		ParticleStats Explosion::Render()
		{
			if(count == deadCount)
				return DONE;
			if(Dimension::SquareIsVisible(Dimension::currentPlayerView, (int)centX, (int)centY) == false)
				return RUNNING;

			glPushMatrix();

			StartEffectRender();
			for(int i = 0; i < count; i++)
			{
				Particle *pCurrent = pParticles[i];
				if(pCurrent->dead == true)
					continue;

				float alpha = pCurrent->currentLife / pCurrent->totalLife;

				glPushMatrix();
				glTranslatef(pCurrent->x, pCurrent->y, pCurrent->z);

				//Multiply with the billboard matrix
				glMultMatrixf((GLfloat*)&matrix);

				glScalef(pCurrent->size, pCurrent->size, 0.0f);
				glRotatef(pCurrent->rot, 0.0f, 0.0f, 1.0f);
				
				glColor4f(1.0f, 1.0f, 1.0f, alpha);
				glBindTexture(GL_TEXTURE_2D, pCurrent->Texture);
				glBegin(GL_QUADS);
					glTexCoord2f(0.0f, 0.0f);
					glVertex3f(-0.5f, -0.5f, 0.0f);

					glTexCoord2f(1.0f, 0.0f);
					glVertex3f(0.5f, -0.5f, 0.0f);

					glTexCoord2f(1.0f, 1.0f);
					glVertex3f(0.5f, 0.5f, 0.0f);

					glTexCoord2f(0.0f, 1.0f);
					glVertex3f(-0.5f, 0.5f, 0.0f);
				glEnd();
				glPopMatrix();
			}
			glPopMatrix();
			EndEffectRender();
			return RUNNING;
		}

		void ParticleSystem::SetParticleCount(int count)
		{
			this->count = count;
			pParticles = new Particle*[count];
			for(int i = 0; i < count; i++)
			{
				pParticles[i] = new Particle();
			}
		}

		void ParticleSystem::TranslateToLocation()
		{
			Dimension::SetParticleCoordSpace(centX, centY, centZ, scale);
		}

		void ParticleSystem::StartBillboarding()
		{
			Utilities::Vector3D near_plane, far_plane, up, right, look, window_coord;

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
		}

		void ParticleSystem::StartEffectRender()
		{
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glDepthMask(GL_FALSE);

			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			TranslateToLocation();
			StartBillboarding();
		}

		void ParticleSystem::EndEffectRender()
		{
			glDepthMask(GL_TRUE);
			glEnable(GL_CULL_FACE);
			glEnable(GL_LIGHTING);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		void ParticleSystem::GetBillboardCoords()
		{
			glMultMatrixf((GLfloat*)&matrix);
		}

		void Explosion::Init(float x, float y, float z, float size)
		{
			centX = x;
			centY = y;
			centZ = z;
			this->scale = size;

			deadCount = 0;
			AreaMotionParameters params;
			params.init.x = 0;
			params.init.y = 0;
			params.init.z = 0;
			params.init.dx = params.init.dy = params.init.dz = 0.0f;
			params.init.ddx = params.init.ddy = params.init.ddz = 0.0f;
			params.init.ddy = -3.0f;
			params.init.ddrot = params.init.drot = params.init.rot = 0.0f;
			params.init.d_fluc = 0.0f;
			params.init.drot_fluc = 0.0f;
			params.init.Texture = blur;

			params.life_min = 0.5f;
			params.life_max = 2.0f;
			params.s_min = 0.3f;
			params.s_max = 1.0f;
			params.xyz_rf = 0.5f;
			params.d_min = 1.0f;
			params.d_max = 3.0f;
			params.dd_min = 1.0f;
			params.dd_max = 5.0f;
			params.drot_min = 0.0f;
			params.drot_max = 90.0f;
			params.ddrot_min = 0.0f;
			params.ddrot_max = 0.0f;
			params.rot_min = 0.0f;
			params.rot_max = 30.0f;
			

			InitAreaMotionParticle(pParticles, 0, count, params);
		}

		void Explosion::InitSpecial(float x, float y, float z, float size, ParticleSystemHandler::ExplosionInit textures)
		{
			int parts = count / 10;

			int center = parts * 2;
			int spliter = parts * 1;
			int glowing = parts * 4;
			int glowing2 = parts * 3;

			int currLoc = 0;
			
			glowing2 += parts % 10;

			centX = x;
			centY = y;
			centZ = z;
			this->scale = size;

			//Center
			deadCount = 0;
			AreaMotionParameters params;
			params.init.x = 0;
			params.init.y = 0;
			params.init.z = 0;
			params.init.dx = params.init.dy = params.init.dz = 0.0f;
			params.init.ddx = params.init.ddy = params.init.ddz = 0.0f;
			params.init.ddrot = params.init.drot = params.init.rot = 0.0f;
			params.init.d_fluc = 0.0f;
			params.init.drot_fluc = 0.0f;
			params.init.Texture = textures.center;

			params.life_min = 0.1f;
			params.life_max = 1.0f;
			params.s_min = 0.2f;
			params.s_max = 0.5f;
			params.xyz_rf = 0.0f;
			params.d_min = 0.2f;
			params.d_max = 0.5f;
			params.dd_min = 1.0f;
			params.dd_max = 1.0f;
			params.drot_min = 0.0f;
			params.drot_max = 90.0f;
			params.ddrot_min = 0.0f;
			params.ddrot_max = 0.0f;
			params.rot_min = 0.0f;
			params.rot_max = 10.0f;
			params.dsize_min = 15.0f;
			params.dsize_max = 20.0f;
			params.ddsize_min = -20.0f;
			params.ddsize_max = -30.0f;

			InitAreaMotionParticle(pParticles, currLoc, center, params);
			currLoc += center;

			//spliter
			params.init.Texture = textures.splinter;
			params.life_min = 0.5f;
			params.life_max = 1.2f;
			params.xyz_rf = 0.0f;
			params.d_min = 0.5f;
			params.d_max = 1.0f;
			params.dd_min = 0.0f;
			params.dd_max = 0.0f;
			params.drot_min = 0.0f;
			params.drot_max = 0.0f;
			params.rot_max = 0.0f;
			params.s_min = 0.0f;
			params.s_max = 0.2f;

			params.dsize_min = 10.0f;
			params.dsize_max = 30.0f;
			params.ddsize_min = -1.0f;
			params.ddsize_max = -3.0f;

			InitAreaMotionParticle(pParticles, currLoc, spliter, params);
			currLoc += spliter;

			//glowing parts
			params.s_min = 0.5f;
			params.s_max = 1.2f;
			params.init.Texture = textures.blob;
			params.life_min = 1.0f;
			params.life_max = 2.0f;
			params.d_min = 1.0f;
			params.d_max = 3.0f;
			params.dd_min = 1.0f;
			params.dd_max = 5.0f;
			params.drot_min = 0.0f;
			params.drot_max = 90.0f;
			params.rot_max = 30.0f;
			params.dsize_min = 0.0f;
			params.dsize_max = 0.0f;
			params.ddsize_min = 0.0f;
			params.ddsize_max = 0.0f;
			
			InitAreaMotionParticle(pParticles, currLoc, glowing, params);
			currLoc += glowing;

			InitAreaMotionParticle(pParticles, currLoc, glowing2, params);
			currLoc += glowing2;
		}

		void CircularParticles::Init(float x, float y, float z, float size)
		{
			centX = x;
			centY = y;
			centZ = z;
			this->scale = size;

			CircularMotionParameters params;
			params.init.x = centX;
			params.init.y =  centY;
			params.init.z = centZ;

			params.init.dx = params.init.dy = params.init.dz = 0.0f;
			params.init.ddx = params.init.ddy = params.init.ddz = 0.0f;
			params.init.ddrot = params.init.drot = params.init.rot = 0.0f;
			params.init.d_fluc = 0.0f;
			params.init.drot_fluc = 0.0f;

			params.init.size = 0.5f;

			params.d_acc_x = x;
			params.d_acc_y = y;
			params.d_acc_z = z;

			params.r = 5.0f;
			params.d_min = 1.0f;
			params.d_max = 5.0f;
			InitCircularMotionParticle(pParticles, count, params);
		}

		void CircularParticles::Emit(int index)
		{
			CircularMotionParameters params;
			params.init.x = centX;
			params.init.y =  centY;
			params.init.z = centZ;

			params.init.dx = params.init.dy = params.init.dz = 0.0f;
			params.init.ddx = params.init.ddy = params.init.ddz = 0.0f;
			params.init.ddrot = params.init.drot = params.init.rot = 0.0f;
			params.init.d_fluc = 0.0f;
			params.init.drot_fluc = 0.0f;

			params.init.size = 0.5f;

			params.d_acc_x = centX;
			params.d_acc_y = centY;
			params.d_acc_z = centZ;

			params.r = 5.0f;
			params.d_min = 1.0f;
			params.d_max = 5.0f;
			
			InitCircularMotionParticle(&pParticles[index], 1, params);
		}

		void ParticleSystem::InitCircularMotionParticle(Particle **start, int count, CircularMotionParameters params)
		{
			deadCount = 0;
			for (int i = 0; i < count; i++)
			{
				Particle *pCurrent = start[i];
				double vector = GetRandomRange(params.d_min, params.d_max);
				double randu = random->rand(PI);
				double randv = random->rand(PI * 2.0);

				pCurrent->dx = params.init.dx + vector*sin(randu)*cos(randv);
				pCurrent->dy = params.init.dy + vector*sin(randu)*sin(randv);
				pCurrent->dz = params.init.dz + vector*cos(randu);

				vector = GetRandomRange(0.0, params.r);
				randu = random->rand(PI);
				randv = random->rand(PI * 2.0);

				pCurrent->x = params.init.dx + vector*sin(randu)*cos(randv) * params.r;
				pCurrent->y = params.init.dy + vector*sin(randu)*sin(randv) * params.r;
				pCurrent->z = params.init.dz + vector*cos(randu) * params.r;

				pCurrent->ddx = 0.0f;
				pCurrent->ddy = 0.0f;
				pCurrent->ddz = 0.0f;

				pCurrent->size = params.init.size;

				pCurrent->dead = false;

				pCurrent->totalLife = pCurrent->currentLife = random->rand(5.0) + 5.0f;

				pCurrent->Texture = blur;

			}
		}

		void CircularParticles::Iterate(float time)
		{
			IterateCircularMotion(pParticles, count, time);
		}

		ParticleStats CircularParticles::Render()
		{
			StartEffectRender();
			for(int i = 0; i < count; i++)
			{
				Particle *pCurrent = pParticles[i];
				if(pCurrent->dead == true)
					continue;

				float alpha = pCurrent->currentLife / pCurrent->totalLife;

				glPushMatrix();
				glTranslatef(pCurrent->x, pCurrent->y, pCurrent->z);
				glMultMatrixf((GLfloat*)&matrix);
				glScalef(pCurrent->size, pCurrent->size, 0.0f);
				glRotatef(pCurrent->rot, 0.0f, 0.0f, 1.0f);

				glColor4f(0.0f, 1.0f, 1.0f, alpha);
				glBindTexture(GL_TEXTURE_2D, pCurrent->Texture);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-0.5f, -0.5f, 0.0f);

				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(0.5f, -0.5f, 0.0f);

				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(0.5f, 0.5f, 0.0f);

				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(-0.5f, 0.5f, 0.0f);
				glEnd();
				glPopMatrix();
			}
			glDepthMask(GL_TRUE);
			glEnable(GL_CULL_FACE);
			glEnable(GL_LIGHTING);
			return RUNNING;
		}

		void CircularParticles::IterateCircularMotion(FX::ParticleSystem::Particle **start, int count, float time)
		{
			for (int i = 0; i < count; i++)
			{
				Particle *pCurrent = start[i];
				float deltaX = centX - pCurrent->x;
				float deltaY = centY - pCurrent->y;
				float deltaZ = centZ - pCurrent->z;
				
				float distance = sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
				
				float acc = 1.0f * distance;
				
				Utilities::Vector3D vect;
				vect.set(deltaX, deltaY, deltaZ);
				vect.normalize();

				pCurrent->ddx = vect.x * acc;
				pCurrent->ddy = vect.y * acc;
				pCurrent->ddz = vect.z * acc;

				pCurrent->dx += pCurrent->ddx * time;
				pCurrent->dy += pCurrent->ddy * time;
				pCurrent->dz += pCurrent->ddz * time;

				pCurrent->x += pCurrent->dx * time;
				pCurrent->y += pCurrent->dy * time;
				pCurrent->z += pCurrent->dz * time;

				pCurrent->currentLife -= time;
				if(pCurrent->currentLife < 0.0)
				{
					pCurrent->currentLife = 0.0f;
					Emit(i);
				}
			}
		}

		void DirectionalExplosion::Init(float x, float y, float z, float size)
		{
			ParticleSystem::DirectionMotionParameters params;

			centX = x;
			centY = y;
			centZ = z;
			this->scale = size;

			params.init.x = 0;
			params.init.y = 0;
			params.init.z = 0;

			params.init.dx = 0.0f;
			params.init.dy = 0.0f;
			params.init.dz = 0.0f;

			params.init.ddx = 0.0f;
			params.init.ddy = -9.0f;
			params.init.ddz = 0.0f;

			params.nx = 0.0f;
			params.ny = 1.0f;
			params.nz = 0.0f;

			params.d_mia = 0.5f;
			params.d_rf = 20.0f;

			params.dd_mia = 0.5f;
			params.dd_rf = 5.0f;

			params.u_rf = PI / 20.0;
			params.v_rf = PI / 10.0;

			params.ddu_rf = PI / 2.0;
			params.ddv_rf = PI / 4.0;

			InitDirectionalMotionParticle(pParticles, count, params);
		}

		void DirectionalExplosion::Iterate(float time)
		{
			IterateMotion(pParticles, count, time);
		}

		ParticleStats DirectionalExplosion::Render()
		{
			if(count == deadCount)
				return DONE;
			
			glPushMatrix();

			StartEffectRender();

			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glDepthMask(GL_FALSE);
			for(int i = 0; i < count; i++)
			{
				Particle *pCurrent = pParticles[i];
				if(pCurrent->dead == true)
					continue;

				float alpha = pCurrent->currentLife / pCurrent->totalLife;

				glPushMatrix();
				glTranslatef(pCurrent->x, pCurrent->y, pCurrent->z);

				glMultMatrixf((GLfloat*)&matrix);

				glScalef(pCurrent->size, pCurrent->size, 0.0f);
				glRotatef(pCurrent->rot, 0.0f, 0.0f, 1.0f);

				glColor4f(1.0f, 1.0f, 1.0f, alpha);
				glBindTexture(GL_TEXTURE_2D, pCurrent->Texture);

				glBegin(GL_QUADS);
					glTexCoord2f(0.0f, 0.0f);
					glVertex3f(-0.5f, -0.5f, 0.0f);

					glTexCoord2f(1.0f, 0.0f);
					glVertex3f(0.5f, -0.5f, 0.0f);

					glTexCoord2f(1.0f, 1.0f);
					glVertex3f(0.5f, 0.5f, 0.0f);

					glTexCoord2f(0.0f, 1.0f);
					glVertex3f(-0.5f, 0.5f, 0.0f);
				glEnd();
				glPopMatrix();
			}

			glDepthMask(GL_TRUE);
			glEnable(GL_CULL_FACE);
			glEnable(GL_LIGHTING);
			glPopMatrix();
			return RUNNING;
		}
	}
}

