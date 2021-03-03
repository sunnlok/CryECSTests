#pragma once


namespace Components::Transform
{
	struct WorldTransform
	{
		Matrix34 tm;
	};

	struct Position {
		Vec3 pos;
	};

	struct Acceleration
	{
		Vec3 val = ZERO;
	};

	struct Direction
	{
		Vec3 dir;
	};

	struct MinSpeed
	{
		float speed = 1.f;

		void Serialize(Serialization::IArchive& ar)
		{
			ar(speed, "speed", "Speed");
		}
	};

	struct MaxSpeed
	{
		float speed = 1.f;

		void Serialize(Serialization::IArchive& ar)
		{
			ar(speed, "speed", "Speed");
		}

	};

	struct Velocity
	{
		Vec3 velocity = ZERO;
	};

	struct AABBConstraint
	{
		float radius = 0.5f;
		AABB area = AABB(radius);
		float weight	= 1;
		float maxForce	= 0.1;
		
		void Serialize(Serialization::IArchive& ar)
		{		
			ar(radius, "radius", "Radius");
			if(ar.isInput())
				area = AABB(radius);


			ar(weight, "weight", "Weight");
			ar(maxForce, "maxForce", "Max Force");
		}
	};
}