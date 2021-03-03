#pragma once

#include "CryEntitySystem/IEntityComponent.h"

#include "entt/Components/DamageComponents.h"
#include <entt/fwd.hpp>


class CTargetSpawner : public IEntityComponent, IEntityPropertyGroup, IEntityComponentPreviewer
{
public:
	CTargetSpawner();
	virtual ~CTargetSpawner();

	auto Initialize() -> void final;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<CTargetSpawner>& desc)
	{
		desc.SetGUID("{9d04766b-902d-4dbe-a54f-13b625f4db99}"_cry_guid);
		desc.SetLabel("Target Spawner");
		desc.SetDescription("Spawns random target entities");
		desc.SetComponentFlags(EntityComponentFlags(EEntityComponentFlags::Singleton));
	}

	void Serialize(Serialization::IArchive& archive) override;

	auto GetPreviewer()->IEntityComponentPreviewer* final { return this; }


	const char* GetLabel() const final { return "Target Spawner"; }


	void SerializeProperties(Serialization::IArchive& archive) override;

	IEntityPropertyGroup* GetPropertyGroup() final { return this; }

	void Render(const IEntity& entity, const IEntityComponent& component, SEntityPreviewContext& context) const override;


	Cry::Entity::EventFlags GetEventMask() const override;

private:
	auto Spawn()	-> void;
	auto Despawn()	-> void;

	Vec2 rect = { 1.f, 1.f };

	AABB area;

	float speed = 0;

	std::vector<entt::entity> m_spawnedEntities;

	_smart_ptr<IStatObj> m_geom;
	_smart_ptr<IMaterial> m_material;

	uint32 count = 0;


	Components::Damage::MaxHealth  m_health;
protected:
	void ProcessEvent(const SEntityEvent& event) override;
};