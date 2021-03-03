#include "StdAfx.h"
#include "EnttSpawner.h"

#include "GamePlugin.h"
#include "MoveDirComponent.h"
#include "CryCore/StaticInstanceList.h"
#include "CryRenderer/IRenderAuxGeom.h"
#include "CrySchematyc/Env/IEnvRegistrar.h"
#include "CrySchematyc/Env/Elements/EnvComponent.h"
#include "CrySerialization/Decorators/Resources.h"
#include "entt/Components/RenderComponents.h"
#include "entt/Components/TargetingComponents.h"
#include "entt/Components/TransformComponents.h"

CEnttSpawner::CEnttSpawner()
{

}

CEnttSpawner::~CEnttSpawner()
{

}

auto CEnttSpawner::Initialize() -> void
{
	if (!gEnv->IsEditor())
	{
		Spawn();
	}
}

void CEnttSpawner::Serialize(Serialization::IArchive& archive)
{
	archive(m_bEnabled, "Enabled", "Enabled");
	archive(m_spawnCount, "Count", "Count");
	
	archive(flockID, "flockParams", "Flock Parameters");
	archive(flockParams, "flockParams", "Flock Parameters");
	archive(flockWeights, "flockWeights", "FlockWeights");
	archive(forceLimits, "forceLimits", "Flock Force Limits");
	archive(minSpeed, "minSpeed", "Min Speed");
	archive(maxSpeed, "maxSpeed", "Max Speed");
	archive(areaConstraint, "constraint", "Constraint");
	archive(hitAvoidance, "hitAvoidance", "Hit Avoidance");

	archive(maxHealth, "health", "Health");
	archive(damage, "damage", "Damage");

	archive(bIsTarget, "isTarget", "Is Target");
	archive(bCanTarget, "canTarget", "Can Target");
	archive(targeting, "targeting", "Targeting Parameters");
	archive(tickDamage, "tickDamage", "Tick Damage");

	auto pMaterial = m_material.get();

	{
		using Serialization::MaterialPicker;
		string matName = pMaterial ? pMaterial->GetName() : "";
		string oldName = matName;

		archive(Serialization::MaterialPicker(matName), "Material", "Material");

		if (matName != oldName)
			m_material = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial(matName, false);
	}

	{

		string modelName = m_geom ? m_geom->GetFilePath() : "";
		string oldName = modelName;

		archive(Serialization::StaticModelFilename(modelName), "Mesh", "Mesh");

		if (modelName != oldName)
			m_geom = gEnv->p3DEngine->LoadStatObj(modelName);
	}
}

void CEnttSpawner::SerializeProperties(Serialization::IArchive& archive)
{
	Serialize(archive);
}

void CEnttSpawner::Render(const IEntity& entity, const IEntityComponent& component, SEntityPreviewContext& context) const
{
	//gEnv->pAuxGeomRenderer->DrawSphere(GetEntity()->GetPos(),  radius, {0,255,0}, false);
	AABB aabb = areaConstraint.area;
	gEnv->pAuxGeomRenderer->DrawAABB(aabb, GetEntity()->GetWorldTM(), false, Col_Green, EBoundingBoxDrawStyle::eBBD_Faceted);

}

Cry::Entity::EventFlags CEnttSpawner::GetEventMask() const
{
	return EEntityEvent::Reset;
}

