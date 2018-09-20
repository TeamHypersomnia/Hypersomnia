#include "view/mode_gui/arena/arena_mode_gui.h"
#include "view/viewables/images_in_atlas_map.h"
#include "augs/gui/text/printer.h"
#include "augs/templates/chrono_templates.h"
#include "application/config_lua_table.h"
#include "game/modes/mode_player_id.h"
#include "augs/window_framework/event.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "game/cosmos/entity_handle.h"

#include "game/cosmos/cosmos.h"
#include "game/modes/test_scene_mode.h"
#include "game/modes/bomb_mode.h"
#include "augs/string/format_enum.h"
#include "game/detail/damage_origin.hpp"

bool arena_gui_state::control(
	const app_ingame_intent_input in
) {
	if (scoreboard.control(in)) {
		return true;
	}

	if (buy_menu.control(in)) {
		return true;
	}

	if (choose_team.control(in)) {
		return true;
	}

	return false;
}

template <class M>
void arena_gui_state::perform_imgui(
	draw_mode_gui_input mode_in, 
	const M& typed_mode, 
	const typename M::input& mode_input
) {
	if constexpr(M::round_based) {
		const auto p = typed_mode.calc_participating_factions(mode_input);

		using I = arena_choose_team_gui::input::faction_info;

		const auto max_players_in_each = mode_input.vars.max_players / p.size();

		std::vector<I> factions;

		auto add = [&](const auto f) {
			const auto col = mode_in.config.faction_view.colors[f].standard;
			factions.push_back({ f, col, typed_mode.num_players_in(f), max_players_in_each });
		};

		add(p.defusing);
		add(p.bombing);

		{
			const auto choice = choose_team.perform_imgui({
				mode_input.vars.view.square_logos,
				factions,
				mode_in.images_in_atlas
			});

			if (choice != std::nullopt) {
				mode_in.entropy.players[mode_in.local_player].team_choice = *choice;
			}
		}

		{
			const auto& cosm = mode_input.cosm;

			if (const auto p = typed_mode.find(mode_in.local_player)) {
				const auto guid = p->guid;

				if (const auto this_player_handle = cosm[guid]) {
					const auto choice = buy_menu.perform_imgui({
						this_player_handle,
						mode_input.vars.view.money_icon,
						p->stats.money,
						mode_in.images_in_atlas,
						mode_in.config.arena_mode_gui.money_indicator_color,
						true,
						p->stats.round_state.done_purchases,
						mode_input.vars.economy.give_extra_mags_on_first_purchase,
						mode_in.config.arena_mode_gui.buy_menu_settings
					});

					if (choice != std::nullopt) {
						mode_in.entropy.players[mode_in.local_player].queues.post(*choice);
					}
				}
			}
		}
	}
}

