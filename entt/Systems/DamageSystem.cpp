#include "StdAfx.h"
#include "DamageSystem.h"
#include "CryECS/TypeContext.h"
#include "entt/Components/DamageComponents.h"
#include "entt/Components/TargetingComponents.h"
#include "Cry3DEngine/I3DEngine.h"
#include "CryParticleSystem/IParticles.h"
#include "CryRenderer/IRenderAuxGeom.h"
#include "entt/Components/TransformComponents.h"

using namespace Components;



void ResetTickDamage(entt::registry& reg, entt::entity entity) {

	auto pTickedDamage = reg.try_get<::Damage::TickedDamage>(entity);
	if (!pTickedDamage)
		return;

	pTickedDamage->tickRemaining = pTickedDamage->tickTotal;

	reg.emplace_or_replace<::Damage::DoTickDamage>(entity);
}

void Systems::Damage::Initialize(entt::registry& reg)
{

	reg.on_construct<::Targeting::HasTarget>().connect<ResetTickDamage>();
	reg.on_destroy<::Targeting::HasTarget>().connect<&entt::registry::remove_if_exists<::Damage::DoTickDamage>>();
}


static std::vector<entt::entity> g_damagesToDo;
static std::vector<entt::entity> g_entitiesToKill;

void Systems::Damage::Update(entt::registry& reg, float dt)
{
	auto damagables = reg.view<::Damage::Health, ::Damage::MaxHealth>(entt::exclude<::Damage::Dead>);

	//Evaluate damage ticks. Can probably generalize this into timers
	auto tickedDamagers = reg.view< ::Damage::TickedDamage, ::Damage::Damage, ::Damage::DoTickDamage>();

	tickedDamagers.each([&](entt::entity entity, ::Damage::TickedDamage& ticker, ::Damage::Damage& d) -> void {

		ticker.tickRemaining -= std::min(ticker.tickRemaining, dt);

		if (ticker.tickRemaining > 0)
			return;

		g_damagesToDo.emplace_back(entity);
		ticker.tickRemaining = ticker.tickTotal;

	});

	reg.insert<::Damage::DoDamage>(g_damagesToDo.begin(), g_damagesToDo.end());
	g_damagesToDo.clear();

	//Do targeted damage
	auto damageDealers = reg.view<::Damage::Damage, ::Targeting::HasTarget, ::Damage::DoDamage>();

	damageDealers.each([&](entt::entity entity,::Damage::Damage& damage, ::Targeting::HasTarget& target) -> void {


		if (damagables.find(target.target) == damagables.end())
			return;

		auto& targetHealth = damagables.get<::Damage::Health>(target.target);

		targetHealth.val -= std::min(damage.val, targetHealth.val);
	});


	//Clear all damage
	reg.clear<::Damage::DoDamage>();

	//Death check
	damagables.each([&](entt::entity entity, ::Damage::Health& health, ::Damage::MaxHealth& maxHealth)  -> void {

		if (health.val)
			return;

		g_entitiesToKill.emplace_back(entity);
	});

	reg.insert<::Damage::Dead>(g_entitiesToKill.begin(), g_entitiesToKill.end());
	g_entitiesToKill.clear();

	reg.view<Components::Transform::WorldTransform, ::Damage::Dead>().each([&](auto entity, Components::Transform::WorldTransform& transform) {
		gEnv->pAuxGeomRenderer->DrawSphere(transform.tm.GetTranslation(), 1.5f, Col_Red);
	});

	//Remove dead entities
	auto deadToRemove = reg.view<::Damage::Dead, ::Damage::RemoveIfDead>();

	std::vector<entt::entity> test{ deadToRemove.begin(), deadToRemove.end() };
	

	reg.destroy(test.begin(), test.end());
}
