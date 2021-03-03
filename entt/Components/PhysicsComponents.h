#pragma once

struct IPhysicalEntity;



namespace Components::Physics
{
	struct ColliderFromMesh
	{

	};

	struct SphereCollider
	{
		float radius;
	};

	struct BoxCollider
	{

	};

	struct PhysicalEntity {
// 		pe_type type;
// 		_smart_ptr<IPhysicalEntity> pEnt;
	};

	struct RayHitAvoidance {
		float weight = 1;
		float force = 1;

		void Serialize(Serialization::IArchive& ar)
		{
			ar(weight, "weight", "Weight");
			ar(force, "force", "Force");
		}
	};

	struct RayHitAvoidanceReflected {
		float weight = 1;
		float force = 1;

		void Serialize(Serialization::IArchive& ar)
		{
			ar(weight, "weight", "Weight");
			ar(force, "force", "Force");
		}
	};

	struct RigidBody
	{
		float mass;
	};

	struct RayCaster
	{
		Vec3 castDir;
	};

	struct MultiRayCaster
	{
		std::array<Vec3, 4> castDirs;
	};


	struct RayHit
	{
		Vec3 pos;
		Vec3 normal;
	};
}