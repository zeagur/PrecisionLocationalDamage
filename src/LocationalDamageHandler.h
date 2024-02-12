#pragma once

#include "PrecisionAPI.h"
#include "Settings.h"

extern std::vector<Settings::HitEffect> g_hitEffectVector;

class LocationalDamageHandler
{
public:
	static inline PRECISION_API::IVPrecision4* g_precision = nullptr;
	static PRECISION_API::PreHitCallbackReturn OnPrecisionPreHit(const PRECISION_API::PrecisionHitData& a_precisionHitData);
	static void OnPrecisionPostHit(const PRECISION_API::PrecisionHitData& a_precisionHitData, const RE::HitData& a_vanillaHitData);
	static void Initialize();
	static LocationalDamageHandler* GetSingleton();
};