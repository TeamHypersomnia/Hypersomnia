#include "augs/templates/logically_empty.h"
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
#include "game/modes/test_mode.h"
#include "game/modes/bomb_mode.h"
#include "augs/string/format_enum.h"
#include "game/detail/damage_origin.hpp"
#include "augs/misc/action_list/standard_actions.h"

#include "augs/math/simple_calculations.h"

const bool show_death_summary_for_teammates = false;

const auto default_popul = augs::populate_with_delays_impl(
	100.f,
	0.65f
);

bool arena_gui_state::control(
	const general_gui_intent_input in
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

	if (spectator.control(in)) {
		return true;
	}

	return false;
}

bool arena_gui_state::escape() {
	if (choose_team.show) {
		choose_team.show = false;
		return true;
	}

	if (buy_menu.escape()) {
		return true;
	}

	return false;
}

template <class M>
mode_player_entropy arena_gui_state::perform_imgui(
	draw_mode_gui_input mode_in, 
	const M& typed_mode, 
	const typename M::const_input& mode_input,

	const prediction_input prediction
) {
	mode_player_entropy result_entropy;

	if (buy_menu.show && choose_team.show) {
		buy_menu.show = false;
	}

	if constexpr(M::round_based) {
		if (prediction.play_unpredictable) {
			const auto p = typed_mode.calc_participating_factions(mode_input);

			using I = arena_choose_team_gui::input::faction_info;

			const auto max_players_in_each = mode_input.rules.max_players_per_team;

			std::vector<I> factions;

			auto add = [&](const auto f) {
				const auto col = mode_in.config.faction_view.colors[f].standard;
				factions.push_back({ f, col, typed_mode.num_players_in(f), max_players_in_each });
			};

			add(p.defusing);
			add(p.bombing);

			const auto player_id = mode_in.local_player_id;

			if (const auto p = typed_mode.find(player_id)) {
				const auto choice = choose_team.perform_imgui({
					mode_input.rules.view.square_logos,
					factions,
					mode_in.images_in_atlas,
					p->faction
				});

				if (logically_set(choice)) {
					result_entropy = *choice;
				}

				spectator.advance(
					player_id,
					typed_mode,
					mode_input
				);
			}
		}

		if (prediction.play_predictable) {
			const auto& cosm = mode_input.cosm;

			if (typed_mode.get_buy_seconds_left(mode_input) <= 0.f) {
				buy_menu.show = false;
			}

			if (const auto p = typed_mode.find(mode_in.local_player_id)) {
				if (const auto this_player_handle = cosm[p->controlled_character_id]) {
					const auto choice = buy_menu.perform_imgui({
						this_player_handle,
						mode_input.rules.view.money_icon,
						p->stats.money,
						mode_in.images_in_atlas,
						mode_in.config.arena_mode_gui.money_indicator_color,
						true,
						p->stats.round_state.done_purchases,
						mode_input.rules.economy.give_extra_mags_on_first_purchase,
						mode_in.config.arena_mode_gui.buy_menu_settings
					});

					if (logically_set(choice)) {
						result_entropy = choice;
					}
				}
			}
		}
	}

	return result_entropy;
}

