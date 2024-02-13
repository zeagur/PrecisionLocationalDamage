#include "LocationalDamageHandler.h"

auto magicSource = RE::MagicSystem::CastingSource::kInstant;

LocationalDamageHandler* LocationalDamageHandler::GetSingleton()
{
	static LocationalDamageHandler singleton;
	return std::addressof(singleton);
}

PRECISION_API::PreHitCallbackReturn LocationalDamageHandler::OnPrecisionPreHit(const PRECISION_API::PrecisionHitData& a_precisionHitData)
{
	PRECISION_API::PreHitCallbackReturn ret;

	const RE::TESObjectREFR* target = a_precisionHitData.target;
	const RE::hkpRigidBody* hitRigidBody = a_precisionHitData.hitRigidBody;

	if (!target) {
		return ret;
	}

	if (!hitRigidBody) {
		return ret;
	}

	if (target->GetFormType() != RE::FormType::ActorCharacter) {
		return ret;
	}

	if (target->IsDead()) {
		return ret;
	}

	//ctds happen if we expect the node to have a name by default
	//plus the entire mechanic kind of hinges on nodes having a name
	if (!hitRigidBody->name.data()) {
		return ret;
	}

	const std::string hitRigidBodyName = hitRigidBody->name.data();

	PRECISION_API::PreHitModifier newModifier {};

	newModifier.modifierOperation = PRECISION_API::PreHitModifier::ModifierOperation::Multiplicative;
	newModifier.modifierType = PRECISION_API::PreHitModifier::ModifierType::Damage;
	newModifier.modifierValue = 1.0f;

	for (auto& hitEffect : g_hitEffectVector) {
		[&] {
			if (std::ranges::find(hitEffect.nodeNames, hitRigidBodyName) == hitEffect.nodeNames.end())
				return;

			if (!isnan(hitEffect.damageMult))
				newModifier.modifierValue = hitEffect.damageMult;
		}();
	}

	ret.modifiers.push_back(newModifier);

	return ret;
}

void LocationalDamageHandler::OnPrecisionPostHit(const PRECISION_API::PrecisionHitData& a_precisionHitData, const RE::HitData& a_vanillaHitData)
{
	RE::TESObjectREFR* target = a_precisionHitData.target;
	const RE::hkpRigidBody* hitRigidBody = a_precisionHitData.hitRigidBody;

	if (!target) {
		return;
	}

	if (!hitRigidBody) {
		return;
	}

	if (target->GetFormType() != RE::FormType::ActorCharacter) {
		return;
	}

	if (target->IsDead()) {
		return;
	}

	//ditto
	if (!hitRigidBody->name.data()) {
		return;
	}

	const std::string hitRigidBodyName = hitRigidBody->name.data();

	const bool isCriticalHit = a_vanillaHitData.flags.any(RE::HitData::Flag::kCritical);
	const bool isPowerAttack = a_vanillaHitData.flags.any(RE::HitData::Flag::kPowerAttack);

	auto* targetActor = target->As<RE::Actor>();
	RE::Actor* attacker = a_precisionHitData.attacker;

	const RE::TESObjectWEAP* weapon = a_vanillaHitData.weapon;

	RE::MagicCaster* magicCaster = attacker->GetMagicCaster(magicSource);

	for (auto hitEffect : g_hitEffectVector) {
		[&] {
			if (!isPowerAttack && hitEffect.spellOnlyPowerAttacks) {
				RE::ConsoleLog::GetSingleton()->Print("spellOnlyPowerAttacks triggered");
				return;
			}

			if (!isCriticalHit && hitEffect.spellOnlyCriticalHits) {
				RE::ConsoleLog::GetSingleton()->Print("spellOnlyCriticalHits triggered");
				return;
			}
			if (!hitRigidBodyName.empty() && std::ranges::find(hitEffect.nodeNames, hitRigidBodyName) == hitEffect.nodeNames.end()) {
				RE::ConsoleLog::GetSingleton()->Print("Something hit triggered");
				return;
			}
			if (weapon && !hitEffect.weaponKeywords.empty() && !weapon->HasKeywordInArray(hitEffect.weaponKeywords, false)) {
				RE::ConsoleLog::GetSingleton()->Print("weapon hit triggered");
				return;
			}
			if (hitEffect.spellForm) {
				magicCaster->CastSpellImmediate(hitEffect.spellForm, false, targetActor, 1.0f, false, 0, nullptr);
				RE::ConsoleLog::GetSingleton()->Print("Magic hit triggered");
			}
		}();
	}
}

void LocationalDamageHandler::Initialize()
{
	if(!g_precision) {
		LocationalDamageHandler::GetSingleton() -> g_precision = reinterpret_cast<PRECISION_API::IVPrecision4*>(RequestPluginAPI(PRECISION_API::InterfaceVersion::V4));
		if (LocationalDamageHandler::GetSingleton() -> g_precision) {
			INFO("Obtained PRECISION_API - {0:x}", reinterpret_cast<uintptr_t>(LocationalDamageHandler::GetSingleton()->g_precision));
		} else {
			WARN("Failed to obtain PRECISION_API");
		}
	}
	if (g_precision) {
		/* first time will be return ok, second time will be marked as registered */
		auto preHitResult = g_precision->AddPreHitCallback(SKSE::GetPluginHandle(), OnPrecisionPreHit);
		if (preHitResult == PRECISION_API::APIResult::AlreadyRegistered || preHitResult == PRECISION_API::APIResult::OK) {
			RE::ConsoleLog::GetSingleton()->Print("precision PRE-Hit callback registered");
		}
		auto postHitResult = g_precision->AddPostHitCallback(SKSE::GetPluginHandle(), OnPrecisionPostHit);
		if (postHitResult == PRECISION_API::APIResult::AlreadyRegistered || postHitResult == PRECISION_API::APIResult::OK) {
			RE::ConsoleLog::GetSingleton()->Print("precision POST-Hit callback registered");
		}
	}
}