#include "StdAfx.h"
#include "DebugSystem.h"
#include "CryECS/TypeContext.h"
#include "Entt/Components/DebugComponents.h"
#include "Entt/Components/TransformComponents.h"
#include "CryRenderer/IRenderAuxGeom.h"

using namespace Components;


void Systems::Debug::UpdateTransformDebug(entt::registry& registry, float frameDelta)
{


	registry.view<Transform::WorldTransform, ::Debug::DebugTransform>().each([frameDelta](auto entt, Transform::WorldTransform& transform)
	{
		gEnv->pAuxGeomRenderer->DrawSphere(transform.tm.GetTranslation(), 1.f, Col_Blue);
	});
}

