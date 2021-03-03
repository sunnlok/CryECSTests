#include "StdAfx.h"
#include "TargetSpawner.h"



#include "GamePlugin.h"
#include "CryCore/StaticInstanceList.h"
#include "CryRenderer/IRenderAuxGeom.h"
#include "CrySchematyc/Env/IEnvRegistrar.h"
#include "CrySchematyc/Env/Elements/EnvComponent.h"
#include "CrySerialization/Decorators/Resources.h"
#include "entt/Components/RenderComponents.h"
#include "entt/Components/TargetingComponents.h"
#include "entt/Components/TransformComponents.h"

CTargetSpawner::CTargetSpawner()
{

}

CTargetSpawner::~CTargetSpawner()
{

}

auto CTargetSpawner::Initialize() -> void
{
	if (!gEnv->IsEditor())
	{
		Spawn();
	}
}

void CTargetSpawner::Serialize(Serialization::IArchive& archive)
{
	archive(rect, "area", "Area");
	archive(speed, "speed", "Speed");
	archive(count, "count", "Count");

	archive(m_health, "health", "Health");

	auto pMaterial = m_material.get();

	{

		string modelName = m_geom ? m_geom->GetFilePath() : "";
		string oldName = modelName;

		archive(Serialization::StaticModelFilename(modelName), "Mesh", "Mesh");

		if (modelName != oldName)
			m_geom = gEnv->p3DEngine->LoadStatObj(modelName);

		using Serialization::MaterialPicker;

		string matName = pMaterial ? pMaterial->GetName() : "";
		string oldMatName = matName;

		archive(Serialization::MaterialPicker(matName), "Material", "Material");

		if (matName != oldMatName)
			m_material = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial(matName, false);

		if (!m_material.get() && m_geom.get())
			m_material = m_geom->GetMaterial();

	}


	if (archive.isInput())
	{
		Vec3 min = rect * -.5;
		min.z = -0.5;
		Vec3 max = rect * 0.5;
		max.z = 0.5;

		AABB bounds{ min, max };

		GetEntity()->SetLocalBounds(bounds, false);
	}

}

void CTargetSpawner::SerializeProperties(Serialization::IArchive& archive)
{
	Serialize(archive);
}

void CTargetSpawner::Render(const IEntity& entity, const IEntityComponent& component, SEntityPreviewContext& context) const
{

	Vec3 min = rect * -.5;
	min.z = -0.5;
	Vec3 max = rect * 0.5;
	max.z = 0.5;

	AABB bounds{ min, max };


	auto entityTm = GetEntity()->GetWorldTM();
	Vec3 forward = (Constants3D::Forward * entityTm).GetNormalized();
	auto pos = entityTm.GetTranslation();

	gEnv->pAuxGeomRenderer->DrawCone(pos + forward * 0.5, forward, 0.2, 0.2, Col_Green);
	gEnv->pAuxGeomRenderer->DrawLine(pos + forward * 0.5, Col_Green, pos + forward * 0.5, Col_Green);
	gEnv->pAuxGeomRenderer->DrawAABB(bounds, entityTm, false, Col_Green, EBoundingBoxDrawStyle::eBBD_Faceted);
}

Cry::Entity::EventFlags CTargetSpawner::GetEventMask() const
{
	return EEntityEvent::Reset;
}

auto CTargetSpawner::Spawn() -> void
{
	using namespace Components;

	auto& reg = CGamePlugin::GetInstance()->GetRegistry();
	auto& transform = GetEntity()->GetWorldTM();

	auto rng = gEnv->pSystem->GetRandomGenerator();



	for (uint32 i = 0; i < count; ++i)
	{
		auto newEnt = reg.create();

		Vec3 spawnPos = rng.GetRandomComponentwise(rect * -.5f, rect * .5f);

		auto newPos = transform.TransformPoint(spawnPos);
		auto dir = transform.TransformVector(Vec3(0, 1.f, 0)).normalize();

		auto tm = Matrix34::Create(Vec3(1.f, 1.f, 1.f), Quat::CreateRotationVDir(dir), newPos);

		
		IBrush* pBrush = static_cast<IBrush*>(gEnv->p3DEngine->CreateRenderNode(eERType_MovableBrush));
		pBrush->DisablePhysicalization(true);
		pBrush->SetMaterial(m_material);
		pBrush->SetEntityStatObj(m_geom, &tm);
		pBrush->SetRndFlags(ERF_MOVES_EVERY_FRAME | ERF_NO_PHYSICS | ERF_CASTSHADOWMAPS | ERF_HAS_CASTSHADOWMAPS, true);


		//pBrush->SetRndFlags(ERF_MOVES_EVERY_FRAME | ERF_NO_PHYSICS, true);

		reg.emplace<Render::RenderComponent>(newEnt, TRenderNodePtr(pBrush)); 
		reg.emplace<Transform::WorldTransform>(newEnt, tm);
		reg.emplace<Components::Transform::Velocity>(newEnt, dir * speed);
		reg.emplace<Targeting::Targeting>(newEnt);


		reg.emplace<Damage::MaxHealth>(newEnt, m_health);
		reg.emplace<Damage::Health>(newEnt, m_health.val);
		reg.emplace<Damage::RemoveIfDead>(newEnt);

		m_spawnedEntities.emplace_back(newEnt);
	}
}

auto CTargetSpawner::Despawn() -> void
{

}

void CTargetSpawner::ProcessEvent(const SEntityEvent& event)
{
	if (event.event != EEntityEvent::Reset)
		return;

	if (event.nParam[0] == 1)
		Spawn();
	else
		Despawn();
}


static void RegisterTargetSpawnerComponent(Schematyc::IEnvRegistrar& registrar)
{
	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CTargetSpawner));
	}
}

CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterTargetSpawnerComponent);