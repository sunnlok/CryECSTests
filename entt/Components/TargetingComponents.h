#pragma once
#include <entt/entity/entity.hpp>

namespace Components::Targeting
{
	struct Targeting {
		float range = 1.f;

		void Serialize(Serialization::IArchive& ar)
		{
			ar(range, "range", "Range");
		}
	};


	struct HasTarget {
		entt::entity target = entt::null;
	};

	struct Targetable {

	};
}
