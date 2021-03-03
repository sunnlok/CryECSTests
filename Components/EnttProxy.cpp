#include "StdAfx.h"
#include "EnttProxy.h"
#include "GamePlugin.h"
#include "CryCore/StaticInstanceList.h"
#include "CrySchematyc/Env/IEnvRegistrar.h"
#include "CrySchematyc/Env/Elements/EnvComponent.h"
#include "entt/Components/BridgeComponents.h"


CEnttProxy::CEnttProxy()
	: registry(CGamePlugin::GetInstance()->GetRegistry())

{
	entity = registry.create();
}

CEnttProxy::~CEnttProxy()
{
	CGamePlugin::GetInstance()->GetRegistry().destroy(entity);
}

auto CEnttProxy::Initialize() -> void
{
	//registry.emplace_or_replace<CryEntityParent>(entity, GetEntityId());
}

void CEnttProxy::Serialize(Serialization::IArchive& archive)
{
	auto idString = CryStringUtils::toString(entt::to_integral(entity));
	archive(idString, "enttID", "!Entity ID");
}

const char* CEnttProxy::GetLabel() const
{
	return "Entt Proxy";
}

void CEnttProxy::SerializeProperties(Serialization::IArchive& archive)
{
	Serialize(archive);
}

static void RegisterEnttComponent(Schematyc::IEnvRegistrar& registrar)
{
	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CEnttProxy));
	}
}

CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterEnttComponent);