#pragma once
#include "CryEntitySystem/IEntityComponent.h"
#include "CryEntitySystem/IEntity.h"
#include "CryEntitySystem/IEntitySystem.h"

class CDirMover : public IEntityComponent, IEntityPropertyGroup
{
public:
	CDirMover() {};
	virtual ~CDirMover() {};

	auto Initialize() -> void final {
	
		m_pTarget = gEnv->pEntitySystem->GetEntity(2);
	
	};

	// Reflect type to set a unique identifier for this component
	static void ReflectType(Schematyc::CTypeDesc<CDirMover>& desc)
	{
		desc.SetGUID("{e623f9fa-2c10-432a-a5b0-9ca37a7ab550}"_cry_guid);
		desc.SetLabel("Dir Mover");
		desc.SetDescription("Moves Entity in a direction");
		desc.SetComponentFlags(EntityComponentFlags(EEntityComponentFlags::Singleton));
	}

	void Serialize(Serialization::IArchive& archive) override {};


	const char* GetLabel() const final { return "Entt Mover"; }


	void SerializeProperties(Serialization::IArchive& archive) override {};

	IEntityPropertyGroup* GetPropertyGroup() final { return this; }


	Cry::Entity::EventFlags GetEventMask() const override { return EEntityEvent::Update; }



	float m_speed = 1;
	Vec3 m_dir = ZERO;


protected:
	void ProcessEvent(const SEntityEvent& event) override {
		if (event.event == EEntityEvent::Update)
		{
			if (!m_pTarget)
				return;

			float dt = event.fParam[0];
			auto tm = GetEntity()->GetWorldTM();
			auto pos = tm.GetTranslation();
			auto targetPos = m_pTarget->GetPos();

			m_dir = { (targetPos - pos).GetNormalizedFast() };

			tm.AddTranslation(m_dir * m_speed * dt);
			GetEntity()->SetWorldTM(tm);
		}
	};

	IEntity* m_pTarget = nullptr;
};
