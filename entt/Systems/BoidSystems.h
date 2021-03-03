#pragma once
#include <entt/entity/registry.hpp>

namespace Systems::Boids
{

	static auto AdjustSteeringForce(Vec3 unadjusted, float maxSpeed, const Vec3& velocity, float maxForce, float weight = 1.f) -> Vec3 {
		Vec3& steering = unadjusted;

		steering.SetLength(maxSpeed);
		steering -= velocity;
		steering.ClampLength(maxForce);
		steering *= weight;

		return steering;
	};


	auto UpdateBoidSystem(entt::registry& reg, float dt) -> void;

	auto InitGroups(entt::registry& reg) -> void;

}