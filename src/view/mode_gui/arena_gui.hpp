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

		auto draw_time_at_top = [&](const std::string& val, const rgba& col) {
			const auto text_style = style(
				in.gui_font,
				col
			);

			const auto t = game_screen_top;
			const auto s = in.screen_size;

			print_stroked(
				in.drawer,
				{ s.x / 2, static_cast<int>(t + 2) },
				formatted_string(val, text_style),
				{ augs::center::X }
			);
		};

		if (const auto freeze_left = typed_mode.get_freeze_seconds_left(mode_input); freeze_left > 0.f) {
			const auto ceiled_left = std::ceil(freeze_left);
			const auto col = ceiled_left <= 10.f ? red : white;

			auto& last = last_seconds_value;

			auto& vol = in.config.audio_volume;

			if (ceiled_left > 0.f && ceiled_left <= 3.f) {
				auto play_tick = [&]() {
					const auto gain = vol.sound_effects;

					tick_sound.set_gain(gain);
					tick_sound.stop();
					tick_sound.bind_buffer(in.sounds.round_clock_tick);
					tick_sound.set_direct_channels(true);
					tick_sound.play();
				};

				if (last == std::nullopt) {
					play_tick();
				}
				else {
					if (ceiled_left < *last) {
						play_tick();
					}
				}

				last = ceiled_left;
			}
			else {
				last = std::nullopt;
			}

			draw_time_at_top(format_mins_secs(ceiled_left), col);
		}
		else {
			const auto time_left = std::ceil(typed_mode.get_round_seconds_left(mode_input));
			const auto col = time_left <= 10.f ? red : white;
			draw_time_at_top(format_mins_secs(time_left), col);
		}
	}
	else {
		(void)typed_mode;
		(void)in;
	}
}
