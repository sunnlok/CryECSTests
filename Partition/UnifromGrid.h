#pragma once

#include <optional>
#include <tuple>
#include <vector>
#include <algorithm>
#include <execution>

namespace Utility::Partition
{
	inline auto PosToIndex(Vec3i pos, Vec3i bounds) {

		return pos.x + pos.y * bounds.y + pos.z * bounds.y * bounds.z;
	};


	class SortedUniformGrid {
		using TEntry = std::tuple<entt::entity, Vec3>;
		using TEntries = std::vector<TEntry>;
	public:


		void Clear() {
			m_sortedEntries.clear();
			
			std::fill(m_cells.begin(), m_cells.end(), std::nullopt);
		}





	protected:
		void Rebuild() {

			//auto sorter =  []()

			//std::sort(std::execution::par_unseq, m_sortedEntries.begin(), m_sortedEntries.end());
		}

	private:
		size_t	cellSize	= 1;
		size_t	size = 1;

		TEntries				m_sortedEntries;
		std::vector<std::optional<std::pair<size_t, size_t>>>	m_cells;
	};


	class UniformGrid
	{
		using TEntry = std::tuple<entt::entity, Vec3>;
	public:

		auto ResizeCells(Vec3i) -> void {

		};

		auto ClearEntries() -> void {

		};

	protected:

		Vec3i m_cells;
		Vec3  m_max;
		std::vector<std::vector<TEntry>> m_entries;
	};
}