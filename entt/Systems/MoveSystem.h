#pragma once
#include <entt/entity/registry.hpp>

namespace Systems
{
	namespace Movement
	{
		auto InitMovement(entt::registry& registry) -> void;

		void BeginMovement(entt::registry& registry, float frameDelta);

		void UpdateMovement(entt::registry &registry, float frameDelta);
	}
}