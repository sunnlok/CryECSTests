#include "StdAfx.h"
#include "MoveSystem.h"
#include "CryECS/TypeContext.h"
#include "Entt/Components/TransformComponents.h"
#include <CrySystem/Profilers/ICryProfilingSystem.h>
#include "../Components/RenderComponents.h"
#include "CryGame/IGameFramework.h"
#include "CryEntitySystem/IEntitySystem.h"
#include <execution>
#include "BoidSystems.h"
#include "CryRenderer/IRenderAuxGeom.h"
#include <optional>
#include <algorithm>

using namespace Components;

auto IntersectAABB(const Vec3& begin, const Vec3& end, const AABB& aabb) -> std::optional<Vec3>
{

	return std::nullopt;
}

auto Systems::Movement::InitMovement(entt::registry& registry) -> void
{

	registry.view<Transform::WorldTransform, Transform::Position>();
	registry.view <Transform::Velocity, Transform::Acceleration>();
	registry.view<Transform::Velocity, Transform::Acceleration, Transform::MaxSpeed, Transform::MinSpeed>();
	
}

void Systems::Movement::BeginMovement(entt::registry& registry, float frameDelta)
{
	{
		auto accelerationInitGroup = registry.view<Transform::Acceleration>();
		auto accInit = [&accelerationInitGroup](entt::entity entity) {
			accelerationInitGroup.get<Transform::Acceleration>(entity).val = ZERO;
		};
		std::for_each(std::execution::par_unseq, accelerationInitGroup.begin(), accelerationInitGroup.end(), accInit);
	}
}

void Systems::Movement::UpdateMovement(entt::registry& registry, float frameDelta)
{
	
	
	//Apply constraints
	{
		auto constraintGroup = registry.view<Transform::WorldTransform, Transform::Velocity, Transform::Acceleration, Transform::AABBConstraint, Transform::MaxSpeed>();

		auto updateAabbConstraint = [&constraintGroup, frameDelta](entt::entity entity) {

			const auto& [tm, area] = constraintGroup.get<Transform::WorldTransform, Transform::AABBConstraint>(entity);
			auto pos = tm.tm.GetTranslation();

// 			if (area.area.IsContainPoint(pos))
// 				return;

			const auto& [vel, maxSpeed] = constraintGroup.get<Transform::Velocity, Transform::MaxSpeed>(entity);

			Vec3 target{ 0,0,0 };
			if (!area.area.IsContainPoint(pos))
			{
				target = area.area.GetCenter() - pos;
			}
			else
			{
				auto nextpos = pos + (vel.velocity);
				auto nextOffset = nextpos - pos;
				auto dir = nextOffset.GetNormalized();


				auto checkOffset = pos + (dir * 5);

				const Ray ray{ checkOffset ,  -dir * 5 };
				Vec3 hitPos;
				auto result = Intersect::Ray_AABB(ray, area.area, hitPos);
				if (result != 0x01)
					return;

				target = -dir;
			}
			
		

			/*gEnv->pAuxGeomRenderer->DrawSphere(hitPos, 0.2, Col_Red);

			auto offset = hitPos - pos;
			offset /= pos.GetSquaredDistance(hitPos);*/

			auto& acceleration = constraintGroup.get<Transform::Acceleration>(entity).val;

			//Vec3 force =  area.area.GetCenter() - pos;
			acceleration += Systems::Boids::AdjustSteeringForce(target, maxSpeed.speed, vel.velocity, area.maxForce, area.weight);
		};
		//std::for_each(std::execution::par_unseq, constraintGroup.begin(), constraintGroup.end(), updateAabbConstraint);
		std::for_each( constraintGroup.begin(), constraintGroup.end(), updateAabbConstraint);
	}


	{
		auto accToVelGroup = registry.view<Transform::Velocity, Transform::Acceleration, Transform::MaxSpeed, Transform::MinSpeed>();
		auto applyAcc = [&accToVelGroup, frameDelta](entt::entity entity) {
			const auto& [acc, minSpeed, maxSpeed] = accToVelGroup.get<Transform::Acceleration, Transform::MinSpeed, Transform::MaxSpeed>(entity);
			auto& vel = accToVelGroup.get<Transform::Velocity>(entity).velocity;

			vel += acc.val * frameDelta;
			//Clamp to min/max speed	
			auto curSpeed = vel.GetLength();
			if (curSpeed == 0)
				return;

			auto dir = vel / curSpeed;

			auto clampedSpeed = std::clamp(curSpeed, minSpeed.speed, maxSpeed.speed);

			vel = dir * clampedSpeed;
		};

		std::for_each(std::execution::par_unseq, accToVelGroup.begin(), accToVelGroup.end(), applyAcc);
	}

	{
		auto moveView = registry.view<Transform::WorldTransform, Transform::Direction, Transform::MaxSpeed>();
		moveView.each([frameDelta](auto ent, Transform::WorldTransform& transform, Transform::Direction& dir, Transform::MaxSpeed& speed) {

			auto frameOffset = dir.dir * speed.speed * frameDelta;

			transform.tm.AddTranslation(frameOffset);
		});
	}

	{
		auto velocityMoveGroup = registry.view<Transform::WorldTransform, Transform::Velocity>();

		/*auto debug = [&velocityMoveGroup, frameDelta](entt::entity entity) {
			auto& tm = velocityMoveGroup.get<WorldTransform>(entity).tm;
			const auto& vel = velocityMoveGroup.get<Velocity>(entity).velocity;

			gEnv->pAuxGeomRenderer->DrawSphere(tm.GetTranslation() + vel * frameDelta, 0.5f, Col_Red);
		};
		std::for_each(velocityMoveGroup.begin(), velocityMoveGroup.end(), debug);*/

		auto update = [&velocityMoveGroup, frameDelta](entt::entity entity)	{
			auto& tm = velocityMoveGroup.get<Transform::WorldTransform>(entity).tm;
			const auto& vel = velocityMoveGroup.get<Transform::Velocity>(entity).velocity;

			tm.AddTranslation(vel * frameDelta);
		};

		auto rotateView = registry.view<Transform::WorldTransform, Transform::Velocity>();
		rotateView.each([frameDelta](auto ent, Transform::WorldTransform& transform, Transform::Velocity& vel) {

			if (vel.velocity.IsZero())
				return;


			transform.tm.SetRotation33(Matrix33::CreateRotationVDir(vel.velocity.GetNormalized()));
		});

		std::for_each(std::execution::par_unseq,velocityMoveGroup.begin(), velocityMoveGroup.end(), update);
	}


	{
		auto geomUpdateView = registry.view<Transform::WorldTransform, Render::RenderComponent>();

		auto updateGeom = [&](entt::entity ent) {
			const auto& [trans, render] = geomUpdateView.get< Transform::WorldTransform, Render::RenderComponent>(ent);

			render.renderNode->SetMatrix(trans.tm);
		};

		std::for_each(geomUpdateView.begin(), geomUpdateView.end(), updateGeom);
	}
	
}