auto CEnttSpawner::Spawn() -> void
{
	using namespace Components;

	if (!m_bEnabled || !m_geom || !m_material)
		return;

	Despawn();

	auto& reg = CGamePlugin::GetInstance()->GetRegistry();

	auto& rng = gEnv->pSystem->GetRandomGenerator();

	auto& transform = GetEntity()->GetWorldTM();

	auto movedArea = areaConstraint;
	movedArea.area = AABB(transform.GetTranslation(), areaConstraint.radius);

	for (uint32 i = 1; i <= m_spawnCount; ++i)
	{
		auto newEnt = reg.create();

		auto spawnPos = rng.GetRandomComponentwise(movedArea.area.min, movedArea.area.max);
		auto dir = rng.GetRandomUnitVector<Vec3>();
		auto tm = Matrix34::Create(Vec3(0.1f, 0.1f, 0.1f), Quat::CreateRotationVDir(dir), spawnPos);
		auto speed = rng.GetRandom(minSpeed.speed, maxSpeed.speed);

		IBrush* pBrush = static_cast<IBrush*>(gEnv->p3DEngine->CreateRenderNode(eERType_MovableBrush));
		pBrush->DisablePhysicalization(true);
		pBrush->SetMaterial(m_material);
		pBrush->SetEntityStatObj(m_geom, &tm);
		pBrush->SetRndFlags(ERF_MOVES_EVERY_FRAME | ERF_NO_PHYSICS | ERF_CASTSHADOWMAPS | ERF_HAS_CASTSHADOWMAPS, true);

		reg.emplace<Render::RenderComponent>(newEnt, TRenderNodePtr(pBrush));

		reg.emplace<Transform::WorldTransform>(newEnt, tm);
		reg.emplace<Transform::Velocity	>(newEnt, dir * speed);
		reg.emplace<Transform::Acceleration	>(newEnt);

		reg.emplace<Boids::Boid>(newEnt);
		reg.emplace<Boids::FlockMate>(newEnt);
		reg.emplace<Boids::FlockID>(newEnt, flockID);
		reg.emplace<Boids::FlockSteering>(newEnt);
		reg.emplace<Boids::FlockParams>(newEnt, flockParams);
		reg.emplace<Boids::FlockWeights>(newEnt, flockWeights);
		reg.emplace<Boids::FlockForceLimits>(newEnt, forceLimits);


		reg.emplace<Transform::MinSpeed	>(newEnt, minSpeed);
		reg.emplace<Transform::MaxSpeed	>(newEnt, maxSpeed);
		reg.emplace<Transform::AABBConstraint>(newEnt,  movedArea);

		reg.emplace<Physics::RayHitAvoidanceReflected>(newEnt, Components::Physics::RayHitAvoidanceReflected{hitAvoidance.weight, hitAvoidance.force});
		//reg.emplace<Physics::Components::RayCaster>(newEnt, Vec3(0.f, 2.f, 0.f));

		std::array<Vec3, 4>  dirs = {
			Vec3{ 0.f,0.25f,0.25f }.GetNormalized(),
			Vec3{ 0.f,0.25f,-0.25f }.GetNormalized(),
			Vec3{ -0.25f,0.25f,0 }.GetNormalized(),
			Vec3{ 0.25f,0.25f,0 }.GetNormalized()
		};

		reg.emplace<Components::Physics::MultiRayCaster>(newEnt, dirs);

		reg.emplace<Components::Damage::MaxHealth>(newEnt, maxHealth);
		reg.emplace<Components::Damage::Health>(newEnt, maxHealth.val);
		reg.emplace<Components::Damage::Damage>(newEnt, damage);

		if (bCanTarget)
		{
			reg.emplace<Components::Targeting::Targeting>(newEnt, targeting);
			reg.emplace<Components::Damage::TickedDamage>(newEnt, tickDamage);
		}
		if (bIsTarget)
		{
			reg.emplace<Components::Targeting::Targetable>(newEnt);
			reg.emplace<Components::Damage::RemoveIfDead>(newEnt);
		}

		m_spawnedEntities.emplace_back(newEnt);
	}
}

auto CEnttSpawner::SpawnCE() -> void
{

}

auto CEnttSpawner::Despawn() -> void
{

}

void CEnttSpawner::ProcessEvent(const SEntityEvent& event)
{
	if (event.event != EEntityEvent::Reset)
		return;


	if (event.nParam[0] == 1)
		if (m_bUseEntt)
			Spawn();
		else
			SpawnCE();
	else
		Despawn();
}

static void RegisterEnttSpawnerComponent(Schematyc::IEnvRegistrar& registrar)
{
	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CEnttSpawner));
	}
}

CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterEnttSpawnerComponent);

static void RegisterDirMoverComponent(Schematyc::IEnvRegistrar& registrar)
{
	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CDirMover));
	}
}

CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterDirMoverComponent);