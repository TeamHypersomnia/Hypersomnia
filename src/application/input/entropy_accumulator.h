#pragma once
#include "augs/templates/logically_empty.h"
#include "augs/math/camera_cone.h"
#include "game/enums/game_intent_type.h"
#include "game/modes/mode_entropy.h"

#include "application/input/input_settings.h"
#include "application/input/adjust_game_motions.h"
#include "game/per_character_input_settings.h"

struct entropy_accumulator {
	struct input {
		input_settings settings;
		const vec2i screen_size;
		const zoom_type zoom;
	};

	static const entropy_accumulator zero;

	mode_player_entropy mode;
	cosmic_player_entropy cosmic;

	accumulated_motions motions;
	game_intents intents;

	mode_entropy_general mode_general;

	template <class E>
	std::optional<raw_game_motion> calc_motion(
		const E& handle,
		const game_motion_type motion_type,
		const input in
	) const {
		if (handle) {
			if (const auto crosshair_motion = mapped_or_nullptr(motions, motion_type)) {
				if (const auto crosshair = handle.find_crosshair()) {
					const auto total_bound = vec2(in.screen_size) / in.zoom;
					return to_game_motion(*crosshair_motion, crosshair->base_offset, in.settings.character.crosshair_sensitivity, total_bound);
				}
			}
		}

		return std::nullopt;
	}

	template <class E>
	auto assemble(
		const E& handle,
		const mode_player_id& m_id,
		const input in
	) const {
		mode_entropy out;
		out.general = mode_general;

		if (logically_set(m_id, mode)) {
			out.players[m_id] = mode;

			/* Disallow controlling mode & cosmic at the same time for bandwidth optimization. */
			return out;
		}

		if (handle) {
			const auto player_id = handle.get_id();
			auto& player_entry = out.cosmic[player_id];
			player_entry.settings = in.settings.character;

			auto& player = player_entry.commands;

			if (const auto new_motion = calc_motion(handle, game_motion_type::MOVE_CROSSHAIR, in)) {
				player.motions[new_motion->motion] = new_motion->offset;
			}

			auto new_intents = intents;

			if (in.settings.swap_mouse_buttons_in_akimbo) {
				if (handle.get_wielded_items().size() > 1) {
					for (auto& i : new_intents) {
						if (i.intent == game_intent_type::SHOOT) {
							i.intent = game_intent_type::SHOOT_SECONDARY;
						}
						else if (i.intent == game_intent_type::SHOOT_SECONDARY) {
							i.intent = game_intent_type::SHOOT;
						}
					}
				}
			}

			concatenate(player.intents, new_intents);

			player += cosmic;
		}

		out.clear_dead_entities(handle.get_cosmos());

		return out;
	}

	template <class E>
	auto assemble_for(
		const E& handle,
		const mode_player_id& m_id,
		const input in
	) const {
		return assemble(handle, m_id, in).get_for(handle, m_id);
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
		mode = {};
		cosmic.clear();

		motions.clear();
		intents.clear();

		mode_general.clear();
	}

	template <class T>
	void control(const T& n) {
		if constexpr(std::is_same_v<T, raw_game_motion_vector>) {
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
			if (logically_set(n)) {
				mode = n;
			}
		}
		else if constexpr(std::is_same_v<T, mode_entropy_general>) {
			mode_general += n;
		}
		else {
			static_assert(always_false_v<T>, "Uncontrollable type.");
		}
	}
};
