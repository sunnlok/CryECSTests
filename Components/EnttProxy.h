#pragma once
#include "CryEntitySystem/IEntityComponent.h"
#include <entt/entity/registry.hpp>
#include <entt/meta/meta.hpp>

class CEnttProxy : public IEntityComponent, IEntityPropertyGroup
{
public:
	CEnttProxy();
	~CEnttProxy();

	auto Initialize() -> void final;

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<CEnttProxy>& desc)
	{
		desc.SetGUID("{8ec8e54a-e9a0-4176-a322-cb81850d36ec}"_cry_guid);
		desc.SetLabel("Entt Proxy");
		desc.SetDescription("Entt component proxy");
		desc.SetComponentFlags(EntityComponentFlags(EEntityComponentFlags::Singleton));
	}

	entt::entity GetEntity() const { return entity; }

	void Serialize(Serialization::IArchive& archive) override;


	const char* GetLabel() const override;


	void SerializeProperties(Serialization::IArchive& archive) override;

	IEntityPropertyGroup* GetPropertyGroup() final { return this; }
private:
	entt::entity entity;
	string test;
	entt::registry& registry;


	std::vector<std::tuple<uint32, entt::meta_any>> m_attachedComponents;

};
