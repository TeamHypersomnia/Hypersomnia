#include "augs/log.h"
#include "augs/drawing/drawing.hpp"
#include "augs/gui/text/printer.h"
#include "view/mode_gui/arena/arena_scoreboard_gui.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/modes/arena_mode.hpp"
#include "game/modes/test_mode.h"
#include "application/setups/draw_setup_gui_input.h"
#include "application/config_lua_table.h"
#include "game/modes/mode_helpers.h"
#include "view/viewables/images_in_atlas_map.h"
#include "augs/string/format_enum.h"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "augs/templates/logically_empty.h"

bool arena_scoreboard_gui::control(general_gui_intent_input in) {
	using namespace augs::event;
	using namespace augs::event::keys;

	const auto ch = in.e.get_key_change();

	if (ch != key_change::NO_CHANGE) {
		const auto key = in.e.get_key();

		if (const auto it = mapped_or_nullptr(in.controls, key)) {
			if (*it == general_gui_intent_type::SCOREBOARD) {
				show = ch == key_change::PRESSED;
				return true;
			}
		}
	}

	return false;
}

template <class M>
void arena_scoreboard_gui::draw_gui(
	const draw_setup_gui_input& in,
	const draw_mode_gui_input& draw_in,

	const M& typed_mode, 
	const typename M::const_input& mode_input
) const {
	using namespace augs::gui::text;

	bool force_show = false;
	bool is_halftime = false;

	if constexpr(!std::is_same_v<M, test_mode>) {
		if (typed_mode.get_state() == arena_mode_state::MATCH_SUMMARY) {
			force_show = true;
		}

		is_halftime = typed_mode.is_match_summary() && typed_mode.is_halfway_round(mode_input);;
	}

	const bool should_show = show || force_show;

	if (!should_show) {
		return;
	}

	const auto& cfg = in.config.arena_mode_gui.scoreboard_settings;

	const auto content_pad = cfg.window_padding;
	const auto cell_pad = cfg.player_row_inner_padding;

	const auto& o = in.get_drawer();

	auto fmt = [&](const auto& text, const rgba col = white) {
		return formatted_string(text, style(in.gui_fonts.gui, col));
	};

	auto fmt_large = [&](const auto& text, const rgba col = white) {
		return formatted_string(text, style(in.gui_fonts.very_large_numbers, col));
	};

	auto calc_size = [&](const auto& text) {
		if constexpr(std::is_same_v<const formatted_string&, decltype(text)>) { 
			return get_text_bbox(text);
		}
		else {
			return get_text_bbox(fmt(text));
		}
	};

	const auto font_h = in.gui_fonts.gui.metrics.get_height();
	const auto cell_h = font_h + cell_pad.y * 2;

	const auto window_name = "Scoreboard";
	const auto window_name_size = calc_size(window_name);

	const int num_participating_factions = 2;

	auto estimated_window_height = [&]() {
		auto num_players = uint32_t(0);

		for (auto& p : typed_mode.get_players()) {
			if (!p.second.should_hide_in_scoreboard()) {
				++num_players;
			}
		}

		const auto player_cells_h = cell_h * num_players;
		const auto headline_cells_h = cell_h * 3 * num_participating_factions;

		const auto num_spectators = typed_mode.num_players_in(faction_type::SPECTATOR);

		const auto separating_cells_h = cell_h * ((num_spectators > 0 ? 1 : 0) + 1);
		const auto column_labels_cells_h = cell_h * ((num_spectators > 0 ? 1 : 0) + 1);

		return 
			player_cells_h + 
			headline_cells_h + 
			separating_cells_h + 
			column_labels_cells_h +
			window_name_size.y +
			content_pad.y * 2
		;
	}();

	const auto window_bg_rect = ltrbi::center_and_size(in.screen_size / 2, vec2i(in.screen_size.x * 0.6f, estimated_window_height));
	o.aabb_with_border(window_bg_rect, cfg.background_color, cfg.border_color);

	const auto sz = window_bg_rect.get_size() - content_pad * 2;
	const auto lt = window_bg_rect.left_top() + content_pad;

	vec2i pen = lt;

	auto text = [&](const auto& ss, const rgba col, vec2i where, auto... args) {
		where += pen;

		print(
			o,
			where,
			fmt(ss, col),
			args...
		);
	};
	(void)text;

	auto text_stroked = [&](const auto& ss, const rgba col, vec2i where, augs::ralign_flags flags = {}) {
		where += pen;

		auto stroke_col = black;
		stroke_col.mult_alpha(cfg.text_stroke_lumi_mult);

		print_stroked(
			o,
			where,
			fmt(ss, col),
			flags,
			stroke_col
		);
	};

	auto text_stroked_large = [&](const auto& ss, const rgba col, vec2i where, augs::ralign_flags flags = {}) {
		where += pen;

		auto stroke_col = black;

		print_stroked(
			o,
			where,
			fmt_large(ss, col),
			flags,
			stroke_col
		);
	};


	{
		text_stroked(window_name, white, { sz.x / 2 - window_name_size.x / 2, 0 });
	}

	pen.y += window_name_size.y;

	struct column {
		int w;
		std::string label;
		bool align_right = false;

		column(
			int w, 
			std::string s,
			bool align_right = false
		) : w(w), label(s), align_right(align_right) {}

		int l;
		int r;
	};

	const auto scoreboard_avatar_icon_side = static_cast<int>(font_h);

	std::vector<column> columns = {
		{ calc_size("9999").x, "Ping", true },
		{ 22, " " },
		{ scoreboard_avatar_icon_side, " " },
		{ sz.x, "Player" },
		{ calc_size("999999$").x, typed_mode.levelling_enabled(mode_input) ? "Level" : "Money", true },

		{ calc_size("999").x, "K", true },
		{ calc_size("999").x, "A", true },
		{ calc_size("999").x, "D", true },

		{ calc_size("999999").x, "Score", true },
	};

	pen.y += cell_h;

	auto& player_col = columns[3];

	for (auto& c : columns) {
		if (&c != &player_col) {
			c.w += cell_pad.x * 2;
			player_col.w -= c.w;
		}
	}

	{
		int ll = 0;

		for (auto& c : columns) {
			c.l = ll;
			c.r = c.l + c.w;

			ll = c.r;
		}
	}

	auto print_col_text = [&](const column& c, const auto& text, const auto& col, bool force_right_align = false) {
		const auto& pp = cell_pad;

		if (force_right_align || c.align_right) {
			text_stroked(text, col, vec2i(c.r - pp.x - calc_size(text).x, pp.y));
		}
		else {
			text_stroked(text, col, vec2i(c.l + pp.x, pp.y));
		}
	};

	const auto column_label_color = gray;

	for (const auto& c : columns) {
		print_col_text(c, c.label, column_label_color);
	}

	pen.y += cell_h;

	auto aabb = [&](auto orig, rgba col) {
		orig.l += pen.x;
		orig.t += pen.y;
		orig.r += pen.x;
		orig.b += pen.y;
		col.a *= cfg.cell_bg_alpha;

		o.aabb(orig, col);
	};

	auto aabb_img = [&](auto img, auto orig, rgba col = white) {
		orig.l += pen.x;
		orig.t += pen.y;
		orig.r += pen.x;
		orig.b += pen.y;

		o.base::aabb(img, orig, col);
	};

	auto& avatar_triangles = in.renderer.dedicated[augs::dedicated_buffer::AVATARS].triangles;
	auto& color_indicator_triangles = in.renderer.dedicated[augs::dedicated_buffer::SCOREBOARD_COLOR_INDICATORS].triangles;

	auto avatar_aabb_img = [&](auto img, auto orig, rgba col = white) {
		orig.l += pen.x;
		orig.t += pen.y;
		orig.r += pen.x;
		orig.b += pen.y;

		auto avatar_output = augs::drawer { avatar_triangles };
		avatar_output.aabb(img, orig, col);
	};

	auto color_indicator_aabb_img = [&](auto img, auto orig, rgba col = white) {
		orig.l += pen.x;
		orig.t += pen.y;
		orig.r += pen.x;
		orig.b += pen.y;

		auto color_indicator_output = augs::drawer { color_indicator_triangles };
		color_indicator_output.aabb(img, orig, col);
	};

	const auto max_score = typed_mode.calc_max_faction_score();

	auto get_nickname_str = [&](const auto& player_id, const auto& player_data) {
		auto str = player_data.get_nickname();

		if (in.player_metas) {
			const auto progress = (*in.player_metas)[player_id.value].stats.download_progress;

			if (progress != 255) {
				const float percent = float(progress) / 255.0f;

				str += typesafe_sprintf(" (downloading: %2f", 100 * percent) + "%)";
			}
		}

		return str;
	};

	auto print_faction = [&](const faction_type faction, const bool on_top) {
		(void)on_top;
		auto& colors = in.config.faction_view.colors[faction];

		const auto& cosm = mode_input.cosm;

		std::vector<std::pair<typename M::player_type, mode_player_id>> sorted_players;

		typed_mode.for_each_player_in(faction, [&](
			const auto& id, 
			const auto& player
		) {
			if (player.should_hide_in_scoreboard()) {
				return;
			}

			sorted_players.emplace_back(player, id);
		});

		sort_range(sorted_players);

		const auto& bg_dark = colors.background_dark;

		const auto faction_score = typed_mode.get_faction_score(faction);

		auto draw_headline = [&]() {
			const auto bg_height = cell_h * 3;
			const auto faction_bg_orig = ltrbi(vec2i::zero, vec2i(sz.x, bg_height));

			if (cfg.dark_color_overlay_under_score) {
				aabb(faction_bg_orig, bg_dark);
			}

			if constexpr(!std::is_same_v<M, test_mode>) {
				if (typed_mode.is_match_summary()) {
					const auto match_result = typed_mode.calc_match_result(mode_input);
					const bool tied = match_result.is_tie();
					auto label = std::string(tied ? "TIED" : "WON");

					if (is_halftime) {
						label += " THE FIRST HALF";
					}
					else {
						label += " THE MATCH";
					}

					auto text_pos = faction_bg_orig.get_center();

					if (tied) {
						text_pos -= vec2i(0, faction_bg_orig.get_size().y / 2);
					}

					auto do_draw = [&]() {
						text_stroked(label, yellow, text_pos, { augs::ralign::CX, augs::ralign::CY });
					};

					if (tied) {
						if (!on_top) {
							do_draw();
						}
					}
					else {
						if (match_result.winner == faction) {
							do_draw();
						}
					}
				}
			}

			auto prev_pen_x = pen.x;

			pen.x += cell_pad.x * 2;

			if constexpr(!std::is_same_v<M, test_mode>) {
				if (sorted_players.size() > 0) {
					if (const auto& head_image = mode_input.rules.view.logos[faction]; head_image.is_set()) {
						if (const auto& entry = draw_in.images_in_atlas.find_or(head_image).diffuse; entry.exists()) {
							const auto size = entry.get_original_size();
							auto head_orig = ltrbi(vec2i::zero, size).place_in_center_of(faction_bg_orig);

							head_orig.l = 0;
							head_orig.r = size.x;

							/* if (!on_top) { */
							/* 	head_orig.t = faction_bg_orig.b - size.y - cell_pad.y * 2; */
							/* 	head_orig.b = head_orig.t + size.y; */
							/* } */
							/* else { */
							/* 	head_orig.t = faction_bg_orig.t + cell_pad.y * 2; */
							/* 	head_orig.b = head_orig.t + size.y; */
							/* } */

							aabb_img(entry, head_orig, rgba(white).mult_alpha(cfg.faction_logo_alpha));

							pen.x += size.x + cell_pad.x * 2;
						}
					}
				}
			}

			const auto score_text_max_w = calc_size(fmt_large((max_score >= 10 ? "99" : "9"))).x;
			auto score_text = typesafe_sprintf("%x", faction_score);

			if (max_score >= 10 && score_text.size() == 1) {
				score_text = "0" + score_text;
			}

			//const auto score_text_size = calc_size(fmt_large(score_text));

			const auto score_text_pos = vec2i(cell_pad.x, faction_bg_orig.get_center().y);
			//+ score_text_max_w - score_text_size.x

			text_stroked_large(score_text, colors.standard, score_text_pos, { augs::ralign::CY });

			//const auto score_text_r = score_text_pos.x + score_text_size.x;

			pen.x += score_text_max_w + cell_pad.x * 4;
			//head_orig.r + cell_pad.x * 2

			text_stroked(std::string("for ") + format_enum(faction), colors.standard, vec2i { 0, faction_bg_orig.get_center().y } , { augs::ralign::CY });

			pen.y += bg_height;
			pen.x = prev_pen_x;
		};

		if (!on_top) {
			draw_headline();
		}

		const auto h = sorted_players.size() * cell_h;

		for (const auto& c : columns) {
			const auto col_border_orig = ltrbi(c.l, 0, c.l + 1, h - 1);
			aabb(col_border_orig, bg_dark);
		}

		const bool avatars_enabled = logically_set(in.general_atlas, in.avatar_atlas);

		for (const auto& p : sorted_players) {
			const auto& player_id = p.second;
			const auto& player_data = p.first;

			const auto player_handle = cosm[player_data.controlled_character_id];
			const auto is_conscious = player_handle.alive() && player_handle.template get<components::sentience>().is_conscious();

			const auto local_player_faction = [&]() {
				if (const auto p = typed_mode.find(draw_in.local_player_id)) {
					return p->get_faction();
				}

				return faction_type::SPECTATOR;
			}();

			const bool is_viewer_faction = faction == local_player_faction;

			const auto faction_bg_col = [&]() {
				auto col = colors.standard;

				auto total_lumi = cfg.bg_lumi_mult;
				auto total_alpha = 1.f;

				const bool is_local = player_id == draw_in.local_player_id;

				if (is_local) {
					col.mult_luminance(cfg.current_player_bg_lumi_mult);

					if (!is_conscious) {
						col.mult_alpha(cfg.dead_player_bg_alpha_mult);
					}

					return col;
				}
				else {
					if (!is_conscious) {
						return bg_dark;
						//return is_local ? rgba(bg_dark).multiply_rgb(1.5f) : bg_dark;
						col = bg_dark;
						total_lumi = 1.f;
						total_alpha = 1.f;
						/* total_lumi *= cfg.dead_player_bg_lumi_mult; */
						/* total_alpha *= cfg.dead_player_bg_alpha_mult; */
					}
				}

				return col.multiply_rgb(total_lumi).mult_alpha(total_alpha);
			}();	

			const auto faction_text_col = [&]() {
				auto col = colors.standard;

				auto total_lumi = cfg.text_lumi_mult;
				auto total_alpha = 1.f;

				if (player_id == draw_in.local_player_id) {
					//return white;
					total_lumi *= cfg.current_player_text_lumi_mult;
				}

				if (!is_conscious) {
					total_alpha *= cfg.dead_player_text_alpha_mult;
					total_lumi *= cfg.dead_player_text_lumi_mult;
				}

				return col.mult_luminance(total_lumi).mult_alpha(total_alpha);
			}();	

			for (const auto& c : columns) {
				const auto cell_body_origin = ltrbi(c.l + 1, 1, c.r, cell_h);
				aabb(cell_body_origin, faction_bg_col);

				const auto cell_upper_border_origin = ltrbi(c.l + 1, 0, c.r, 1);
				aabb(cell_upper_border_origin, bg_dark);
			}

			auto* current_column = columns.data();

			auto next_col = [&]() {
				++current_column;
			};

			auto col_text = [&](const auto& text) {
				print_col_text(*current_column, text, faction_text_col);
			};

			if (in.player_metas != nullptr) {
				const auto ping = (*in.player_metas)[player_id.value].stats.ping;
				auto ping_str = typesafe_sprintf("%x", ping);

				if (ping >= 0) {
					if (ping == 0) {
						ping_str = "Host";
					}

					if (ping >= 255) {
						ping_str = ">254";
					}

					col_text(ping_str);
				}
			}

			next_col();

			if (player_handle.alive()) {
				using S = scoreboard_icon_type;
				std::optional<S> icon;

				auto set_if_more_important = [&](const auto new_icon) {
					if (icon == std::nullopt) {
						icon = new_icon;
						return;
					}
					
					using U = std::underlying_type_t<S>;
					
					if (static_cast<U>(new_icon) < static_cast<U>(*icon)) {
						*icon = new_icon;
					}
				};

				const auto& sentience = player_handle.template get<components::sentience>();

				if (sentience.unconscious_but_alive()) {
					set_if_more_important(S::UNCONSCIOUS_ICON);
				}

				if (sentience.is_dead()) {
					set_if_more_important(S::DEATH_ICON);
				}

				player_handle.for_each_contained_item_recursive(
					[&](const auto& typed_item) {
						if (const auto tool = typed_item.template find<invariants::tool>()) {
							if (tool->defusing_speed_mult > 1.f) {
								set_if_more_important(S::DEFUSE_KIT_ICON);
							}
						}

						if (const auto hand_fuse = typed_item.template find<invariants::hand_fuse>()) {
							if (hand_fuse->is_like_plantable_bomb()) {
								set_if_more_important(S::BOMB_ICON);
							}
						}
					}
				);

				if constexpr(!std::is_same_v<M, test_mode>) {
					auto show_icon = [&]() {
						if (icon.has_value()) {
							const auto icon_image = mode_input.rules.view.icons.at(*icon);

							if (const auto& entry = draw_in.images_in_atlas.find_or(icon_image).diffuse; entry.exists()) {
								const auto size = entry.get_original_size();

								const auto& c = *current_column;
								const auto& cell_orig = ltrbi(c.l, 0, c.r, cell_h);

								auto icon_orig = ltrbi(vec2i::zero, size);
								icon_orig.place_in_center_of(cell_orig);

								auto total_alpha = cfg.icon_alpha;
									
								if (!is_conscious) {
									total_alpha *= cfg.dead_player_text_alpha_mult;
								}

								aabb_img(
									entry,
									icon_orig,
									rgba(white).mult_alpha(total_alpha)
								);
							}
						}
					};

					if (mode_input.rules.view.show_info_icons_of_opponents) {
						show_icon();
					}
					else {
						if (is_viewer_faction) {
							show_icon();
						}
						else {
							if (
								icon == scoreboard_icon_type::DEATH_ICON
								|| icon == scoreboard_icon_type::UNCONSCIOUS_ICON
							) {
								show_icon();
							}
						}
					}
				}
			}

			next_col();

			if (avatars_enabled) {
				if (const auto entry = in.avatars_in_atlas.at(player_id.value); entry.exists()) {
					const auto& c = *current_column;
					const auto& cell_orig = ltrbi(c.l + 1, 1, c.r, cell_h);

					auto total_alpha = cfg.avatar_alpha;
						
					if (!is_conscious) {
						total_alpha *= cfg.dead_player_text_alpha_mult;
					}

					avatar_aabb_img(
						entry,
						cell_orig,
						rgba(white).mult_alpha(total_alpha)
					);
				}
			}

			if constexpr(!std::is_same_v<M, test_mode>) {
				if (is_viewer_faction && mode_input.rules.enable_player_colors) {
					const auto color_indicator_image = assets::necessary_image_id::SMALL_COLOR_INDICATOR;

					if (const auto& entry = in.necessary_images.at(color_indicator_image); entry.exists()) {
						const auto size = entry.get_original_size();

						const auto& c = *current_column;

						auto icon_orig = ltrbi(vec2i::zero, size);
						icon_orig.l = (c.l + c.r) / 2 - size.x / 2;
						icon_orig.r = icon_orig.l + size.x;
						icon_orig.t++;
						icon_orig.b++;

						auto total_alpha = cfg.icon_alpha;

						if (!is_conscious) {
							total_alpha *= cfg.dead_player_text_alpha_mult;
						}

						color_indicator_aabb_img(
							entry,
							icon_orig,
							rgba(player_data.assigned_color).mult_alpha(total_alpha)
						);
					}
				}
			}

			next_col();
			col_text(get_nickname_str(player_id, player_data));
			next_col();

			const auto& stats = player_data.stats;

			if constexpr(!std::is_same_v<M, test_mode>) {
				auto do_money = [&]() {
					if (typed_mode.levelling_enabled(mode_input)) {
						col_text(typesafe_sprintf("%x", stats.level));
					}
					else {
						col_text(typesafe_sprintf("%x$", stats.money));
					}
				};

				if (draw_in.demo_replay_mode || mode_input.rules.view.show_money_of_opponents) {
					do_money();
				}
				else {
					if (is_viewer_faction) {
						do_money();
					}
				}
			}

			next_col();
			col_text(typesafe_sprintf("%x", stats.knockouts));
			next_col();
			col_text(typesafe_sprintf("%x", stats.assists));
			next_col();
			col_text(typesafe_sprintf("%x", stats.deaths));
			next_col();
			col_text(typesafe_sprintf("%x", stats.calc_score()));

			pen.y += cell_h;
		}

		if (on_top) {
			draw_headline();
		}
	};

	if constexpr(std::is_same_v<M, test_mode>) {
		print_faction(faction_type::METROPOLIS, true);
		print_faction(faction_type::RESISTANCE, false);
	}
	else {
		const auto participants = typed_mode.calc_participating_factions(mode_input);

		print_faction(participants.defusing, true);
		print_faction(participants.bombing, false);
	}

	{
		const auto faction = faction_type::SPECTATOR;

		std::vector<std::pair<typename M::player_type, mode_player_id>> sorted_players;

		typed_mode.for_each_player_in(faction, [&](
			const auto& id, 
			const auto& player
		) {
			sorted_players.emplace_back(player, id);
		});

		if (const auto num_spectators = sorted_players.size(); num_spectators > 0) {
			pen.y += cell_h;

			const auto bg_height = sorted_players.size() * cell_h;
			const auto faction_bg_orig = ltrbi(vec2i::zero, vec2i(sz.x, bg_height));

			auto& colors = in.config.faction_view.colors[faction];
			const auto& bg_dark = colors.background_dark;

			aabb(faction_bg_orig, bg_dark);

			print_col_text(columns[3], typesafe_sprintf("Spectators (%x)", num_spectators), column_label_color);

			pen.y += cell_h;

			for (const auto& p : sorted_players) {
				print_col_text(columns[3], get_nickname_str(p.second, p.first), column_label_color);
				pen.y += cell_h;
			}
		}
	}

	in.renderer.call_and_clear_triangles();

	if (avatar_triangles.size() > 0) {
		in.avatar_atlas->set_as_current(in.renderer);
		in.renderer.call_triangles(augs::dedicated_buffer::AVATARS);

		augs::graphics::texture::set_current_to_previous(in.renderer);
	}

	if (color_indicator_triangles.size() > 0) {
		in.renderer.call_triangles(augs::dedicated_buffer::SCOREBOARD_COLOR_INDICATORS);
	}
}


template void arena_scoreboard_gui::draw_gui(
	const draw_setup_gui_input&,
	const draw_mode_gui_input&, 

	const arena_mode& mode, 
	const typename arena_mode::const_input&
) const;

template void arena_scoreboard_gui::draw_gui(
	const draw_setup_gui_input&,
	const draw_mode_gui_input&, 

	const test_mode& mode, 
	const typename test_mode::const_input&
) const;
