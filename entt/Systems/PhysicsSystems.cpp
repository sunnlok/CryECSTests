#include "StdAfx.h"
#include "CryECS/TypeContext.h"
#include "PhysicsSystems.h"
#include "Entt/Components/PhysicsComponents.h"
#include "Entt/Components/TransformComponents.h"
#include "CryPhysics/physinterface.h"
#include <entt/entity/view_pack.hpp>

#include "BoidSystems.h"
#include "CryRenderer/IRenderAuxGeom.h"

using namespace Components;

auto Systems::Physics::InitSystems(entt::registry& reg) -> void
{
	
	reg.prepare<::Transform::Acceleration>();
	reg.prepare<::Physics::RayHitAvoidanceReflected>();
}

auto Systems::Physics::UpdatePhysicsEarly(entt::registry& reg, float dt) -> void
{

	//Reset all our ray hits
	auto hitGroup = reg.view<::Physics::RayHit>();

	reg.remove<::Physics::RayHit>(hitGroup.begin(), hitGroup.end());
}


auto Systems::Physics::UpdatePhysics(entt::registry& reg, float dt) -> void
{

	//Run the raycasts
	auto castGroup = reg.view<::Physics::RayCaster, Transform::WorldTransform>();
	castGroup.each([&reg, dt](entt::entity entity, ::Physics::RayCaster& caster, Transform::WorldTransform& tm) {
			
		auto adjustedDir = tm.tm.TransformVector(caster.castDir);
		auto pos = tm.tm.GetTranslation();

		ray_hit hit;
		
		auto hitCount = gEnv->pPhysicalWorld->RayWorldIntersection(
			pos, adjustedDir,
			ent_all, rwi_colltype_any | rwi_ignore_noncolliding | rwi_stop_at_pierceable | rwi_ignore_back_faces,
			&hit,1
			);

		if (!hitCount)
			return;

		reg.emplace_or_replace<::Physics::RayHit>(entity, hit.pt, hit.n);
		
	});

	{
		/*auto castView = reg.view<MultiRayCaster, WorldTransform>();
		castView.each([](MultiRayCaster& caster, WorldTransform& tm)
		{
			auto pos = tm.tm.GetTranslation();
			for (auto& ray : caster.castDirs)
			{
				auto dir = tm.tm.TransformVector(ray);

				gEnv->pAuxGeomRenderer->DrawLine(pos, Col_Blue, pos + dir, Col_Blue);
			}
			
		}
		);*/

	}

	auto multiCastGroup = reg.view<::Physics::MultiRayCaster, Transform::WorldTransform>();
	multiCastGroup.each([&reg, dt](entt::entity entity, ::Physics::MultiRayCaster& caster, Transform::WorldTransform& tm) {

		auto pos = tm.tm.GetTranslation();

		ray_hit hit;
		for (auto& dir : caster.castDirs)
		{
			auto adjustedDir = tm.tm.TransformVector(dir);

			auto hitCount = gEnv->pPhysicalWorld->RayWorldIntersection(
				pos, adjustedDir,
				ent_all, rwi_colltype_any | rwi_ignore_noncolliding | rwi_stop_at_pierceable | rwi_ignore_back_faces,
				&hit, 1
			);

			if (hitCount) {
				reg.emplace_or_replace<::Physics::RayHit>(entity, hit.pt, hit.n);
				break;
			}
		}		
	});

	{
		//auto hitView = reg.view<RayHit>();
		//hitView.each([](const RayHit& hit)
		//{
		//	gEnv->pAuxGeomRenderer->DrawSphere(hit.pos, 0.2, Col_Red);
		//	gEnv->pAuxGeomRenderer->DrawLine(hit.pos, Col_Red, hit.pos + hit.normal, Col_Red);
		//}
		//);

	}

	{
		auto avoidGroup = (reg.view<Transform::WorldTransform>() | reg.view<::Physics::RayHit, ::Physics::RayHitAvoidance, Transform::Velocity, Transform::Acceleration, Transform::MaxSpeed>());
		auto avoidUpdate = [&avoidGroup, dt](entt::entity entity) {
			const auto& [hit, avoid, vel, speed, tm] = avoidGroup.get<::Physics::RayHit, ::Physics::RayHitAvoidance, Transform::Velocity, Transform::MaxSpeed, Transform::WorldTransform>(entity);
			auto& acc = avoidGroup.get<Transform::Acceleration>(entity);

			auto pos = tm.tm.GetTranslation();

			auto hitposAdjusted = (hit.pos + hit.normal) * 0.5;

			auto reflected = Vec3::CreateReflection(hitposAdjusted - pos, hit.normal);

			acc.val += Systems::Boids::AdjustSteeringForce(-reflected, speed.speed, vel.velocity, avoid.force, avoid.weight);

		};
		//std::for_each(std::execution::par_unseq, avoidGroup.begin(), avoidGroup.end(), avoidUpdate);
		std::for_each(avoidGroup.begin(), avoidGroup.end(), avoidUpdate);
	}

	{
		auto avoidGroup = reg.view<::Physics::RayHit, ::Physics::RayHitAvoidanceReflected, Transform::Velocity, Transform::Acceleration, Transform::MaxSpeed, Transform::WorldTransform>();
		auto avoidUpdate = [&avoidGroup, dt](entt::entity entity) {
			const auto& [hit, avoid, vel, speed, tm] = avoidGroup.get<::Physics::RayHit, ::Physics::RayHitAvoidanceReflected, Transform::Velocity, Transform::MaxSpeed, Transform::WorldTransform>(entity);
			auto& acc = avoidGroup.get<Transform::Acceleration>(entity);

			auto pos = tm.tm.GetTranslation();

			auto reflected = Vec3::CreateReflection(-(hit.pos - pos), hit.normal).GetNormalized();

			acc.val += Systems::Boids::AdjustSteeringForce(reflected, speed.speed, vel.velocity, avoid.force, avoid.weight);

			gEnv->pAuxGeomRenderer->DrawLine(pos, Col_Red, pos + reflected, Col_Red);
		};
		//std::for_each(std::execution::par_unseq, avoidGroup.begin(), avoidGroup.end(), avoidUpdate);
		std::for_each(avoidGroup.begin(), avoidGroup.end(), avoidUpdate);
	}
	
}