template <class M>
void arena_gui_state::draw_mode_gui(
	const draw_setup_gui_input& in,
	const draw_mode_gui_input& mode_in,

	const M& typed_mode, 
	const typename M::input& mode_input
) const {
	const auto& cfg = in.config.arena_mode_gui;
	const auto line_height = in.gui_fonts.gui.metrics.get_height();

	if constexpr(M::round_based) {
		scoreboard.draw_gui(in, mode_in, typed_mode, mode_input);

		using namespace augs::gui::text;

		const auto local_player = mode_in.local_player;
		auto game_screen_top = mode_in.game_screen_top;

		game_screen_top += 2;

		auto colored = [&](const auto& text, const auto& c) {
			const auto text_style = style(
				in.gui_fonts.gui,
				c
			);

			return formatted_string(text, text_style);
		};

		auto calc_size = [&](const auto& text) { 
			return get_text_bbox(colored(text, white));;
		};

		{
			// TODO: fix this for varying icon sizes

			if (const auto p = typed_mode.find(local_player)) {
				const auto& stats = p->stats;

				auto drawn_current_money = stats.money;

				const auto money_indicator_pos = [&]() {
					// const auto max_money_indicator_w = calc_size("999999$").x;

					const auto& off = cfg.money_indicator_pos;

					auto x = off.x;

					if (x < 0) {
						x = in.screen_size.x + x;
					}

					auto y = off.y;

					if (y < 0) {
						y = in.screen_size.y + y;
					}
					else {
						y += game_screen_top;
					}

					return vec2i(x, y);
				}();

				{
					const auto& cosm = mode_input.cosm;
					const auto& awards = stats.round_state.awards;
					const auto& clk = cosm.get_clock();

					const auto awards_to_show = std::min(
						awards.size(), 
						static_cast<std::size_t>(cfg.show_recent_awards_num)
					);

					const auto starting_i = [&]() {
						auto i = awards.size() - awards_to_show;

						while (i < awards.size() && clk.diff_seconds(awards[i].when) >= cfg.keep_recent_awards_for_seconds) {
							++i;
						}

						return i;
					}();

					for (std::size_t i = starting_i; i < awards.size(); ++i) {
						const auto& a = awards[i].amount;
						drawn_current_money -= a;

						const auto award_color = a > 0 ? cfg.award_indicator_color : red;
						const auto award_text = a > 0 ? typesafe_sprintf("+ %x$", a) : typesafe_sprintf("- %x$", -a);
						const auto award_text_w = calc_size(award_text).x;

						auto award_indicator_pos = money_indicator_pos;
						award_indicator_pos.y += line_height * (i + 1 - starting_i);

						print_stroked(
							in.drawer,
							award_indicator_pos - vec2i(award_text_w, 0),
							colored(award_text, award_color)
						);
					}
				}

				const auto money_text = typesafe_sprintf("%x$", drawn_current_money);
				const auto money_text_w = calc_size(money_text).x;

				print_stroked(
					in.drawer,
					money_indicator_pos - vec2i(money_text_w, 0),
					colored(money_text, cfg.money_indicator_color)
				);
			}
		}

		{
			const auto& cosm = mode_input.cosm;
			const auto& kos = typed_mode.current_round.knockouts;
			const auto& clk = cosm.get_clock();

			const auto knockouts_to_show = std::min(
				kos.size(), 
				static_cast<std::size_t>(cfg.show_recent_knockouts_num)
			);

			const auto starting_i = [&]() {
				auto i = kos.size() - knockouts_to_show;

				while (i < kos.size() && clk.diff_seconds(kos[i].when) >= cfg.keep_recent_knockouts_for_seconds) {
					++i;
				}

				return i;
			}();

			auto pen = vec2i(10, game_screen_top);

			for (std::size_t i = starting_i; i < kos.size(); ++i) {
				pen.x = 10;
				pen.y += cfg.between_knockout_boxes_pad;

				const auto& ko = kos[i];

				auto get_col = [&](const mode_player_id id) {
					if (const auto p = typed_mode.find(id)) {
						const auto guid = p->guid;

						if (const auto this_player_handle = cosm[guid]) {
							return in.config.faction_view.colors[this_player_handle.get_official_faction()].standard;
						}
					}

					return gray;
				};

				auto get_name = [&](const auto id) -> entity_name_str {
					if (!id.is_set()) {
						return "";
					}

					if (const auto p = typed_mode.find(id)) {
						return p->chosen_name;
					}

					return "Disconnected";
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

				cols.background.mult_alpha(bg_alpha);
				cols.border.mult_alpha(bg_alpha);

				const auto tool_image_id = ko.origin.on_tool_used(cosm, [&](const auto& tool) -> assets::image_id {
					if constexpr(is_spell_v<decltype(tool)>) {
						return tool.appearance.icon;
					}
					else {
						return tool.get_image_id();
					}
				});

				const auto death_fallback_icon = in.images_in_atlas.at(mode_input.vars.view.icons[scoreboard_icon_type::DEATH_ICON]);
				const auto& entry = tool_image_id != std::nullopt ? in.images_in_atlas.at(*tool_image_id) : death_fallback_icon;

				const auto tool_size = [&]() {
					const auto original_tool_size = static_cast<vec2i>(entry.get_original_size());
					const auto max_tool_height = cfg.max_weapon_icon_height;

					if (max_tool_height == 0) {
						/* No limit */
						return original_tool_size;
					}

					const auto m = vec2(0, max_tool_height);
					auto s = vec2(original_tool_size);

					if (s.y > m.y) {
						s.x *= m.y / s.y;
						s.y = m.y;
					}

					return static_cast<vec2i>(s);
				}();

				const auto total_bbox = xywhi(
					pen.x,
					pen.y,
					cfg.inside_knockout_box_pad * 2 + cfg.weapon_icon_horizontal_pad * 2 + tool_size.x + lhs_bbox.x + rhs_bbox.x, 
					cfg.inside_knockout_box_pad * 2 + std::max(tool_size.y, std::max(lhs_bbox.y, rhs_bbox.y))
				);

				in.drawer.aabb_with_border(
					total_bbox,
					cols.background,
					cols.border,
					border_input { 1, 0 }
				);

				pen.x += cfg.inside_knockout_box_pad;

				const auto icon_drawn_aabb = [&]() {
					auto r = ltrbi(vec2i(pen.x + lhs_bbox.x + cfg.weapon_icon_horizontal_pad, 0), tool_size);

					auto temp = r;
					temp.place_in_center_of(ltrb(total_bbox));

					r.t = temp.t;
					r.b = temp.b;

					return r;
				}();

				pen.y = icon_drawn_aabb.get_center().y;

				print_stroked(
					in.drawer,
					pen,
					lhs_text,
					{ augs::center::Y }
				);

				pen.x += lhs_bbox.x;
				pen.x += cfg.weapon_icon_horizontal_pad;

				{
					in.drawer.base::aabb(
						entry.diffuse,
						icon_drawn_aabb,
						white
					);
				}

				pen.x += tool_size.x;
				pen.x += cfg.weapon_icon_horizontal_pad;

				print_stroked(
					in.drawer,
					pen,
					rhs_text,
					{ augs::center::Y }
				);

				pen.y = total_bbox.b();
			}
		}

		auto draw_indicator_at = [&](const auto& val, const auto t) {
			const auto s = in.screen_size;

			print_stroked(
				in.drawer,
				{ s.x / 2, static_cast<int>(t) },
				val,
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
			draw_indicator_at(colored(val, col), game_screen_top);
		};

		auto draw_info_indicator = [&](const auto& val) {
			const auto one_fourth_t = in.screen_size.y / 6;
			draw_indicator_at(val, one_fourth_t);
		};

		auto draw_warmup_indicator = draw_info_indicator;

		if (const auto warmup_left = typed_mode.get_warmup_seconds_left(mode_input); warmup_left > 0.f) {
			const auto c = std::ceil(warmup_left);

			play_tick_if_soon(c, 5.f, true);
			draw_warmup_indicator(colored("WARMUP\n" + format_mins_secs(c), white));
			return;
		}

		{
			if (const auto secs = typed_mode.get_seconds_since_planting(mode_input); secs >= 0.f && secs <= 3.f) {
				draw_info_indicator(colored("The bomb has been planted!", yellow));
				return;
			}

			const auto& win = typed_mode.current_round.last_win;

			if (win.was_set()) {
				auto indicator_text = colored(format_enum(win.winner) + " wins!", yellow);

				if (typed_mode.state == arena_mode_state::MATCH_SUMMARY) {
					indicator_text += colored(typed_mode.is_halfway_round(mode_input) ? "\n\nHalftime\n" : "\n\nIntermission\n", white);

					{
						const auto summary_secs_left = typed_mode.get_match_summary_seconds_left(mode_input);
						const auto c = std::ceil(summary_secs_left);

						indicator_text += colored(format_mins_secs(c), white);
					}
				}

				draw_info_indicator(indicator_text);

				return;
			}
		}

		if (const auto match_begins_in_seconds = typed_mode.get_match_begins_in_seconds(mode_input); match_begins_in_seconds >= 0.f) {
			const auto c = std::ceil(match_begins_in_seconds - 1.f);

			if (c > 0.f) {
				draw_warmup_indicator(colored("Match begins in\n" + format_mins_secs(match_begins_in_seconds), yellow));
			}
			else {
				draw_warmup_indicator(colored("The match has begun", yellow));
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

template void arena_gui_state::draw_mode_gui(
	const draw_setup_gui_input&,
	const draw_mode_gui_input&,
	const test_scene_mode&, 
	const test_scene_mode::input&
) const;

template void arena_gui_state::draw_mode_gui(
	const draw_setup_gui_input&,
	const draw_mode_gui_input&,
	const bomb_mode&, 
	const bomb_mode::input&
) const;

template void arena_gui_state::perform_imgui(
	draw_mode_gui_input, 
	const bomb_mode&, 
	const typename bomb_mode::input&
);

template void arena_gui_state::perform_imgui(
	draw_mode_gui_input, 
	const test_scene_mode&, 
	const typename test_scene_mode::input&
);
