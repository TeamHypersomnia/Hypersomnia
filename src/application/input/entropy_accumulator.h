#pragma once
#include "game/enums/game_intent_type.h"
#include "game/modes/mode_entropy.h"

#include "application/input/adjust_game_motions.h"

struct entropy_accumulator {
	mode_entropy entropy;

	accumulated_motions motions;
	game_intents intents;

	template <class E, class I>
	auto extract(
		const E& handle,
		const I& in
	) {
		entropy.clear_dead_entities(handle.get_cosmos());

		auto out = entropy;

		if (handle) {
			auto& player = out.cosmic[handle.get_id()];

			if (const auto crosshair_motion = mapped_or_nullptr(motions, game_motion_type::MOVE_CROSSHAIR)) {
				if (const auto crosshair = handle.find_crosshair()) {
					const auto motion = to_game_motion(*crosshair_motion, crosshair->base_offset, in.settings.mouse_sensitivity, in.screen_size);
					player.motions.push_back(motion);
				}
			}

			concatenate(player.intents, intents);
		}

		clear();

		return out;
	}

	void clear() {
		entropy.clear();
		motions.clear();
		intents.clear();
	}

	template <class T>
	void control(const T& n) {
		if constexpr(std::is_same_v<T, raw_game_motions>) {
			for (const auto& m : n) {
				auto& motion_entry = motions[m.motion];
				const auto total_offset = motion_entry.offset + m.offset;

				motion_entry = m;
				motion_entry.offset = total_offset;
			}
		}
		else if constexpr(std::is_same_v<T, game_intents>) {
			concatenate(intents, n);
		}
		else if constexpr(std::is_same_v<T, cosmic_entropy>) {
			entropy.cosmic += n;
		}
		else {
			entropy += n;
		}
	}
};
