#pragma once
#include "game/detail/buy_area_in_range.h"

template <class E, class M, class I>
inline void draw_context_tip(
	const M& typed_mode,
	const I& mode_input,
	const config_lua_table& config,
	const augs::drawer_with_default out,
	const vec2i screen_size,
	const augs::baked_font& font,
	const E& viewed_character,
	const faction_type current_faction,
	const bool show_choose_team_notice
) {
	using namespace augs::gui::text;
	using namespace augs::event;

	auto colored = [&](const auto& text, const auto& c) {
		const auto text_style = style(font, c);

		return formatted_string(text, text_style);
	};

	const auto& maybe_settings = config.arena_mode_gui.context_tip_settings;

	if (!maybe_settings.is_enabled) {
		return;
	}

	const auto& settings = maybe_settings.value;

	auto format_key = [&](const auto& m, const auto key) {
		const auto found_k = key_or_default(m, key);

		if (found_k == keys::key()) {
			return colored(" (UNASSIGNED) ", settings.bound_key_color);
		}

		return colored(" \"" + key_to_string_shortened(found_k) + "\" ", settings.bound_key_color);
	};

	const auto total_text = [&]() {
		formatted_string total_text;


		auto get_key_str = [&](auto h) {
			using H = decltype(h);

			if constexpr(std::is_same_v<H, general_gui_intent_type>) {
				return format_key(config.general_gui_controls, h);
			}
			else {
				static_assert(always_false_v<H>);
			}
		};

		auto hotkey = [&](auto h) {
			total_text += get_key_str(h);
		};

		auto text = [&](const auto& str) {
			total_text += colored(str, settings.tip_text_color);
		};

		if (current_faction == faction_type::SPECTATOR) {
			if (show_choose_team_notice) {
				text("You are a Spectator. Press");
				hotkey(general_gui_intent_type::CHOOSE_TEAM);
				text("to change teams.");
			}

			return total_text;
		}

		if (viewed_character.dead()) {
			return total_text;
		}

		if (::buy_area_in_range(viewed_character)) {
			if (typed_mode.get_buy_seconds_left(mode_input) <= 0.f) {
				text("It is too late to buy items now.");
				return total_text;
			}

			text("Press");
			hotkey(general_gui_intent_type::OPEN_BUY_MENU);
			text("to buy items.");

			return total_text;
		}

		(void)viewed_character;
		return total_text;
	}();

	const auto final_pos = vec2i(screen_size.x / 2, settings.tip_offset_mult * screen_size.y);

	print_stroked(
		out,
		final_pos,
		total_text,
		{ augs::ralign::CX, augs::ralign::CY }
	);
}
