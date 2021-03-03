#include "StdAfx.h"
#include "GamePlugin.h"
#include <CryCore/Platform/platform_impl.inl>

#include "Components/Player.h"

#include <CrySchematyc/Env/IEnvRegistry.h>
#include <CrySchematyc/Env/EnvPackage.h>
#include <CrySchematyc/Utils/SharedString.h>

#include <IGameObjectSystem.h>
#include <IGameObject.h>

// Included only once per DLL module.

#include "entt/Systems/MoveSystem.h"
#include "entt/Systems/DebugSystem.h"
#include "entt/Systems/BoidSystems.h"
#include "entt/Systems/PhysicsSystems.h"
#include "entt/Systems/TargetingSystem.h"
#include "entt/Systems/DamageSystem.h"

#include "CryECS/IRegistryManager.h"
#include "CryECS/Components/Registration.h"


CGamePlugin::CGamePlugin()
{
	auto ecs = Cry::ECS::Get();

	Cry::ECS::Components::PerModuleRegistrar registrar;
	ecs->RegisterModuleComponents(registrar);
}

CGamePlugin::~CGamePlugin()
{
	// Remove any registered listeners before 'this' becomes invalid
	if (gEnv->pGameFramework != nullptr)
	{
		gEnv->pGameFramework->RemoveNetworkedClientListener(*this);
	}

	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);

	if (gEnv->pSchematyc)
	{
		gEnv->pSchematyc->GetEnvRegistry().DeregisterPackage(CGamePlugin::GetCID());
	}
}

bool CGamePlugin::Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams)
{
	// Register for engine system events, in our case we need ESYSTEM_EVENT_GAME_POST_INIT to load the map
	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this, "CGamePlugin");

	using namespace entt::literals;
	m_pRegistry = Cry::ECS::Get()->GetRegistryManager()->CreateRegistry("Game"_hs);

	m_pPartitionSystem = std::make_unique<Systems::Partition::PartitionSystem>();

	EnableUpdate(IEnginePlugin::EUpdateStep::MainUpdate, true);

	auto& registry = GetRegistry();

	m_pPartitionSystem->Initialize(registry);
	Systems::Movement::InitMovement(registry);
	Systems::Physics::InitSystems(registry);
	Systems::Boids::InitGroups(registry);
	Systems::Targeting::InitSystems(registry);
	Systems::Damage::Initialize(registry);

	return true;
}

void CGamePlugin::MainUpdate(float frameTime)
{
	auto& registry = GetRegistry();
	m_pPartitionSystem->Update(registry, frameTime);
	Systems::Movement::BeginMovement(registry, frameTime);



	Systems::Physics::UpdatePhysicsEarly(registry, frameTime);
	Systems::Boids::UpdateBoidSystem(registry, frameTime);
	Systems::Physics::UpdatePhysics(registry, frameTime);
	Systems::Targeting::UpdateTargeting(registry, frameTime);
	Systems::Damage::Update(registry, frameTime);

	Systems::Movement::UpdateMovement(registry, frameTime);
	//Systems::Debug::UpdateTransformDebug(registry, frameTime);
	
}

