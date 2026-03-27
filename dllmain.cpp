#include <Windows.h>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>

#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>
#ifndef _DEBUG
// #define PLOG_DISABLE_LOGGING
#endif

#include "Detours/include/detours.h"

#include <uhook.hpp>
#include "processinternal_hooks.hpp"

#include "SdkHeaders.h"

using namespace UE3;

// #define HOOK_CALLFUNCTION
#define LOG_FILE_NAME "ClientDemoPlayback.txt"

constexpr size_t kProcessEventAddress{0x00456F90};
constexpr size_t kProcessInternalAddress{0x00459040};
constexpr size_t kCallFunctionAddress{0x0045AD20};

void SetupUFunctionHooks(size_t base_address)
{
	log_function = [](const std::string &log_string)
	{ PLOG_VERBOSE << log_string; };

	get_ufunction_from_name = [](const std::string &ufunction_name)
	{
		return reinterpret_cast<UFunction *>(UObject::FindObject<UFunction>(ufunction_name.c_str()));
	};

	get_ufunction_id = [](UFunctionInternal *ufunction_object)
	{
		return reinterpret_cast<UFunction *>(ufunction_object)->ObjectInternalInteger;
	};

	get_uobject_name = [](UObjectInternal *uobject_object)
	{
		return reinterpret_cast<UObject *>(uobject_object)->GetFullName();
	};

	is_ufunction_native = [](UFunctionInternal *ufunction_object)
	{
		static constexpr unsigned int kFUNC_Native{0x00000400};
		auto ufunction{reinterpret_cast<UFunction *>(ufunction_object)};

		auto is_native{ufunction->iNative};
		auto is_funcnative{ufunction->FunctionFlags & kFUNC_Native};
		return is_native || is_funcnative;
	};

	original_processevent = reinterpret_cast<ProcessEventPrototype>(kProcessEventAddress);
	original_processinternal = reinterpret_cast<ProcessInternalPrototype>(kProcessInternalAddress);
	original_callfunction = reinterpret_cast<CallFunctionPrototype>(kCallFunctionAddress);
}

void PerformUFunctionHooks()
{
	std::vector<UFunctionHooks<ProcessInternalPrototype>::UFunctionHookInformation> processinternal_hooks_informations{
		// Prevent crashes during demo recording (legacy)
		{.name_ = "Function Engine.Actor.SetInitialState", .hook_function_ = ActorSetInitialState, .hook_type_ = FunctionHookType::kPre, .hook_absorb_ = FunctionHookAbsorb::kAbsorb},
		{.name_ = "Function TribesGame.TrPlayerController.ReceiveLocalizedMessage", .hook_function_ = TrPlayerControllerReceiveLocalizedMessage, .hook_type_ = FunctionHookType::kPre, .hook_absorb_ = FunctionHookAbsorb::kAbsorb},

		// !! Legacy comment (Create a HUD for the DemoRecSpectator)
		{.name_ = "Function TrPlayerController.RovingSpectate.BeginState", .hook_function_ = TrPlayerControllerRovingSpectateBeginState, .hook_type_ = FunctionHookType::kPre},

		// !! Legacy comment (Change view point while spectating)
		{.name_ = "Function TrPlayerController.RovingSpectate.ViewAPlayer", .hook_function_ = TrPlayerControllerRovingSpectateViewAPlayer, .hook_type_ = FunctionHookType::kPre, .hook_absorb_ = FunctionHookAbsorb::kAbsorb},

		// Initial setup of input bindings for spectator tools
		// This may be neccessary for certain spectator bindings (eg. zoom) to work after initialisation
		{.name_ = "Function TribesGame.TrPlayerController.InitInputSystem", .hook_function_ = TrPlayerControllerInitInputSystem, .hook_type_ = FunctionHookType::kPre, .hook_absorb_ = FunctionHookAbsorb::kAbsorb},

		// TODO: Find a better way to do this...
		// Setup of input bindings for spectator tools every demo rec controller tick
		{.name_ = "Function TribesGame.TrPlayerController.PlayerTick", .hook_function_ = TrPlayerControllerPlayerTick, .hook_type_ = FunctionHookType::kPost},
	};

	for (const auto &ufunction_hook_information : processinternal_hooks_informations)
	{
		const auto result{processinternal_hooks.AddHook(ufunction_hook_information)};
		ValidateUFunctionHookResult(result, ufunction_hook_information);
	}

	processinternal_hooks.SetOriginalFunction(original_processinternal);
}

void OnDLLProcessAttach()
{
	auto base_address{reinterpret_cast<size_t>(GetModuleHandle(0))};

	std::filesystem::remove(LOG_FILE_NAME);

#if defined(_DEBUG)
	static plog::RollingFileAppender<plog::TxtFormatter> file_appender(LOG_FILE_NAME);
	plog::init(plog::info, &file_appender);
#else
	static plog::RollingFileAppender<plog::TxtFormatter> file_appender(LOG_FILE_NAME);
	plog::init(plog::info, &file_appender);
#endif
	PLOG_INFO << std::format("Successfully Injected DLL");
	PLOG_INFO << std::format("Base address: {0}", reinterpret_cast<void *>(base_address));

	SetupUFunctionHooks(base_address);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

#if defined(_DEBUG)
	DetourAttach(&(PVOID &)original_processevent, ProcessEventHook);
#endif

	DetourAttach(&(PVOID &)original_processinternal, ProcessInternalHook);

	// For some reason multiple hooks on CallFunction (eg. multiple injected dlls) causes buggy behaviour/crashes.
	// We should really only hook CallFunction when tracing UFunctions in the logs (ie. debugging)
#if defined(_DEBUG) && defined(HOOK_CALLFUNCTION)
	DetourAttach(&(PVOID &)original_callfunction, CallFunctionHook);
#endif

	// Make sure all detours attaches are placed BEFORE this call
	// Make sure UFunctionHooks objects are created AFTER this call
	auto error{DetourTransactionCommit()};

	PerformUFunctionHooks();
}

BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD ul_reason_for_call,
					  LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnDLLProcessAttach, NULL, NULL, NULL);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
