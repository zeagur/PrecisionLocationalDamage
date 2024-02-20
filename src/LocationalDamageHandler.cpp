#include "LocationalDamageHandler.h"
#include "PrecisionAPI.h"

bool LocationalDamageHandler::isTargetValid(const RE::TESObjectREFR* target)
{
	return target && target->GetFormType() == RE::FormType::ActorCharacter && !target->IsDead();
}

bool LocationalDamageHandler::isRigidBodyValid(const RE::hkpRigidBody* hitRigidBody)
{
	return hitRigidBody && hitRigidBody->name.data();
}

bool LocationalDamageHandler::isCriticalHit(const RE::HitData& hitData)
{
	return hitData.flags.any(RE::HitData::Flag::kCritical);
}

bool LocationalDamageHandler::isPowerAttack(const RE::HitData& hitData)
{
	return hitData.flags.any(RE::HitData::Flag::kPowerAttack);
}

PRECISION_API::PreHitCallbackReturn LocationalDamageHandler::OnPrecisionPreHit(const PRECISION_API::PrecisionHitData& a_precisionHitData)
{
	PRECISION_API::PreHitCallbackReturn ret;

	const RE::TESObjectREFR* target = a_precisionHitData.target;
	const RE::hkpRigidBody* hitRigidBody = a_precisionHitData.hitRigidBody;

	if (!isTargetValid(target) || !isRigidBodyValid(hitRigidBody)) {
		return ret;
	}

	const std::string hitRigidBodyName = hitRigidBody->name.data();

	PRECISION_API::PreHitModifier newModifier{};

	newModifier.modifierOperation = PRECISION_API::PreHitModifier::ModifierOperation::Multiplicative;
	newModifier.modifierType = PRECISION_API::PreHitModifier::ModifierType::Damage;
	newModifier.modifierValue = 1.0f;

	for (auto& hitEffect : g_hitEffectVector) {
		[&] {
			if (std::ranges::find(hitEffect.nodeNames, hitRigidBodyName) == hitEffect.nodeNames.end())
				return;

			if (isnan(hitEffect.damageMult))
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
	const std::string hitRigidBodyName = hitRigidBody ? hitRigidBody->name.data() : std::string{};

	if (!isTargetValid(target) || !isRigidBodyValid(hitRigidBody)) {
		return;
	}

	auto* targetActor = target->As<RE::Actor>();
	RE::Actor* attacker = a_precisionHitData.attacker;
	const RE::TESObjectWEAP* weapon = a_vanillaHitData.weapon;
	RE::MagicCaster* magicCaster = attacker->GetMagicCaster(magicSource);

	for (auto hitEffect : g_hitEffectVector) {
		[&] {
			if (!isPowerAttack(a_vanillaHitData) && hitEffect.spellOnlyPowerAttacks)
				return;
			if (!isCriticalHit(a_vanillaHitData) && hitEffect.spellOnlyCriticalHits)
				return;
			if (!hitRigidBodyName.empty() && std::ranges::find(hitEffect.nodeNames, hitRigidBodyName) == hitEffect.nodeNames.end())
				return;
			if (weapon && !hitEffect.weaponKeywords.empty() && !weapon->HasKeywordInArray(hitEffect.weaponKeywords, false))
				return;
			if (hitEffect.spellForm)
				magicCaster->CastSpellImmediate(hitEffect.spellForm, false, targetActor, 1.0f, false, 0, nullptr);
		}();
	}
}

void LocationalDamageHandler::Initialize()
{
	g_precision = static_cast<PRECISION_API::IVPrecision4*>(RequestPluginAPI(PRECISION_API::InterfaceVersion::V4));
	if (g_precision) {
		logger::info("Obtained PrecisionAPI");
	} else {
		logger::error("Unable to acquire requested PrecisionAPI interface version");
	}

	if (const auto preHitResult = g_precision->AddPreHitCallback(SKSE::GetPluginHandle(), OnPrecisionPreHit);
		preHitResult == PRECISION_API::APIResult::OK || preHitResult == PRECISION_API::APIResult::AlreadyRegistered) {
		logger::info("precision pre hit callback registered");
	}

	if (const auto postHitResult = g_precision->AddPostHitCallback(SKSE::GetPluginHandle(), OnPrecisionPostHit);
		postHitResult == PRECISION_API::APIResult::OK || postHitResult == PRECISION_API::APIResult::AlreadyRegistered) {
		logger::info("precision post hit callback registered");
	}
}