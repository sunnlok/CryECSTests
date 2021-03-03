#include "StdAfx.h"
#include "TargetingSystem.h"
#include "CryECS/TypeContext.h"
#include <execution>


#include "BoidSystems.h"
#include "entt/Components/TargetingComponents.h"
#include "entt/Components/TransformComponents.h"
#include "PartitionSystem.h"
#include "CryRenderer/IRenderAuxGeom.h"
#include "Partition/HashGrid.h"

using namespace Components;

static std::vector<entt::entity> targetsToRemove;
static std::vector<std::pair<entt::entity,entt::entity>> targetsToAdd;

auto Systems::Targeting::InitSystems(entt::registry& reg) -> void
{
	reg.view<::Targeting::Targeting, ::Targeting::HasTarget>();

	targetsToRemove.reserve(2000);
	targetsToAdd.reserve(2000);
}
#pragma optimize("",off)
auto Systems::Targeting::UpdateTargeting(entt::registry& reg, float dt) -> void
{
	auto hasTargetGroup = reg.view<Transform::WorldTransform,::Targeting::Targeting, ::Targeting::HasTarget>();
	auto targetables = reg.view<Transform::WorldTransform, ::Targeting::Targetable>();
	{
		

		auto checkRanges = [&](entt::entity entity) {
			const auto& [targeting, hasTarget, targetingTransform] = hasTargetGroup.get<::Targeting::Targeting, ::Targeting::HasTarget, Transform::WorldTransform>(entity);

			entt::entity target = hasTarget.target;
			if (!reg.valid(target))
			{
				targetsToRemove.emplace_back(entity);
				return;
			}

			const Transform::WorldTransform& targetTm = targetables.get<Transform::WorldTransform>(target);

			auto pos = targetingTransform.tm.GetTranslation();
			auto targetPos = targetTm.tm.GetTranslation();

			if (pos.GetDistance(targetPos) > targeting.range)
			{
				targetsToRemove.emplace_back(entity);
				return;
			}
		};
		std::for_each(hasTargetGroup.begin(), hasTargetGroup.end(), checkRanges);
	}

	{
		auto findTargetGroup = reg.view<::Targeting::Targeting, Transform::WorldTransform>( entt::exclude<::Targeting::HasTarget>);

		auto findTarget = [&](entt::entity entity) {
			const auto& [targeting, targetingTransform] = findTargetGroup.get<::Targeting::Targeting, Transform::WorldTransform>(entity);


			entt::entity target = entt::null;
			auto pos = targetingTransform.tm.GetTranslation();

			auto& grid = Systems::Partition::PartitionSystem::Get()->GetGrid();

			auto forEntries = [&, &targetingData = targeting](const Utility::Partition::HashGrid::TEntry& entry) ->bool {
				auto [potentialTarget, targetPos] = entry;

				if (potentialTarget == entity)
					return false;

				if (targetables.find(potentialTarget) == targetables.end())
					return false;

				if (pos.GetDistance(targetPos) <= targetingData.range) {
					target = potentialTarget;
					return true;
				}

				return false;
			};

			AABB area(pos, targeting.range);
			grid.ForEachInArea(area, forEntries);

			if (target == entt::null)
				return;


			targetsToAdd.emplace_back(std::make_pair(entity, target));
		};
		std::for_each(findTargetGroup.begin(), findTargetGroup.end(), findTarget);
	}

	reg.remove<::Targeting::HasTarget>(targetsToRemove.begin(), targetsToRemove.end());
	targetsToRemove.clear();

	for (auto [entity, target] : targetsToAdd)
		reg.emplace<::Targeting::HasTarget>(entity, target);	

	targetsToAdd.clear();

	{
		auto targetsDebug = [&](entt::entity entity) {
			const auto& [transform, target] = hasTargetGroup.get<Transform::WorldTransform, ::Targeting::HasTarget>(entity);

			if (!reg.has<Transform::WorldTransform>(target.target))
				return;

			const auto& targetTransform = reg.get<Transform::WorldTransform>(target.target);

			gEnv->pAuxGeomRenderer->DrawLine(transform.tm.GetTranslation(), Col_Red, targetTransform.tm.GetTranslation(), Col_Red, 2.f);
		};
		std::for_each(hasTargetGroup.begin(), hasTargetGroup.end(), targetsDebug);
	}
	


	auto moveToTargetView = reg.view<Transform::WorldTransform, Transform::Velocity, Transform::Acceleration, Transform::MaxSpeed, ::Targeting::HasTarget>();


	auto moveToTarget = [&reg,&targetables, &moveToTargetView](entt::entity entity) -> void{
		auto [transform, vel, acc, maxSpeed, target] = moveToTargetView.get<
			Transform::WorldTransform,
			Transform::Velocity,
			Transform::Acceleration,
			Transform::MaxSpeed,
			::Targeting::HasTarget
		>(entity);

		if (!reg.valid(target.target))
			return;

		auto pTargetTransform = reg.try_get<Transform::WorldTransform>(target.target);
		if (!pTargetTransform)
			return;



		auto force = pTargetTransform->tm.GetTranslation() - transform.tm.GetTranslation();
		acc.val += Systems::Boids::AdjustSteeringForce(force, maxSpeed.speed, vel.velocity, 4);
	};

	std::for_each(std::execution::par_unseq, moveToTargetView.begin(), moveToTargetView.end(), moveToTarget);
}
#pragma optimize("",on)