#pragma once
#include <cstdint>
#include <optional>
#include "augs/log.h"
#include "steam_integration.h"

constexpr std::uint32_t tutorial_advanced_end_level_v = 20;

inline const char* get_tutorial_steam_achievement_api_name(
	const std::uint32_t entered_level,
	const bool left_surfing_challenge,
	const std::optional<std::uint32_t> basic_finish_level
) {
	if (basic_finish_level.has_value() && entered_level == *basic_finish_level) {
		return "TUTORIAL_BASIC";
	}

	if (entered_level == tutorial_advanced_end_level_v) {
		return left_surfing_challenge ? "SURFING_CHALLENGE" : "TUTORIAL_ADVANCED";
	}

	return nullptr;
}

inline void unlock_tutorial_steam_achievement(
	const std::uint32_t entered_level,
	const bool left_surfing_challenge,
	const std::optional<std::uint32_t> basic_finish_level
) {
	const auto api_name = ::get_tutorial_steam_achievement_api_name(
		entered_level,
		left_surfing_challenge,
		basic_finish_level
	);

	if (api_name != nullptr) {
		const bool unlocked = ::steam_unlock_achievement(api_name);
		LOG("Steam achievement %x unlocked: %x", api_name, unlocked);
	}
}
