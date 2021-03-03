#pragma once
#include <entt/entity/registry.hpp>

namespace Systems::Damage
{
	void Initialize(entt::registry& reg);
	void Update(entt::registry& reg, float dt);
}