#ifndef __EFFECT_H_PRE__
#define __EFFECT_H_PRE__

#ifdef DEBUG_DEP
#warning "effect.h-pre"
#endif

namespace Game
{
	namespace FX
	{
		enum ParticleStats
		{
			RUNNING = 0,
			DONE
		};

		enum StepType
		{
			START = 0, //i.e. Spark
			DURATION, //i.e. Fire
			FINISH, //i.e. Smoke
			COUNT
		};

		enum EffectType
		{
			PARTICLE_SPHERICAL_EXPLOSION,
			PARTICLE_DIRECTIONAL_EXPLOSION,
			NOT_INITED
		};

		class ParticleSystem;
		class Explosion;
		class CircularParticles;
		class DirectionalExplosion;
		class ParticleSystemHandler;

	}
}

#endif

