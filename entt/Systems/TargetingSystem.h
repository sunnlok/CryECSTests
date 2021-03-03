#pragma once
#include <entt/entity/registry.hpp>


namespace Systems::Targeting {

	auto InitSystems(entt::registry& reg) -> void;

	auto UpdateTargeting(entt::registry& reg, float dt) -> void;

}