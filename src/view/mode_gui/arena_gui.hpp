#pragma once
#include "view/mode_gui/arena_gui.h"
#include "augs/gui/text/printer.h"
#include "augs/templates/chrono_templates.h"
#include "application/config_lua_table.h"
#include "game/modes/mode_player_id.h"

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
	float game_screen_top,
	const M& typed_mode, 
	const I& mode_input,

	const mode_player_id local_player
) const {
	if constexpr(M::round_based) {
		using namespace augs::gui::text;

		game_screen_top += 2;

		{
			const auto& cosm = mode_input.cosm;
			const auto& kos = typed_mode.knockouts;
			const auto& clk = cosm.get_clock();

			const auto show_recent_knockouts_num = std::size_t(5);
			const auto knockouts_to_show = std::min(kos.size(), show_recent_knockouts_num);

			const auto font_height = in.gui_font.metrics.get_height();

			const auto starting_i = [&]() {
				auto i = kos.size() - knockouts_to_show;

				while (i < kos.size() && clk.diff_seconds(kos[i].when) >= 5.f) {
					++i;
				}

				return i;
			}();

			for (std::size_t i = starting_i; i < kos.size(); ++i) {
				const auto& ko = kos[i];
				const auto t = game_screen_top + font_height * 1.6f * (i - starting_i);

				auto get_col = [&](const mode_player_id id) {
					if (const auto p = mapped_or_nullptr(typed_mode.players, id)) {
						return ::get_faction_color(p->faction);
					}

					return gray;
				};

				auto get_name = [&](const auto id) -> entity_name_str {
					if (!id.is_set()) {
						return "";
					}

					if (const auto p = mapped_or_nullptr(typed_mode.players, id)) {
						return p->chosen_name;
					}

					return "Disconnected";
				};

				auto colored = [&](const auto& t, const auto& c) {
					const auto text_style = style(
						in.gui_font,
						c
					);

					return formatted_string(t, text_style);
				};

				const auto knockouter = get_name(ko.knockouter);
				const auto assist = get_name(ko.assist);
				const auto victim = get_name(ko.victim);

				const bool is_suicide = ko.knockouter == ko.victim;
				(void)is_suicide;

				const auto lhs_text = 
					colored(is_suicide ? "" : knockouter, get_col(ko.knockouter))
					+ colored(assist.size() > 0 ? " (+ " + assist + ")" : "", get_col(ko.assist))
				;

				const auto rhs_text = colored(victim, get_col(ko.victim));

				const auto lhs_bbox = get_text_bbox(lhs_text);
				const auto rhs_bbox = get_text_bbox(rhs_text);

				const auto bg_alpha = 0.9f;

				struct colors {
					rgba background;
					rgba border;
				} cols = [&]() -> colors {
					if (local_player == ko.victim) {
						return { rgba(150, 0, 0, 170), rgba(0, 0, 0, 0) };
					}

					if (local_player == ko.knockouter || local_player == ko.assist) {
						return { black, rgba(180, 0, 0, 255) };
					}
					
					return { { 0, 0, 0, 80 }, rgba(0, 0, 0, 0) };
				}();

				cols.background.multiply_alpha(bg_alpha);
				cols.border.multiply_alpha(bg_alpha);

				const auto tool_image_id = [&]() {
					auto from_flavour = [&](const auto flavour_id) -> auto {
						if (!flavour_id.is_set()) {
							return assets::image_id();
						}

						return flavour_id.dispatch([&](const auto& typed_flavour_id) {
							if (const auto flavour = cosm.find_flavour(typed_flavour_id)) {
								if (const auto sentience = flavour->template find<invariants::sentience>()) {
									return assets::image_id();
								}

								if (const auto sprite = flavour->template find<invariants::sprite>()) {
									return sprite->image_id;
								}
							}

							return assets::image_id();
						});
					};

					if (ko.origin.cause.spell.is_set()) {
						return ko.origin.cause.spell.dispatch([&](auto t){
							const auto& meta = get_meta_of(t, cosm.get_common_significant().spells);
							return meta.appearance.icon;
						});
					}

					if (const auto img = from_flavour(ko.origin.sender.direct_sender_flavour); img.is_set()) {
						return img;
					}

					if (const auto img = from_flavour(ko.origin.cause.flavour); img.is_set()) {
						return img;
					}

					return assets::image_id();
				}();

				const auto& entry = tool_image_id.is_set() ? in.images_in_atlas.at(tool_image_id) : image_in_atlas();
				const auto& original_tool_size = entry.get_original_size();

				const auto knockout_box_padding = 2;
				const auto max_tool_height = static_cast<int>(font_height + knockout_box_padding);
				const auto tool_size = [&]() {
					const auto m = vec2(100u, max_tool_height);

					auto s = vec2(original_tool_size);

					if (s.x > m.x) {
						s.y *= m.x / s.x;
						s.x = m.x;
					}

					if (s.y > m.y) {
						s.x *= m.y / s.y;
						s.y = m.y;
					}

					return s;
				}();

				const auto tool_img_padding = 10;
				
				auto pen = vec2i(10, static_cast<int>(t) + 10);

				const auto total_bbox = xywhi(
					pen.x,
					pen.y,
					tool_size.x + tool_img_padding * 2 + lhs_bbox.x + rhs_bbox.x, 
					std::max(lhs_bbox.y, rhs_bbox.y)
				).expand_from_center(vec2i::square(knockout_box_padding));

				in.drawer.aabb_with_border(
					total_bbox,
					cols.background,
					cols.border
				);

				print_stroked(
					in.drawer,
					pen,
					lhs_text
				);

				pen.x += lhs_bbox.x;
				pen.x += tool_img_padding;

				{
					auto drawn_aabb = ltrb(pen, tool_size);

					auto huh = drawn_aabb;
					huh.place_in_center_of(ltrb(total_bbox));
					drawn_aabb.t = huh.t;
					drawn_aabb.b = huh.b;

					in.drawer.base::aabb(
						entry.diffuse,
						drawn_aabb,
						white
					);
				}

				pen.x += tool_size.x;
				pen.x += tool_img_padding;

				print_stroked(
					in.drawer,
					pen,
					rhs_text
				);
			}
		}

		auto draw_indicator_at = [&](const std::string& val, const rgba& col, const auto t) {
			const auto text_style = style(
				in.gui_font,
				col
			);

			const auto s = in.screen_size;

			print_stroked(
				in.drawer,
				{ s.x / 2, static_cast<int>(t) },
				formatted_string(val, text_style),
				{ augs::center::X }
			);
		};

		auto play_tick_if_soon = [&](const auto& val, const auto& when_ticking_starts, const bool alarm) {
			auto play_tick = [&]() {
				auto& vol = in.config.audio_volume;

				tick_sound.just_play(
					alarm ? in.sounds.alarm_tick : in.sounds.round_clock_tick, 
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

		auto draw_info_indicator = [&](const std::string& val, const rgba& col) {
			const auto one_fourth_t = in.screen_size.y / 6;
			draw_indicator_at(val, col, one_fourth_t);
		};

		auto draw_warmup_indicator = draw_info_indicator;

		if (const auto warmup_left = typed_mode.get_warmup_seconds_left(mode_input); warmup_left > 0.f) {
			const auto c = std::ceil(warmup_left);

			play_tick_if_soon(c, 5.f, true);
			draw_warmup_indicator("WARMUP\n" + format_mins_secs(c), white);
			return;
		}

		{
			if (const auto secs = typed_mode.get_seconds_since_planting(mode_input); secs >= 0.f && secs <= 3.f) {
				draw_info_indicator("The bomb has been planted!", yellow);
				return;
			}

			auto& win = typed_mode.last_win;

			if (win.was_set()) {
				draw_info_indicator(format_enum(win.winner) + " wins!", yellow);
				return;
			}
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

			play_tick_if_soon(c, 3.f, false);
			draw_time_at_top(format_mins_secs(c), col);

			return;
		}

		if (const auto critical_left = typed_mode.get_critical_seconds_left(mode_input); critical_left > 0.f) {
			const auto c = std::ceil(critical_left);
			const auto col = red;

			draw_time_at_top(format_mins_secs(c), col);

			return;
		}

		if (const auto time_left = std::ceil(typed_mode.get_round_seconds_left(mode_input)); time_left > 0.f) {
			const auto c = std::ceil(time_left);
			const auto col = time_left <= 10.f ? red : white;

			draw_time_at_top(format_mins_secs(c), col);

			return;
		}

		/* If the code reaches here, it's probably the round end delay, so draw no timer. */
	}
	else {
		(void)typed_mode;
		(void)in;
	}
}
