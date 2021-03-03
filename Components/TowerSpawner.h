#pragma once

#include "CryEntitySystem/IEntityComponent.h"

#include "entt/Components/DamageComponents.h"
#include <entt/fwd.hpp>


class CTowerSpawner : public IEntityComponent, IEntityPropertyGroup, IEntityComponentPreviewer
{
public:
	CTowerSpawner();
	virtual ~CTowerSpawner();

	auto Initialize() -> void final;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<CTowerSpawner>& desc)
	{
		desc.SetGUID("{7d416d8b-b754-495e-8d24-ff57f2c7e420}"_cry_guid);
		desc.SetLabel("Tower spawner");
		desc.SetDescription("Spawns ");
		desc.SetComponentFlags(EntityComponentFlags(EEntityComponentFlags::Singleton));
	}

	void Serialize(Serialization::IArchive& archive) override;

	auto GetPreviewer()->IEntityComponentPreviewer* final { return this; }


	const char* GetLabel() const final { return "Tower Spawner"; }


	void SerializeProperties(Serialization::IArchive& archive) override;

	IEntityPropertyGroup* GetPropertyGroup() final { return this; }

	void Render(const IEntity& entity, const IEntityComponent& component, SEntityPreviewContext& context) const override;


	Cry::Entity::EventFlags GetEventMask() const override;

private:
	auto Spawn()	-> void;
	auto Despawn()	-> void;

	Vec2 rect;

	float towerRange = 1.f;

	std::vector<entt::entity> m_spawnedEntities;

	AABB area;

	_smart_ptr<IStatObj> m_geom;
	_smart_ptr<IMaterial> m_material;

	uint32 count = 0;

	Components::Damage::Damage			m_damage;
	Components::Damage::TickedDamage m_tickDamage;
protected:

	void ProcessEvent(const SEntityEvent& event) override;
};