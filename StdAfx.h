// Copyright 2001-2019 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CryCore/Project/CryModuleDefs.h>
#define eCryModule eCryM_EnginePlugin
#define GAME_API   DLL_EXPORT

#include <CryCore/Platform/platform.h>
#include <CrySystem/ISystem.h>
#include <Cry3DEngine/I3DEngine.h>
#include <CryNetwork/ISerialize.h>

#define ENTT_ASSERT CRY_ASSERT
#include <CryECS/TypeContext.h>

namespace Constants3D
{
	inline static Vec3 Forward( 0.f, 1.f, 0.f );
}