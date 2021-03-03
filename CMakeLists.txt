cmake_minimum_required (VERSION 3.14)

start_sources()

sources_platform(ALL)
add_sources("Code_uber.cpp"
	PROJECTS Game
	SOURCE_GROUP "Root"
		"GamePlugin.h"
		"GamePlugin.cpp"
		"StdAfx.cpp"
		"StdAfx.h"
)
add_sources("Components_uber.cpp"
	PROJECTS Game
	SOURCE_GROUP "Components"
		"Components/Player.cpp"
		"Components/SpawnPoint.cpp"
		"Components/EnttProxy.cpp"
		"Components/EnttSpawner.cpp"
		"Components/TowerSpawner.cpp"
		"Components/TargetSpawner.cpp"
		"Components/Bullet.h"
		"Components/Player.h"
		"Components/SpawnPoint.h"
		"Components/EnttProxy.h"
		"Components/EnttSpawner.h"
		"Components/TowerSpawner.h"
		"Components/TargetSpawner.h"
		"Components/MoveDirComponent.h"
)


add_sources("Entt_Components_uber.cpp"
 PROJECTS Game
	SOURCE_GROUP "Entt\\\\Components"
		"Entt/Components/BridgeComponents.h"
		"Entt/Components/TransformComponents.h"
		"Entt/Components/RenderComponents.h"
		"Entt/Components/DebugComponents.h"
		"Entt/Components/BoidComponents.h"
		"Entt/Components/PhysicsComponents.h"
		"Entt/Components/DamageComponents.h"
		"Entt/Components/TargetingComponents.h"
   )

add_sources("Utility_uber.cpp"
 PROJECTS Game
 SOURCE_GROUP "Entt\\\\Utility"
	"Partition/HashGrid.h"
	"Partition/UnifromGrid.h"
)

add_sources("Entt_Systems_uber.cpp"
 PROJECTS Game
	SOURCE_GROUP "Entt\\\\Systems"
		"Entt/Systems/MoveSystem.h"
		"Entt/Systems/MoveSystem.cpp"
		"Entt/Systems/DebugSystem.h"
		"Entt/Systems/DebugSystem.cpp"
		"Entt/Systems/BoidSystems.h"
		"Entt/Systems/BoidSystems.cpp"
		"Entt/Systems/PhysicsSystems.h"
		"Entt/Systems/PhysicsSystems.cpp"
		"Entt/Systems/DamageSystem.h"
		"Entt/Systems/DamageSystem.cpp"
		"Entt/Systems/TargetingSystem.h"
		"Entt/Systems/TargetingSystem.cpp"
		"Entt/Systems/PartitionSystem.h"
		"Entt/Systems/PartitionSystem.cpp"
   )

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CVarOverrides.h")
	add_sources("NoUberFile"
		PROJECTS Game
		SOURCE_GROUP "Root"
			"CVarOverrides.h"
	)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/EngineDefineOverrides.h")
	add_sources("NoUberFile"
		PROJECTS Game
		SOURCE_GROUP "Root"
			"EngineDefineOverrides.h"
	)
endif()
end_sources()

CryGameModule(GameFirstPerson FORCE_SHARED PCH "StdAfx.cpp" SOLUTION_FOLDER "Project")

target_include_directories(${THIS_PROJECT}
PRIVATE
	"${CRYENGINE_DIR}/Code/CryEngine/CryCommon"
	"${CRYENGINE_DIR}/Code/CryEngine/CryAction"
	"${CRYENGINE_DIR}/Code/CryEngine/CrySchematyc/Core/Interface"
	"${CRYENGINE_DIR}/Code/CryPlugins/CryDefaultEntities/Module"
	"${CRYENGINE_DIR}/Code/CryPlugins/CryImgui/Module/Imgui"
)
target_link_libraries( ${THIS_PROJECT} PRIVATE CryImgui CryDefaultEntities CryECS)
#Enable cpp 17 support
set_target_properties(${THIS_PROJECT} PROPERTIES CXX_STANDARD 17)