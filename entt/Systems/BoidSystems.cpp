#include "StdAfx.h"
#include "BoidSystems.h"
#include "CryECS/TypeContext.h"
#include "Entt/Components/BoidComponents.h"
#include "Entt/Components/TransformComponents.h"
#include <numeric>
#include <execution>
#include "imgui.h"
#include "imconfig.h"
#include "PartitionSystem.h"
#include "CryECS/Components/Registration.h"
#include "Partition/HashGrid.h"


using namespace Components;

auto Systems::Boids::InitGroups(entt::registry& reg) -> void
{
	reg.view<::Boids::FlockMate, ::Boids::FlockWeights, ::Boids::FlockForceLimits, ::Boids::FlockSteering, ::Boids::FlockID>();
}

auto UpdateFLocking(entt::registry& reg, float dt) -> void
{

	{
		auto flockGroup = reg.view<Transform::WorldTransform, Boids::FlockMate, Boids::FlockParams, ::Boids::FlockID, Boids::Boid, Transform::Velocity>();

		auto flockUpdate = [&reg, &flockGroup, dt](entt::entity entity) {

			const auto& transform = flockGroup.get<Transform::WorldTransform>(entity).tm;
			const auto& params = flockGroup.get<Boids::FlockParams>(entity);
			const auto& flockID = flockGroup.get<::Boids::FlockID>(entity);

			auto pos = transform.GetTranslation();

			Boids::FlockMate newFlockData;
			//newFlockData.flockCenter = pos;

			auto& grid = Systems::Partition::PartitionSystem::Get()->GetGrid();

			auto forEntry = [&](const Utility::Partition::HashGrid::TEntry& entry) {
				auto [other, matePos] = entry;

				if (other == entity)
					return false;

				if (flockGroup.find(other) == flockGroup.end())
					return false;

				auto& otherID = flockGroup.get<::Boids::FlockID>(other);
				if (otherID.val == 0 || otherID.val != flockID.val)
					return false;

				float distanceSqr = pos.GetSquaredDistance(matePos);

				if (distanceSqr > params.viewRadius * params.viewRadius)
					return false;

				auto otherVel = flockGroup.get<Transform::Velocity>(other).velocity;

				Vec3 offset = matePos - pos;

				newFlockData.nearMateCount++;
				newFlockData.alignmentForce += otherVel;
				newFlockData.flockCenter += matePos;


				if (distanceSqr < params.avoidRadius * params.avoidRadius || distanceSqr)
					newFlockData.separationForce -= offset / distanceSqr;

				return false;
			};

			AABB area = { pos, params.viewRadius };
			grid.ForEachInArea(area, forEntry);

			auto& flockData = flockGroup.get<Boids::FlockMate>(entity);
			if (newFlockData.nearMateCount == 0)
			{
				flockData = newFlockData;
				return;
			}

			newFlockData.flockCenter /= (float)newFlockData.nearMateCount;
			newFlockData.cohesionForce = newFlockData.flockCenter - pos;

			newFlockData.alignmentForce /= (float)newFlockData.nearMateCount;
			newFlockData.separationForce /= (float)newFlockData.nearMateCount;

			flockData = newFlockData;
		};
		std::for_each(std::execution::par, flockGroup.begin(), flockGroup.end(), flockUpdate);
	}

	{
		//Adjust all the flock steering forces
		auto forceAdjustGroup = reg.view<Boids::FlockMate, Boids::FlockWeights, Boids::FlockForceLimits, Boids::FlockSteering, 
			Transform::Velocity, Transform::MaxSpeed , Transform::WorldTransform>();

		auto steeringUpdate = [&group = forceAdjustGroup](entt::entity entity)
		{
			const auto& [flock, weights, limits, velocity, maxSpeed] = group.get<
				Boids::FlockMate,
				Boids::FlockWeights,
				Boids::FlockForceLimits,
				Transform::Velocity,
				Transform::MaxSpeed
			>(entity);

			auto pos = group.get<Transform::WorldTransform>(entity).tm.GetTranslation();

			auto& flockSteering = group.get<Boids::FlockSteering>(entity);
			
			Vec3 newSteeringForce = ZERO;

			if (!flock.cohesionForce.IsZero()) {
				auto force = Systems::Boids::AdjustSteeringForce(
					flock.cohesionForce,
					maxSpeed.speed,
					velocity.velocity,
					limits.cohesion,
					weights.cohesionWeight);

				newSteeringForce += force;
			}
				

			if (!flock.alignmentForce.IsZero()) {
				auto force = Systems::Boids::AdjustSteeringForce(
					flock.alignmentForce,
					maxSpeed.speed,
					velocity.velocity,
					limits.alignment,
					weights.avgHeadingWeight);

				newSteeringForce += force;
			}
				

			if (!flock.separationForce.IsZero()) {
				auto force = Systems::Boids::AdjustSteeringForce(
					flock.separationForce,
					maxSpeed.speed,
					velocity.velocity,
					limits.separation,
					weights.separation);

				newSteeringForce += force;

			}
				

	

			flockSteering.force = newSteeringForce;

		};
		/*std::for_each(std::execution::par_unseq, forceAdjustGroup.begin(), forceAdjustGroup.end(), steeringUpdate);*/
		std::for_each(forceAdjustGroup.begin(), forceAdjustGroup.end(), steeringUpdate);
	}

	{
		//Update acceleration
		auto accelGroup = reg.view<Boids::FlockSteering, Transform::Acceleration>();

		auto accelUpdate = [&accelGroup](entt::entity entity) {
			const auto& steering = accelGroup.get<Boids::FlockSteering>(entity).force;
			auto& acceleration = accelGroup.get<Transform::Acceleration>(entity).val;

			acceleration += steering;

		};
		std::for_each(std::execution::par_unseq, accelGroup.begin(), accelGroup.end(), accelUpdate);

	}
}

auto Systems::Boids::UpdateBoidSystem(entt::registry& reg, float dt) -> void
{

	UpdateFLocking(reg, dt);
}
