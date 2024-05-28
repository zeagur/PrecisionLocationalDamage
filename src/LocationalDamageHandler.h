#pragma once

#include "PrecisionAPI.h"
#include "Settings.h"

extern std::vector<Settings::HitEffect> g_hitEffectVector;
inline auto magicSource = RE::MagicSystem::CastingSource::kInstant;

class LocationalDamageHandler
{
public:
	static bool isTargetValid(const RE::TESObjectREFR* target);
	static bool isRigidBodyValid(const RE::hkpRigidBody* hitRigidBody);
	static bool isCriticalHit(const RE::HitData& hitData);
	static bool isPowerAttack(const RE::HitData& hitData);
	static PRECISION_API::PreHitCallbackReturn OnPrecisionPreHit(const PRECISION_API::PrecisionHitData& a_precisionHitData);
	static void OnPrecisionPostHit(const PRECISION_API::PrecisionHitData& a_precisionHitData, const RE::HitData& a_vanillaHitData);
	static void Initialize();
	static LocationalDamageHandler* GetSingleton()
	{
		static LocationalDamageHandler singleton;
		return std::addressof(singleton);
	}
	static inline PRECISION_API::IVPrecision4* g_precision = nullptr;
};