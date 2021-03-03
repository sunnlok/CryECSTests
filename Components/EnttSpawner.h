#pragma once

#include <entt/entity/fwd.hpp>

#include "CryEntitySystem/IEntityComponent.h"

#include "entt/Components/BoidComponents.h"
#include "entt/Components/TransformComponents.h"
#include "entt/Components/PhysicsComponents.h"
#include "entt/Components/TargetingComponents.h"
#include "entt/Components/DamageComponents.h"

class CEnttSpawner : public IEntityComponent, IEntityPropertyGroup, IEntityComponentPreviewer
{
public:
	CEnttSpawner();
	virtual ~CEnttSpawner();

	auto Initialize() -> void final;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<CEnttSpawner>& desc)
	{
		desc.SetGUID("{6e7894bd-ca4c-4551-9542-a8d59d4f04f0}"_cry_guid);
		desc.SetLabel("Entt Spawner");
		desc.SetDescription("Spawns random entt entities");
		desc.SetComponentFlags(EntityComponentFlags(EEntityComponentFlags::Singleton));
	}

	void Serialize(Serialization::IArchive& archive) override;

	auto GetPreviewer()->IEntityComponentPreviewer* final { return this; }


	const char* GetLabel() const final { return "Entt Spawner"; }


	void SerializeProperties(Serialization::IArchive& archive) override;

	IEntityPropertyGroup* GetPropertyGroup() final { return this; }

	void Render(const IEntity& entity, const IEntityComponent& component, SEntityPreviewContext& context) const override;


	Cry::Entity::EventFlags GetEventMask() const override;

private:
	auto Spawn()	-> void;
	auto SpawnCE()	-> void;
	auto Despawn()	-> void;

	bool m_bUseEntt = true;
	bool m_bEnabled;
	uint32 m_spawnCount;
	float m_speed = 1;

	AABB  m_area = AABB(Vec3(1, 1, 1));


	_smart_ptr<IStatObj> m_geom;
	_smart_ptr<IMaterial> m_material;


	float seperationWeight = 1.0f;
	float centerWeight = 1.0f;
	float averageWeight = 1.0f;
	float constraintWeight = 1.f;

	float viewDistance = 10.f;
	float avoidDistance = 1.f;


	float radius = 1.f;
	std::vector<entt::entity> m_spawnedEntities;

	std::vector<EntityId> m_spawnedCEEntities;

	bool bIsTarget = false;
	bool bCanTarget = false;
protected:
	void ProcessEvent(const SEntityEvent& event) override;

	Components::Boids::FlockParams		flockParams;
	Components::Boids::FlockWeights		flockWeights;
	Components::Boids::FlockForceLimits forceLimits;
	Components::Boids::FlockID			flockID;

	Components::Transform::MaxSpeed			maxSpeed;
	Components::Transform::MinSpeed			minSpeed;
	Components::Transform::AABBConstraint	areaConstraint;

	Components::Physics::RayHitAvoidance	hitAvoidance;

	Components::Targeting::Targeting targeting;
	Components::Damage::MaxHealth maxHealth;
	Components::Damage::Damage damage;
	Components::Damage::TickedDamage tickDamage;
};