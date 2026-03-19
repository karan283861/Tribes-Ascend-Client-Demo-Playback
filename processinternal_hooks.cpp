#include <format>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <string>
#include <plog/Log.h>
#include <SdkHeaders.h>
#include "helper.hpp"
#include "processinternal_hooks.hpp"

PROCESSINTERNAL_HOOK(ActorSetInitialState)
{
	if (calling_uobject->Class == kDemoRecControllerClass)
	{
		return;
	}
	original_processinternal(calling_uobject, unused, stack, result);
}

PROCESSINTERNAL_HOOK(TrPlayerControllerReceiveLocalizedMessage)
{
	if (calling_uobject->Class == kDemoRecControllerClass)
	{
		return;
	}
	original_processinternal(calling_uobject, unused, stack, result);
}

PROCESSINTERNAL_HOOK(TrPlayerControllerClientShowAccoladeText)
{
	if (calling_uobject->Class == kDemoRecControllerClass)
	{
		return;
	}
	original_processinternal(calling_uobject, unused, stack, result);
}

PROCESSINTERNAL_HOOK(TrPlayerControllerClientSetHUD)
{
	if (calling_uobject->Class == kDemoRecControllerClass)
	{
		return;
	}
	original_processinternal(calling_uobject, unused, stack, result);
}

PROCESSINTERNAL_HOOK(TrPawnClientUpdateHUDHealth)
{
}

PROCESSINTERNAL_HOOK(TrPlayerControllerRovingSpectateBeginState)
{
	if (calling_uobject->Class == kDemoRecControllerClass)
	{
		auto demo_rec_controller{reinterpret_cast<DemoRecController *>(calling_uobject)};
		if (!demo_rec_controller->myHUD)
		{
			demo_rec_controller->myHUD = reinterpret_cast<AHUD *>(demo_rec_controller->Spawn(ATrHUDTeam::StaticClass(),
																							 demo_rec_controller,
																							 FName(),
																							 demo_rec_controller->Location,
																							 demo_rec_controller->Rotation,
																							 nullptr,
																							 true));
		}
	}
}

PROCESSINTERNAL_HOOK(TrPlayerControllerRovingSpectateViewAPlayer)
{
	if (calling_uobject->Class == kDemoRecControllerClass)
	{
		auto demo_rec_controller{reinterpret_cast<DemoRecController *>(calling_uobject)};
		demo_rec_controller->DemoViewNextPlayer();
	}
	original_processinternal(calling_uobject, unused, stack, result);
}

DemoRecController *drc{};

PROCESSINTERNAL_HOOK(TrPlayerControllerInitInputSystem)
{
	original_processinternal(calling_uobject, unused, stack, result);
	if (calling_uobject->Class == kDemoRecControllerClass)
	{
		auto demo_rec_controller{reinterpret_cast<DemoRecController *>(calling_uobject)};
		if (demo_rec_controller->PlayerInput)
		{
			demo_rec_controller->PlayerInput->Bindings.Data = demo_rec_controller->GetSpectatorSettings()->Bindings.Data;
			demo_rec_controller->PlayerInput->Bindings.Count = demo_rec_controller->GetSpectatorSettings()->Bindings.Count;
			demo_rec_controller->PlayerInput->Bindings.Max = demo_rec_controller->GetSpectatorSettings()->Bindings.Max;
			// !! Legacy comment (Prevent RPCs recursive crash)
			// Not actually sure if this is needed
			demo_rec_controller->Role = ROLE_MAX;

			drc = demo_rec_controller;
		}
	}
}

PROCESSINTERNAL_HOOK(TrGameReplicationInfoTick)
{
	return;
	if (drc && drc->PlayerInput)
	{
		static auto init{false};
		if (init && drc->PlayerInput->Bindings.Data != drc->GetSpectatorSettings()->Bindings.Data)
		{
			PLOG_ERROR << "FART!";
			*((int *)NULL) = 44;
		}
		drc->PlayerInput->Bindings.Data = drc->GetSpectatorSettings()->Bindings.Data;
		drc->PlayerInput->Bindings.Count = drc->GetSpectatorSettings()->Bindings.Count;
		drc->PlayerInput->Bindings.Max = drc->GetSpectatorSettings()->Bindings.Max;
		// !! Legacy comment (Prevent RPCs recursive crash)
		// Not actually sure if this is needed
		drc->Role = ROLE_MAX;
		init = true;
	}
}

PROCESSINTERNAL_HOOK(TrPlayerControllerGameEngineSettings)
{
}

PROCESSINTERNAL_HOOK(TrPlayerControllerPlayerTick)
{
	if (calling_uobject->Class == kDemoRecControllerClass)
	{
		auto demo_rec_controller{reinterpret_cast<DemoRecController *>(calling_uobject)};
		if (demo_rec_controller->PlayerInput)
		{
			demo_rec_controller->PlayerInput->Bindings.Data = demo_rec_controller->GetSpectatorSettings()->Bindings.Data;
			demo_rec_controller->PlayerInput->Bindings.Count = demo_rec_controller->GetSpectatorSettings()->Bindings.Count;
			demo_rec_controller->PlayerInput->Bindings.Max = demo_rec_controller->GetSpectatorSettings()->Bindings.Max;
			// !! Legacy comment (Prevent RPCs recursive crash)
			// Needed - zooming in/out crashes if we don't do this
			demo_rec_controller->Role = ROLE_MAX;
		}
	}
}