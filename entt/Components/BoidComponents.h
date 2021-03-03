#pragma once
#include <CryCore/Containers/CryArray.h>

namespace Components::Boids
{
	struct Boid {};

	//-----------------------------------------------------------------

	struct SphereContstraintArea
	{
		float	weight = 2.f;
		Vec3	pos;
		float   radius = 10;
	};

	struct FlockSteering
	{
		Vec3 force = ZERO;
	};

	struct FlockWeights
	{
		float avgHeadingWeight = 1.f;
		float separation = 1.f;
		float cohesionWeight = 1.f;

		void Serialize(Serialization::IArchive& ar)
		{
			ar(avgHeadingWeight, "alignment", "Alignment");
			ar(cohesionWeight, "cohesion", "Cohesion");
			ar(separation, "separation", "Separation");
		}
	};

	struct FlockForceLimits
	{
		float cohesion		= 0.1f;
		float alignment		= 0.1f;
		float separation	= 0.1f;

		void Serialize(Serialization::IArchive& ar)
		{
			ar(alignment, "alignment", "Alignment");
			ar(cohesion, "cohesion", "Cohesion");
			ar(separation, "separation", "Separation");
		}

	};

	struct FlockParams {

		float viewRadius =	4.f;
		float avoidRadius = 1.f;

		void Serialize(Serialization::IArchive& ar)
		{
			ar(viewRadius, "viewRadius", "View Radius");
			ar(avoidRadius, "avoidRadius", "Avoid Radius");
		}
	};

	struct FlockID {
		uint32 val = 0;

		void Serialize(Serialization::IArchive& ar)
		{
			ar(val, "flockID", "Flock ID");
		}
	};

	struct FlockMate {
		uint32	nearMateCount = 0;


		Vec3	alignmentForce		= ZERO;
		Vec3	separationForce		= ZERO;
		Vec3    flockCenter			= ZERO;
		Vec3	cohesionForce		= ZERO;
	};

	struct Target {};




}