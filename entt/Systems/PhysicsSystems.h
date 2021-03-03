#pragma once
#include <entt/entity/registry.hpp>

namespace Systems::Physics
{
	auto InitSystems(entt::registry& reg) -> void;

	auto UpdatePhysicsEarly(entt::registry& reg, float dt) -> void;
	auto UpdatePhysics(entt::registry& reg, float dt) -> void;
}