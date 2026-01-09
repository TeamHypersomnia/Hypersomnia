#pragma once
#include "augs/misc/simple_pair.h"
#include "game/components/crosshair_component.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/modes/difficulty_type.h"

inline void interpolate_crosshair(
	const ai_character_context& ctx,
	const bool has_target,
	const float dt_secs,
	const difficulty_type difficulty
) {
	if (auto crosshair = ctx.character_handle.find_crosshair()) {
		const auto average_factor = 0.5f;
		auto averages_per_sec = has_target ? 4.0f : 3.0f;

		if (difficulty == difficulty_type::EASY) {
			averages_per_sec = 1.5f;
		}

		const auto averaging_constant = 1.0f - static_cast<real32>(repro::pow(average_factor, averages_per_sec * dt_secs));

		crosshair->base_offset = augs::interp(crosshair->base_offset, ctx.ai_state.target_crosshair_offset, averaging_constant);
	}
}
