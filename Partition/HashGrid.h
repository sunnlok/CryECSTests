#pragma once
#include <entt/fwd.hpp>

namespace Utility::Partition {


	

	class HashGrid {
	public:
		using TEntry = std::tuple<entt::entity, Vec3>;
		using TEntryList = std::vector<TEntry>;

		constexpr uint64 CellToKey(uint16 x, uint16 y, uint16 z) const{

			uint64 hash = x;
			hash = hash << 16;
			hash |= (uint64)y;
			hash = hash << 16;
			hash |= (uint64)z;

			return hash;
		};

		constexpr auto KeyToCell(uint64 cellKey) const -> std::tuple<uint16, uint16, uint16> {
			uint16 z = (uint16)cellKey;
			cellKey = cellKey >> 16;
			uint16 y = (uint16)cellKey;
			cellKey = cellKey >> 16;
			uint16 x = (uint16)cellKey;

			return { x, y, z };
		};
	

		void Clear() {
			m_hashGrid.clear();
		}

		bool TryInsertAt(entt::entity entity, Vec3 pos) {
			AABB gridBounds(ZERO, bounds);

			if (!gridBounds.IsContainPoint(pos))
				return false;

			auto [x, y, z] = Vec3_tpl<uint16>((uint16)floor(pos.x), (uint16)floor(pos.y), (uint16)floor(pos.z));
			uint64 key = CellToKey((uint16)x, (uint16)y, (uint16)z);
			m_hashGrid[key].emplace_back(std::make_tuple(entity, pos));

			return true;
		}


		template<typename TCallback>
		void ForEachInArea(AABB area, TCallback&& calback) const {

			area = {
				{ floor(area.min.x), floor(area.min.y), floor(area.min.z) },
				{ ceil(area.max.x), ceil(area.max.y), ceil(area.max.z) }
			};
	
			Vec3i min = { (int)area.min.x, (int)area.min.y, (int)area.min.z };
			Vec3i max = { (int)area.max.x, (int)area.max.y, (int)area.max.z };

			for (int z = min.z; z <= max.z; ++z)
			{
				if (z < 0)
					continue;
				else if(z > bounds.z)
					break;

				for (int y = min.y; y <= max.y; ++y)
				{
					if (y < 0)
						continue;
					else if (y > bounds.y)
						break;

					for (int x = min.x; x <= max.x; ++x)
					{
						if (x < 0)
							continue;
						else if (x > bounds.x)
							break;

						
						uint64 key = CellToKey((uint16)x, (uint16)y, (uint16)z);
						auto entries = m_hashGrid.find(key);
						if(entries == m_hashGrid.end())
							continue;

						for (auto& entry : entries->second)
							if (calback(entry) == true)
								return;
					}
				}

			}
		}

		template<typename TCallback>
		void ForEachEntry(TCallback&& callback) const {
			for (auto& cell : m_hashGrid)
			{
				for (auto& entry : cell->second)
				{
					callback(entry);
				}
			}
		};

		template<typename TCallback>
		void ForEachCell(TCallback&& callback)
		{
			for (auto& cell : m_hashGrid)
			{
				callback(cell.first, cell.second);
			}
		}


	protected:

		Vec3 cellWidth = { 1, 1, 1 };
		Vec3 bounds = { 1024, 1024, 200 };


		std::unordered_map<uint64, std::vector<TEntry>> m_hashGrid;
	};	

}
