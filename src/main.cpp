#include "LocationalDamageHandler.h"
#include "Settings.h"

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		Settings::Initialize();
		LocationalDamageHandler::GetSingleton()->Initialize();
		break;
	}
}

DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent()) { Sleep(100); }
#endif

	DKUtil::Logger::Init(Plugin::NAME, REL::Module::get().version().string());

	Init(a_skse);
	
	RE::ConsoleLog::GetSingleton()->Print("{} v{} loaded", Plugin::NAME, Plugin::Version);

	SKSE::GetMessagingInterface() -> RegisterListener("SKSE", MessageHandler);

	return true;
}