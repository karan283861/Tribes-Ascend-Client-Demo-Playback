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

UE3_PROCESSINTERNAL_HOOK(ActorSetInitialState)
{
	auto controller{reinterpret_cast<AController *>(calling_uobject)};
	if (controller->Class == kDemoRecControllerClass)
	{
		return;
	}
	original_processinternal(calling_uobject, unused, stack, result);
}

UE3_PROCESSINTERNAL_HOOK(TrPlayerControllerReceiveLocalizedMessage)
{
	auto controller{reinterpret_cast<AController *>(calling_uobject)};
	if (controller->Class == kDemoRecControllerClass)
	{
		return;
	}
	original_processinternal(calling_uobject, unused, stack, result);
}

UE3_PROCESSINTERNAL_HOOK(TrPlayerControllerRovingSpectateBeginState)
{
	auto controller{reinterpret_cast<AController *>(calling_uobject)};
	if (controller->Class == kDemoRecControllerClass)
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

UE3_PROCESSINTERNAL_HOOK(TrPlayerControllerRovingSpectateViewAPlayer)
{
	auto controller{reinterpret_cast<AController *>(calling_uobject)};
	if (controller->Class == kDemoRecControllerClass)
	{
		auto demo_rec_controller{reinterpret_cast<DemoRecController *>(calling_uobject)};
		demo_rec_controller->DemoViewNextPlayer();
	}
	original_processinternal(calling_uobject, unused, stack, result);
}

UE3_PROCESSINTERNAL_HOOK(TrPlayerControllerInitInputSystem)
{
	original_processinternal(calling_uobject, unused, stack, result);
	auto controller{reinterpret_cast<AController *>(calling_uobject)};
	if (controller->Class == kDemoRecControllerClass)
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

UE3_PROCESSINTERNAL_HOOK(TrPlayerControllerPlayerTick)
{
	auto controller{reinterpret_cast<AController *>(calling_uobject)};
	if (controller->Class == kDemoRecControllerClass)
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