template <class M>
void arena_gui_state::draw_mode_gui(
	const draw_setup_gui_input& in,
	const draw_mode_gui_input& mode_in,

	const M& typed_mode, 
	const typename M::const_input& mode_input,

	const prediction_input prediction
) const {
	const auto& cfg = in.config.arena_mode_gui;
	const auto line_height = in.gui_fonts.gui.metrics.get_height();

	if constexpr(M::round_based) {
		using namespace augs::gui::text;

		const auto local_player_id = mode_in.local_player_id;

		const auto local_player_faction = [&]() -> std::optional<faction_type> {
			if (const auto p = typed_mode.find(local_player_id)) {
				return p->faction;
			}

			return std::nullopt;
		}();

		const auto game_screen_top = mode_in.game_screen_top + 2;

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

		augs::vertex_triangle_buffer avatar_triangles;

		auto draw_death_summary = [&]() {
			const auto viewed_player_id = spectator.show ? spectator.now_spectating : local_player_id;
			const auto viewed_player_data = typed_mode.find(viewed_player_id);

			const auto window_padding = vec2i(16, 16);
			const auto item_padding = window_padding;//vec2i(10, 10);

			if (!show_death_summary_for_teammates) {
				if (viewed_player_id != local_player_id) {
					return;
				}
			}

			if (viewed_player_data == nullptr) {
				return;
			}

			const auto& cosm = mode_input.cosm;

			const auto viewed_player_handle = cosm[viewed_player_data->controlled_character_id];

			if (viewed_player_handle.dead()) {
				return;
			}

			const auto viewed_sentience = viewed_player_handle.template find<components::sentience>();

			if (viewed_sentience == nullptr) {
				return;
			}

			{
				const bool is_dead = viewed_sentience->is_dead();

				if (!is_dead) {
					return;
				}
			}

			mode_player_id killer_player_id;
			damage_origin killer_origin;
			std::string killer_name;

			{
				const auto& kos = typed_mode.get_current_round().knockouts;

				for (const auto& k : reverse(kos)) {
					if (k.victim.id == viewed_player_id) {
						killer_player_id = k.knockouter.id;
						killer_origin = k.origin;
						killer_name = k.knockouter.name;
						break;
					}
				}
			}

			if (!killer_player_id.is_set()) {
				return;
			}

			const bool is_suicide = viewed_player_id == killer_player_id;

			mode_player_id tool_owner;

			const auto tool_name = killer_origin.on_tool_used(cosm, [&](const auto& tool) -> std::string {
				if constexpr(is_spell_v<decltype(tool)>) {
					return tool.appearance.name;
				}
				else {
					return tool.get_name();
				}
			});

			auto try_get_owner_from = [&](const auto& entity) {
				if (tool_owner.is_set()) {
					return;
				}

				if (const auto tool_entity = cosm[entity]) {
					if (const auto tool_item = tool_entity.template find<components::item>()) {
						const auto tool_owner_entity = tool_item.get_owner_meta().original_owner;
						tool_owner = typed_mode.lookup(tool_owner_entity);
					}
				}
			};

			try_get_owner_from(killer_origin.cause.entity);
			try_get_owner_from(killer_origin.sender.direct_sender);

			const bool is_weapon_your_own = tool_owner == viewed_player_id;
			const bool is_weapon_killers = tool_owner == killer_player_id;

			auto larger_colored = [&](const auto& text, const auto& c) {
				const auto text_style = style(
					in.gui_fonts.larger_gui,
					c
				);

				return formatted_string(text, text_style);
			};

			const auto killed_you_str = is_suicide ? "killed yourself" : "killed you with";
			const auto owner_str = [&]() -> std::string {
				if (is_weapon_your_own) {
					return "your own";
				}

				if (is_weapon_killers) {
					return "their";
				}

				if (tool_owner.is_set()) {
					if (const auto owner_data = typed_mode.find(tool_owner)) {
						return owner_data->chosen_name + "'s";
					}
				}

				return is_suicide ? "your own" : "their";
			}();

			const auto killed_color = rgba(255, 50, 50, 255);

			auto total_subtext = colored(std::string("  ") + killed_you_str, killed_color);

			if (tool_name) {
				total_subtext += colored(" with ", killed_color);

				if (owner_str.size() > 0) {
					total_subtext += colored(owner_str + " ", killed_color);
				}

				total_subtext += colored(*tool_name, cyan);
			}

			total_subtext += colored("\n", white);

			auto get_stat_text = [&](const auto preffix, const damage_owner& o) {
				const auto text_col = rgba(150, 150, 150, 255);
				const auto stat_col = white;

				auto total_stats_text = colored(preffix, text_col);

				total_stats_text += colored(typesafe_sprintf("%x dmg ", static_cast<int>(o.applied_damage)), stat_col);
				total_stats_text += colored("in ", text_col);
				total_stats_text += colored(typesafe_sprintf(o.hits == 1 ? "%x hit" : "%x hits", o.hits), stat_col);
				total_stats_text += colored(", ", text_col);
				total_stats_text += colored(typesafe_sprintf("%x HP", static_cast<int>(o.hp_loss)), stat_col);
				total_stats_text += colored(" loss, ", text_col);
				total_stats_text += colored(typesafe_sprintf("%x PE", static_cast<int>(o.pe_loss)), stat_col);
				total_stats_text += colored(" loss.", text_col);

				return total_stats_text;
			};

			if (const auto killer_character_id = typed_mode.lookup(killer_player_id); killer_character_id.is_set()) {
				for (const auto& o : viewed_sentience->damage_owners) {
					if (o.who == killer_character_id) {
						total_subtext += get_stat_text("\nTaken: ", o);
					}
				}

				if (const auto killer_character = cosm[killer_character_id]) {
					if (const auto killer_sentience = killer_character.template find<components::sentience>()) {
						for (const auto& o : killer_sentience->damage_owners) {
							if (o.who == viewed_player_handle) {
								total_subtext += get_stat_text("\nGiven: ", o);
							}
						}
					}
				}
			}

			const auto& killer_nickname_text = larger_colored((is_suicide ? std::string("YOU") : killer_name) + "\n", white);
			const auto killer_nickname_text_size = get_text_bbox(killer_nickname_text);

			const auto total_text = killer_nickname_text + total_subtext;
			const auto total_text_size = get_text_bbox(total_text);

			const auto free_w_for_tool = total_text_size.x - killer_nickname_text_size.x;

			const auto tool_image_id = killer_origin.on_tool_used(cosm, [&](const auto& tool) -> assets::image_id {
				if constexpr(is_spell_v<decltype(tool)>) {
					return tool.appearance.icon;
				}
				else {
					return tool.get_image_id();
				}
			});

			const auto& tool_image_entry = tool_image_id != std::nullopt ? in.images_in_atlas.at(*tool_image_id).diffuse : augs::atlas_entry();

			const bool tool_image_drawn = tool_image_entry.exists();
			const auto tool_image_size = static_cast<vec2i>(tool_image_entry.get_original_size());

			const auto w_for_tool_image = [&]() {
				if (tool_image_drawn) {
					const auto requested_w = tool_image_size.x;

					if (requested_w > free_w_for_tool) {
						return requested_w - free_w_for_tool;
					}
				}

				return 0;
			}();

			const auto& avatar_atlas_entry = in.avatars_in_atlas.at(killer_player_id.value); 
			const bool avatars_enabled = in.general_atlas != std::nullopt && in.avatar_atlas != std::nullopt;
			const bool avatar_displayed = avatar_atlas_entry.exists() && avatars_enabled;

			const auto displayed_avatar_size = vec2i::square(avatar_displayed ? max_avatar_side_v : 0);
			
			const auto predicted_size = 
				vec2i(
					window_padding.x * 2 + displayed_avatar_size.x + total_text_size.x + item_padding.x + w_for_tool_image,
					window_padding.x * 2 + std::max(displayed_avatar_size.y, total_text_size.y)
				)
			;
			
			/* if (tool_image_drawn) { */
			/* 	predicted_size.x += item_padding.x + tool_image_size.x; */
			/* } */

			const auto off_mult = cfg.death_summary_offset_mult;
			const auto popup_lt = vec2(in.screen_size.x / 2 - predicted_size.x / 2, in.screen_size.y * off_mult);
			const auto window_bg_rect = ltrb(popup_lt, predicted_size);

			// TODO give it its own settings struct
			{
				const auto& cfg = in.config.arena_mode_gui.scoreboard_settings;

				in.drawer.aabb_with_border(window_bg_rect, cfg.background_color, killed_color);
			}

			if (avatar_displayed) {
				const auto avatar_orig = ltrbi(popup_lt + window_padding, displayed_avatar_size);

				//const auto& cfg = in.config.arena_mode_gui.scoreboard_settings;
				//in.drawer.border(avatar_orig, cfg.border_color);

				auto avatar_output = augs::drawer { avatar_triangles };
				avatar_output.aabb(avatar_atlas_entry, avatar_orig, white);
			}

			const auto total_text_pos = popup_lt + window_padding + vec2i(item_padding.x + displayed_avatar_size.x, 0);

			print_stroked(
				in.drawer,
				total_text_pos,
				total_text
			);

			if (tool_image_drawn) {
				const auto tool_image_padding = window_padding;
				const auto tool_window_size = tool_image_size + tool_image_padding * 2;
				const auto tool_window_orig = ltrbi(window_bg_rect.right_top() + vec2i(-tool_window_size.x, 0), tool_window_size);

				// TODO give it its own settings struct
				{
					//const auto& cfg = in.config.arena_mode_gui.scoreboard_settings;
					//in.drawer.aabb_with_border(tool_window_orig, cfg.background_color, cfg.border_color);
				}

				const auto tool_image_orig = ltrbi(tool_window_orig.left_top() + tool_image_padding, tool_image_size);
				in.drawer.base::aabb(tool_image_entry, tool_image_orig, white);
			}

			if (avatar_triangles.size() > 0) {
				in.renderer.call_and_clear_triangles();

				auto previous_texture = augs::graphics::texture::find_current();

				in.avatar_atlas->set_as_current();
				in.renderer.call_triangles(avatar_triangles);

				augs::graphics::texture::set_current_to(previous_texture);
			}
		};

		auto draw_scoreboard = [&]() {
			scoreboard.draw_gui(in, mode_in, typed_mode, mode_input);
		};

		auto draw_spectator = [&]() {
			spectator.draw_gui(in, mode_in, typed_mode, mode_input);
		};

		auto draw_money_and_awards = [&]() {
			// TODO: fix this for varying icon sizes

			if (const auto p = typed_mode.find(local_player_id)) {
				const auto& stats = p->stats;

				auto drawn_current_money = stats.money;

				const auto money_indicator_pos = augs::get_screen_pos_from_offset(in.screen_size, cfg.money_indicator_pos, game_screen_top); 
				
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
			else {
				warmup.requested.clear();
				warmup.current.clear();
			}
		};

		auto draw_knockouts = [&]() {
			const auto& cosm = mode_input.cosm;
			const auto& kos = typed_mode.get_current_round().knockouts;
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

				auto get_col = [&](const auto& participant) {
					return in.config.faction_view.colors[participant.faction].standard;
				};

				auto get_name = [&](const auto& entry) -> const auto& {
					return entry.name;
				};

				const auto& knockouter = get_name(ko.knockouter);
				const auto& assist = get_name(ko.assist);
				const auto& victim = get_name(ko.victim);

				const bool is_suicide = ko.knockouter.id == ko.victim.id;

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
					if (local_player_id == ko.victim.id) {
						return { rgba(150, 0, 0, 170), rgba::zero };
					}

					if (local_player_id == ko.knockouter.id || local_player_id == ko.assist.id) {
						return { black, rgba(180, 0, 0, 255) };
					}
					
					return { { 0, 0, 0, 80 }, rgba::zero };
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

				const auto death_fallback_icon = in.images_in_atlas.at(mode_input.rules.view.icons[scoreboard_icon_type::DEATH_ICON]);
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
					{ augs::ralign::CY }
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
					{ augs::ralign::CY }
				);

				pen.y = total_bbox.b();
			}
		};

		auto draw_text_indicator_at = [&](const auto& val, const auto t) {
			const auto s = in.screen_size;

			print_stroked(
				in.drawer,
				{ s.x / 2, static_cast<int>(t) },
				val,
				{ augs::ralign::CX }
			);
		};

		auto play_tick_if_soon = [&](const auto& val, const auto& when_ticking_starts, const bool alarm) {
			auto play_tick = [&]() {
				auto& vol = in.config.audio_volume;

				static augs::sound_source tick_sound;

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
			draw_text_indicator_at(colored(val, col), game_screen_top);
		};

		auto draw_info_indicator = [&](const auto& val) {
			const auto one_sixth_t = in.screen_size.y / 6;
			draw_text_indicator_at(val, one_sixth_t);
		};

		auto draw_game_commencing = [&]() {
			if (auto commencing_left = typed_mode.get_commencing_left_ms(); commencing_left != -1.f) {
				// const auto c = std::ceil(commencing_left);
				draw_info_indicator(colored("Game Commencing!", white));
				return true;
			}

			return false;
		};

		/* Synonym */
		auto draw_warmup_indicator = draw_info_indicator;

		const auto warmup_left = typed_mode.get_warmup_seconds_left(mode_input);
		const bool now_is_warmup = warmup_left > 0.f;

		auto draw_warmup_timer = [&]() {
			const auto c = std::ceil(warmup_left);

			play_tick_if_soon(c, 5.f, true);
			draw_warmup_indicator(colored("WARMUP\n" + format_mins_secs(c), white));
		};

		auto draw_warmup_welcome = [&]() {
			const auto& original_welcome = mode_input.rules.view.warmup_welcome_message;
			const bool non_spectator = local_player_faction && *local_player_faction != faction_type::SPECTATOR;

			if (non_spectator && !original_welcome.empty()) {
				auto& last = warmup.last_seconds_value;

				thread_local auto populator = default_popul;

				if (!last || warmup_left > *last || warmup.requested.empty()) {
					last = std::nullopt;

					warmup.requested = augs::gui::text::from_bbcode(original_welcome, style(in.gui_fonts.gui, white));
					warmup.current.clear();

					populator = default_popul;
					populator.on_enter(warmup.requested);
				}

				const auto dt_secs = last.has_value() ? *last - warmup_left : 0.f;
				const auto dt_ms = dt_secs * 1000.f;
				const auto dt = augs::delta::from_milliseconds(dt_ms);

				{
					populator.on_update(warmup.current, warmup.requested, dt);

					const auto one_fourth_t = in.screen_size.y / 6;
					const auto t = one_fourth_t + line_height * 3;
					const auto s = in.screen_size;
					const auto target_pos = vec2i { s.x / 2, static_cast<int>(t) };

					auto target = warmup.current;

					auto alpha_mult = 1.f;

					if (!populator.is_complete()) {
						target += colored("|", white);
					}
					else if (warmup.completed_at_secs > 0.f) {
						const auto fade_delay = 5.f;
						const auto diff = warmup.completed_at_secs - warmup_left;

						if (diff > fade_delay) {
							const auto fade_duration = 5.f;
							const auto mult = 1.f - std::min(1.f, (diff - fade_delay) / fade_duration);
							alpha_mult = mult;
							target.mult_alpha(mult);
						}
					}
					else {
						warmup.completed_at_secs = warmup_left;
					}

					print_stroked(
						in.drawer,
						target_pos,
						target,
						{ augs::ralign::CX },
						rgba(black).mult_alpha(alpha_mult)
					);
				}

				last = warmup_left;
			}
		};

		auto draw_important_match_event = [&]() {
			if (const auto secs = typed_mode.get_seconds_since_planting(mode_input); secs >= 0.f && secs <= 3.f) {
				draw_info_indicator(colored("The bomb has been planted!", yellow));
				return true;
			}

			const auto& win = typed_mode.get_current_round().last_win;

			if (win.was_set()) {
				auto indicator_text = colored(format_enum(win.winner) + " wins!", yellow);

				if (typed_mode.get_state() == arena_mode_state::MATCH_SUMMARY) {
					indicator_text += colored(typed_mode.is_halfway_round(mode_input) ? "\n\nHalftime\n" : "\n\nIntermission\n", white);

					{
						const auto summary_secs_left = typed_mode.get_match_summary_seconds_left(mode_input);
						const auto c = std::ceil(summary_secs_left);

						indicator_text += colored(format_mins_secs(c), white);
					}
				}

				draw_info_indicator(indicator_text);

				return true;
			}

			return false;
		};

		auto draw_match_timers = [&]() {
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
		};

		if (prediction.play_unpredictable) {
			if (cfg.show_client_resyncing_notifier && resyncing_notifier) {
				const auto warning = colored("WARNING! Resynchronizing client with the server.", orange);

				const auto where = in.screen_size.y / 8;
				draw_text_indicator_at(warning, where);
			}

			draw_death_summary();
			draw_scoreboard();

			if (!choose_team.show) {
				draw_spectator();
			}

			const bool draw_money = [&]() {
				if (mode_input.rules.hide_details_when_spectating_enemies) {
					if (local_player_faction && *local_player_faction != faction_type::SPECTATOR) {
						const auto viewed_player_id = spectator.show ? spectator.now_spectating : local_player_id;
						const auto viewed_player_data = typed_mode.find(viewed_player_id);

						if (viewed_player_data == nullptr) {
							return false;
						}

						return viewed_player_data->faction == *local_player_faction;
					}
				}

				return true;
			}();

			if (draw_money) {
				draw_money_and_awards();
			}

			draw_knockouts();

			if (now_is_warmup) {
				draw_warmup_welcome();
			}
			else {
				warmup.requested.clear();
				warmup.current.clear();
			}

			draw_important_match_event();
		}

		if (prediction.play_predictable) {
			if (draw_game_commencing()) {
				return;
			}

			if (now_is_warmup) {
				draw_warmup_timer();
			}
			else {
				warmup.requested.clear();
				warmup.current.clear();

				draw_match_timers();
			}
		}
	}
	else {
		(void)typed_mode;
		(void)in;
	}
}

bool arena_gui_state::requires_cursor() const {
	return choose_team.show || buy_menu.show;
}

template void arena_gui_state::draw_mode_gui(
	const draw_setup_gui_input&,
	const draw_mode_gui_input&,
	const test_mode&, 
	const test_mode::const_input&,
	prediction_input
) const;

template void arena_gui_state::draw_mode_gui(
	const draw_setup_gui_input&,
	const draw_mode_gui_input&,
	const bomb_mode&, 
	const bomb_mode::const_input&,
	prediction_input
) const;

template mode_player_entropy arena_gui_state::perform_imgui(
	draw_mode_gui_input, 
	const bomb_mode&, 
	const typename bomb_mode::const_input&,
	prediction_input
);

template mode_player_entropy arena_gui_state::perform_imgui(
	draw_mode_gui_input, 
	const test_mode&, 
	const typename test_mode::const_input&,
	prediction_input
);
