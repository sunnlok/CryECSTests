#include "StdAfx.h"
#include "TowerSpawner.h"
#include "GamePlugin.h"
#include "CryCore/StaticInstanceList.h"
#include "CryRenderer/IRenderAuxGeom.h"
#include "CrySchematyc/Env/IEnvRegistrar.h"
#include "CrySchematyc/Env/Elements/EnvComponent.h"
#include "CrySerialization/Decorators/Resources.h"
#include "entt/Components/RenderComponents.h"
#include "entt/Components/TargetingComponents.h"
#include "entt/Components/TransformComponents.h"

CTowerSpawner::CTowerSpawner()
{

}

CTowerSpawner::~CTowerSpawner()
{

}

auto CTowerSpawner::Initialize() -> void
{
	if (!gEnv->IsEditor())
	{
		Spawn();
	}
}

void CTowerSpawner::Serialize(Serialization::IArchive& archive)
{

	archive(rect, "area", "Area");
	archive(towerRange, "range", "Range");
	archive(count, "count", "Count");

	archive(m_damage, "damage", "Damage");
	archive(m_tickDamage, "tick", "Tick");


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
}

void CTowerSpawner::SerializeProperties(Serialization::IArchive& archive)
{
	Serialize(archive);
}

void CTowerSpawner::Render(const IEntity& entity, const IEntityComponent& component, SEntityPreviewContext& context) const
{
	Vec3 min = rect * -.5;
	min.z = -0.5;
	Vec3 max = rect * 0.5;
	max.z = 0.5;

	AABB bounds{ min, max };


	auto entityTm = GetEntity()->GetWorldTM();

	gEnv->pAuxGeomRenderer->DrawAABB(bounds, entityTm, false, Col_Green, EBoundingBoxDrawStyle::eBBD_Faceted);
}

Cry::Entity::EventFlags CTowerSpawner::GetEventMask() const
{
	return EEntityEvent::Reset;
}


#pragma optimize("",off)
auto CTowerSpawner::Spawn() -> void
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

		IBrush* pBrush = static_cast<IBrush*>(gEnv->p3DEngine->CreateRenderNode(eERType_Brush));
		pBrush->DisablePhysicalization(true);
		pBrush->SetMaterial(m_material);
		pBrush->SetEntityStatObj(m_geom, &tm);
		pBrush->SetRndFlags(ERF_MOVES_EVERY_FRAME | ERF_NO_PHYSICS | ERF_CASTSHADOWMAPS | ERF_HAS_CASTSHADOWMAPS, true);


		//pBrush->SetRndFlags(ERF_NO_PHYSICS | ERF_CASTSHADOWMAPS | ERF_HAS_CASTSHADOWMAPS, true);

		reg.emplace<Render::RenderComponent>(newEnt, TRenderNodePtr(pBrush));
		reg.emplace<Transform::WorldTransform>(newEnt, tm);


		reg.emplace<Targeting::Targeting>(newEnt, towerRange);


		reg.emplace<Damage::Damage>(newEnt, m_damage);
		reg.emplace<Damage::TickedDamage>(newEnt, m_tickDamage.tickTotal, m_tickDamage.tickTotal);


		reg.emplace<Transform::Velocity	>(newEnt);
		reg.emplace<Transform::Acceleration	>(newEnt);

		reg.emplace<Transform::MinSpeed	>(newEnt, 0.f);
		reg.emplace<Transform::MaxSpeed	>(newEnt, 2.f);


		m_spawnedEntities.emplace_back(newEnt);
	}
}
#pragma optimize("",on)

auto CTowerSpawner::Despawn() -> void
{
	CGamePlugin::GetInstance()->GetRegistry().destroy(m_spawnedEntities.begin(), m_spawnedEntities.end());
	m_spawnedEntities.clear();
}

void CTowerSpawner::ProcessEvent(const SEntityEvent& event)
{
	if (event.event != EEntityEvent::Reset)
		return;


	if (event.nParam[0] == 1)
			Spawn();
	else
		Despawn();
}

static void RegisterTowerSpawnerComponent(Schematyc::IEnvRegistrar& registrar)
{
	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CTowerSpawner));
	}
}

CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterTowerSpawnerComponent);