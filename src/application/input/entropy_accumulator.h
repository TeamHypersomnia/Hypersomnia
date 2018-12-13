#pragma once
#include "game/enums/game_intent_type.h"
#include "game/modes/mode_entropy.h"

#include "application/input/input_settings.h"
#include "application/input/adjust_game_motions.h"

struct entropy_accumulator {
	struct input {
		const input_settings& settings;
		const vec2i screen_size;
	};

	mode_player_entropy mode;
	cosmic_player_entropy cosmic;

	mode_entropy_general mode_general;

	accumulated_motions motions;
	game_intents intents;

	template <class E>
	auto assemble(
		const E& handle,
		const mode_player_id& m_id,
		const input in
	) const {
		mode_entropy out;
		out.general = mode_general;

		if (m_id.is_set() && !mode.empty()) {
			out.players[m_id] = mode;

			/* Disallow controlling mode & cosmic at the same time for bandwidth optimization. */
			return out;
		}

		if (handle) {
			const auto player_id = handle.get_id();
			auto& player = out.cosmic[player_id];

			if (const auto crosshair_motion = mapped_or_nullptr(motions, game_motion_type::MOVE_CROSSHAIR)) {
				if (const auto crosshair = handle.find_crosshair()) {
					const auto motion = to_game_motion(*crosshair_motion, crosshair->base_offset, in.settings.mouse_sensitivity, in.screen_size);
					player.motions.push_back(motion);
				}
			}

			concatenate(player.intents, intents);

			player += cosmic;
		}

		out.clear_dead_entities(handle.get_cosmos());

		return out;
	}

	template <class E>
	auto extract(
		const E& handle,
		const mode_player_id& m_id,
		const input in
	) {
		auto out = assemble(handle, m_id, in);
		clear();
		return out;
	}

	void clear() {
		mode.clear();
		cosmic.clear();

		motions.clear();
		intents.clear();

		mode_general.clear();
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
		else if constexpr(std::is_same_v<T, cosmic_player_entropy>) {
			cosmic += n;
		}
		else if constexpr(std::is_same_v<T, mode_player_entropy>) {
			mode += n;
		}
		else if constexpr(std::is_same_v<T, mode_entropy_general>) {
			mode_general += n;
		}
		else {
			static_assert(always_false_v<T>, "Uncontrollable type.");
		}
	}
};
