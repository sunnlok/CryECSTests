#pragma once
#include <entt/entity/registry.hpp>


namespace Utility::Partition {
	class HashGrid;
}

namespace Systems::Partition {

	class PartitionSystem {
	public:
		PartitionSystem();
		~PartitionSystem();

		static PartitionSystem* Get();

		void Initialize(entt::registry& reg);
		void Update(entt::registry& reg, float dt);

		auto GetGrid() const -> const Utility::Partition::HashGrid& { return *m_hashGrid; }
	protected:
		std::unique_ptr<Utility::Partition::HashGrid> m_hashGrid;
	};

}