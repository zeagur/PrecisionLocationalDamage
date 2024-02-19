void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		Settings::Initialize();
		LocationalDamageHandler::Initialize();
		break;
	default:
		break;
	}
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
	SKSE::Init(skse);

	SetupLog();

	auto* plugin = SKSE::PluginDeclaration::GetSingleton();
	auto name = plugin->GetName();
	auto version = plugin->GetVersion();
	logger::info("SKSE plugin : {} v{} loaded", name, version);

	// Once all plugins and mods are loaded, then the ~ console is ready and can
	SKSE::GetMessagingInterface()->RegisterListener("SKSE", MessageHandler);

	return true;
}