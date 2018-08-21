#pragma once
#include "view/mode_gui/arena_gui.h"
#include "augs/gui/text/printer.h"
#include "augs/templates/chrono_templates.h"
#include "application/config_lua_table.h"

bool arena_gui_state::control(
	const augs::event::state& common_input_state,
	const augs::event::change e
) {
	using namespace augs::event;
	using namespace augs::event::keys;

	(void)common_input_state;
	/* const bool has_ctrl{ common_input_state[key::LCTRL] }; */
	/* const bool has_shift{ common_input_state[key::LSHIFT] }; */

	if (e.was_pressed(key::TAB)) {
		show_scores = true;
		return true;
	}

	if (e.was_released(key::TAB)) {
		show_scores = false;
		return true;
	}

	return false;
}

template <class M, class I>
void arena_gui_state::draw_mode_gui(
	const draw_setup_gui_input& in,
	const float game_screen_top,
	const M& typed_mode, 
	const I& mode_input
) const {
	if constexpr(M::round_based) {
		using namespace augs::gui::text;

		auto draw_indicator_at = [&](const std::string& val, const rgba& col, const auto t) {
			const auto text_style = style(
				in.gui_font,
				col
			);

			const auto s = in.screen_size;

			print_stroked(
				in.drawer,
				{ s.x / 2, static_cast<int>(t + 2) },
				formatted_string(val, text_style),
				{ augs::center::X }
			);
		};

		auto play_tick_if_soon = [&](const auto& val, const auto& when_ticking_starts) {
			auto play_tick = [&]() {
				auto& vol = in.config.audio_volume;

				tick_sound.just_play(
					in.sounds.round_clock_tick, 
					vol.sound_effects
				);
			};

			auto& last = last_seconds_value;

			if (val > 0.f && val <= when_ticking_starts) {
				if (last == std::nullopt) {
					play_tick();
				}
				else {
					if (val < *last) {
						play_tick();
					}
				}

				last = val;
			}
			else {
				last = std::nullopt;
			}
		};

		auto draw_time_at_top = [&](const std::string& val, const rgba& col) {
			draw_indicator_at(val, col, game_screen_top);
		};

		auto draw_warmup_indicator = [&](const std::string& val, const rgba& col) {
			const auto one_fourth_t = in.screen_size.y / 6;
			draw_indicator_at(val, col, one_fourth_t);
		};

		if (const auto warmup_left = typed_mode.get_warmup_seconds_left(mode_input); warmup_left > 0.f) {
			const auto c = std::ceil(warmup_left);

			play_tick_if_soon(c, 5.f);
			draw_warmup_indicator("WARMUP\n" + format_mins_secs(c), white);
			return;
		}

		if (const auto match_begins_in_seconds = typed_mode.get_match_begins_in_seconds(mode_input); match_begins_in_seconds >= 0.f) {
			const auto c = std::ceil(match_begins_in_seconds - 1.f);

			if (c > 0.f) {
				draw_warmup_indicator("Match begins in\n" + format_mins_secs(match_begins_in_seconds), yellow);
			}
			else {
				draw_warmup_indicator("The match has begun", yellow);
			}

			return;
		}

		if (const auto freeze_left = typed_mode.get_freeze_seconds_left(mode_input); freeze_left > 0.f) {
			const auto c = std::ceil(freeze_left);
			const auto col = c <= 10.f ? red : white;

			play_tick_if_soon(c, 3.f);
			draw_time_at_top(format_mins_secs(c), col);

			return;
		}

		{
			const auto time_left = std::ceil(typed_mode.get_round_seconds_left(mode_input));
			const auto c = std::ceil(time_left);
			const auto col = time_left <= 10.f ? red : white;

			draw_time_at_top(format_mins_secs(c), col);

			return;
		}
	}
	else {
		(void)typed_mode;
		(void)in;
	}
}