void CGamePlugin::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	auto& registry = GetRegistry();
	switch (event)
	{
		// Called when the game framework has initialized and we are ready for game logic to start
		case ESYSTEM_EVENT_GAME_POST_INIT:
		{
			// Listen for client connection events, in order to create the local player
			gEnv->pGameFramework->AddNetworkedClientListener(*this);

			// Don't need to load the map in editor
			if (!gEnv->IsEditor())
			{
				// Load the example map in client server mode
				//gEnv->pConsole->ExecuteString("map example s", false, true);
			}
		}
		break;

		case ESYSTEM_EVENT_REGISTER_SCHEMATYC_ENV:
		{
			// Register all components that belong to this plug-in
			auto staticAutoRegisterLambda = [](Schematyc::IEnvRegistrar& registrar)
			{
				// Call all static callback registered with the CRY_STATIC_AUTO_REGISTER_WITH_PARAM
				Detail::CStaticAutoRegistrar<Schematyc::IEnvRegistrar&>::InvokeStaticCallbacks(registrar);
			};

			if (gEnv->pSchematyc)
			{
				gEnv->pSchematyc->GetEnvRegistry().RegisterPackage(
					stl::make_unique<Schematyc::CEnvPackage>(
						CGamePlugin::GetCID(),
						"EntityComponents",
						"Crytek GmbH",
						"Components",
						staticAutoRegisterLambda
						)
				);
			}
		}
		break;
		
		case ESYSTEM_EVENT_LEVEL_UNLOAD:
		{
			m_players.clear();
		}
		break;
		case ESYSTEM_EVENT_EDITOR_GAME_MODE_CHANGED:
		{
			if (wparam)
			{
				/*registry.each([&](entt::entity entity) {
					registry.emplace_or_replace<entt::tag<"KeepAlive"_hs>>(entity);
				});*/
			}
			else
			{
				/*auto destroy = registry.view<>(entt::exclude<entt::tag<"KeepAlive"_hs>>);
				registry.destroy(destroy.begin(), destroy.end());*/

				registry.clear();
			}
			
		}
		break;
	}
}

bool CGamePlugin::OnClientConnectionReceived(int channelId, bool bIsReset)
{
	// Connection received from a client, create a player entity and component
	SEntitySpawnParams spawnParams;
	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
	
	// Set a unique name for the player entity
	const string playerName = string().Format("Player%" PRISIZE_T, m_players.size());
	spawnParams.sName = playerName;
	
	// Set local player details
	if (m_players.empty() && !gEnv->IsDedicated())
	{
		spawnParams.id = LOCAL_PLAYER_ENTITY_ID;
		spawnParams.nFlags |= ENTITY_FLAG_LOCAL_PLAYER;
	}

	// Spawn the player entity
	if (IEntity* pPlayerEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams))
	{
		// Set the local player entity channel id, and bind it to the network so that it can support Multiplayer contexts
		pPlayerEntity->GetNetEntity()->SetChannelId(channelId);

		// Create the player component instance
		CPlayerComponent* pPlayer = pPlayerEntity->GetOrCreateComponentClass<CPlayerComponent>();

		if (pPlayer != nullptr)
		{
			// Push the component into our map, with the channel id as the key
			m_players.emplace(std::make_pair(channelId, pPlayerEntity->GetId()));
		}
	}

	return true;
}

bool CGamePlugin::OnClientReadyForGameplay(int channelId, bool bIsReset)
{
	// Revive players when the network reports that the client is connected and ready for gameplay
	auto it = m_players.find(channelId);
	if (it != m_players.end())
	{
		if (IEntity* pPlayerEntity = gEnv->pEntitySystem->GetEntity(it->second))
		{
			if (CPlayerComponent* pPlayer = pPlayerEntity->GetComponent<CPlayerComponent>())
			{
				pPlayer->OnReadyForGameplayOnServer();
			}
		}
	}

	return true;
}

void CGamePlugin::OnClientDisconnected(int channelId, EDisconnectionCause cause, const char* description, bool bKeepClient)
{
	// Client disconnected, remove the entity and from map
	auto it = m_players.find(channelId);
	if (it != m_players.end())
	{
		gEnv->pEntitySystem->RemoveEntity(it->second);

		m_players.erase(it);
	}
}

void CGamePlugin::IterateOverPlayers(std::function<void(CPlayerComponent& player)> func) const
{
	for (const std::pair<int, EntityId>& playerPair : m_players)
	{
		if (IEntity* pPlayerEntity = gEnv->pEntitySystem->GetEntity(playerPair.second))
		{
			if (CPlayerComponent* pPlayer = pPlayerEntity->GetComponent<CPlayerComponent>())
			{
				func(*pPlayer);
			}
		}
	}
}

CRYREGISTER_SINGLETON_CLASS(CGamePlugin)