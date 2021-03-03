#pragma once
#include <entt/entity/registry.hpp>

namespace Systems::Debug
{
	void UpdateTransformDebug(entt::registry& registry, float frameDelta);
}