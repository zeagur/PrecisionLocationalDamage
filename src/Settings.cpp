#include "boost/algorithm/string.hpp"

#include "nlohmann/json.hpp"
#include "tojson.h"

#include "LocationalDamageHandler.h"
#include "Settings.h"

using namespace nlohmann;

std::vector<Settings::HitEffect> g_hitEffectVector;

namespace Settings {
	void RunConfig(json& a_jsonData) {

		auto dataHandler = RE::TESDataHandler::GetSingleton();

		for (auto& element : a_jsonData) {
			HitEffect newHitEffect;

			if (element.contains("damageMult")) {
				newHitEffect.damageMult = element["damageMult"];
			} else {
				logger::info("no damage multiplier specified, defaulted to 1.0");
				newHitEffect.damageMult = 1.0f;
			}

			if (element.contains("weaponKeywords")) {
				if (element["weaponKeywords"].is_array()) {
					for (auto& entry : element["weaponKeywords"]) {
						std::string input = entry;

						std::vector<std::string> result;

						boost::split(result, input, boost::is_any_of("|"));

						std::string pluginName = result[0];
						std::string formIDString = result[1];

						int formID = stoi(formIDString, nullptr, 0);

						if (auto* newKeyword = dataHandler->LookupForm(formID, pluginName)->As<RE::BGSKeyword>()) {
							newHitEffect.weaponKeywords.push_back(newKeyword);
						} else {
							logger::warn("could not get keyword from {}", input);
						}
						
					}

				} else {
					std::string input = element["weaponKeywords"];

					std::vector<std::string> result;

					boost::split(result, input, boost::is_any_of("|"));

					std::string pluginName = result[0];
					std::string formIDString = result[1];

					int formID = stoi(formIDString, nullptr, 0);

					if (auto* newKeyword = dataHandler->LookupForm(formID, pluginName)->As<RE::BGSKeyword>()) {
						newHitEffect.weaponKeywords.push_back(newKeyword);
					} else {
						logger::warn("could not get keyword from {}", input);
					}
				}
			}

			if (element.contains("spell")) {
				std::string input = element["spell"];

				std::vector<std::string> result;

				boost::split(result, input, boost::is_any_of("|"));

				std::string pluginName = result[0];
				std::string formIDString = result[1];

				int formID = stoi(formIDString, nullptr, 0);

				if (auto* newSpellItem = dataHandler->LookupForm(formID, pluginName)->As<RE::SpellItem>()) {
					newHitEffect.spellForm = newSpellItem;
				} else {
					logger::warn("could not get spell from {}", input);
				}
				
			}

			if (element.contains("spellOnlyCriticalHits")) {
				newHitEffect.spellOnlyCriticalHits = element["spellOnlyCriticalHits"];
			}

			if (element.contains("spellOnlyPowerAttacks")) {
				newHitEffect.spellOnlyPowerAttacks = element["spellOnlyPowerAttacks"];
			}

			if (auto nodeNames = element.find("nodeNames"); nodeNames != element.end()) {
				if (nodeNames->is_array()) {
					newHitEffect.nodeNames = element["nodeNames"].get<std::vector<std::string>>();
				} else {
					newHitEffect.nodeNames.push_back(*nodeNames);
				}
			}

			g_hitEffectVector.push_back(newHitEffect);
			logger::info("Settings loaded with damage multiplier of : {}", g_hitEffectVector.data()->damageMult);
		}

		logger::info("finished parsing .yamls :)");
	}

	void ParseConfigs(std::set<std::string>& a_configs) {
		for (const auto& config : a_configs) {
			auto path = std::filesystem::path(config).filename();
			auto filename = path.string();
			logger::info("Parsing {}", filename);

			try {
				if (std::ifstream i(config); i.good()) {
					json data;
					try {
						logger::info("Converting {} to JSON object", filename);
						data = tojson::loadyaml(config);
					}
					catch (const std::exception& exc) {
						std::string errorMessage = std::format("Failed to convert {} to JSON object\n{}", filename, exc.what());
						ERROR("{}", errorMessage);
						continue;
					}
					i.close();
					RunConfig(data);
				}
				else {
					std::string errorMessage = std::format("Failed to parse {}\nBad file stream", filename);
					ERROR("{}", errorMessage);
				}
			}
			catch (const std::exception& exc) {
				std::string errorMessage = std::format("Failed to parse {}\n{}", filename, exc.what());
				ERROR("{}", errorMessage);
			}
		}
	}

	void Initialize() {

		std::set<std::string> configs;

		for (auto constexpr folder = R"(Data\)"sv; const auto& entry : std::filesystem::directory_iterator(folder)) {
			if (entry.exists() && !entry.path().empty() && (entry.path().extension() == ".yaml"sv)) {
				const auto path = entry.path().string();
				const auto filename = entry.path().filename().string();
				const auto lastindex = filename.find_last_of('.');
				if (auto rawname = filename.substr(0, lastindex); rawname.ends_with("_PLD")) {
					const auto entryPath = entry.path().string();
					configs.insert(entryPath);
				}
			}
		}

		if (configs.empty()) {
			logger::warn("no valid .yaml files found; returning");
			return;
		}

		ParseConfigs(configs);
	}
}