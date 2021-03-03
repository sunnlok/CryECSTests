#include "StdAfx.h"
#include "PartitionSystem.h"
#include "CryECS/TypeContext.h"
#include "Partition/HashGrid.h"
#include "Partition/UnifromGrid.h"
#include "../Components/TransformComponents.h"

using namespace Systems::Partition;


static Systems::Partition::PartitionSystem* g_pPartitionSystem = nullptr;

Systems::Partition::PartitionSystem* Systems::Partition::PartitionSystem::Get()
{
	return g_pPartitionSystem;
}

Systems::Partition::PartitionSystem::~PartitionSystem()
{
	g_pPartitionSystem = nullptr;
}

PartitionSystem::PartitionSystem()
	: m_hashGrid(std::make_unique<Utility::Partition::HashGrid>())
{
	g_pPartitionSystem = this;
}


void PartitionSystem::Initialize(entt::registry& reg)
{

}

void PartitionSystem::Update(entt::registry& reg, float dt)
{

	m_hashGrid->Clear();



	auto transforms = reg.view<Components::Transform::WorldTransform>();

	transforms.each([&](auto entity, auto& transform) {

		m_hashGrid->TryInsertAt(entity, transform.tm.GetTranslation());

	});


	/*auto debugGrid = [&](uint64 key, Utility::Partition::HashGrid::TEntryList& entries) {

		auto [xu, yu, zu] = m_hashGrid->KeyToCell(key);

		Vec3 min = { (float)xu, (float)yu, (float)zu };
		Vec3 max = min + Vec3(1.f);

		AABB aabb(min, max);

		gEnv->pAuxGeomRenderer->DrawAABB(aabb, false, Col_Green, EBoundingBoxDrawStyle::eBBD_Faceted);
	};

	m_hashGrid->ForEachCell(debugGrid);*/
}



