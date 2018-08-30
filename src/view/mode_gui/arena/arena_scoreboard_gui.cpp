#include "augs/gui/text/printer.h"
#include "view/mode_gui/arena/arena_scoreboard_gui.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/modes/bomb_mode.hpp"
#include "application/setups/draw_setup_gui_input.h"
#include "application/config_lua_table.h"
#include "game/modes/mode_helpers.h"
#include "view/viewables/images_in_atlas_map.h"
#include "augs/string/format_enum.h"

bool arena_scoreboard_gui::control(
	const augs::event::state& common_input_state,
	const augs::event::change e
) {
	using namespace augs::event;
	using namespace augs::event::keys;

	(void)common_input_state;
	/* const bool has_ctrl{ common_input_state[key::LCTRL] }; */
	/* const bool has_shift{ common_input_state[key::LSHIFT] }; */

	if (e.was_pressed(key::TAB)) {
		show = true;
		return true;
	}

	if (e.was_released(key::TAB)) {
		show = false;
		return true;
	}

	return false;
}

template <class M>
void arena_scoreboard_gui::draw_gui(
	const draw_setup_gui_input& in,
	const draw_mode_gui_input& draw_in,

	const M& typed_mode, 
	const typename M::input& mode_input
) const {
	using namespace augs::gui::text;

	if (!show) {
		return;
	}

	const auto& cfg = in.config.arena_mode_gui.scoreboard_settings;

	const auto content_pad = cfg.window_padding;
	const auto cell_pad = cfg.player_row_inner_padding;

	const auto& o = in.drawer;

	const auto window_bg_rect = ltrbi(vec2i::zero, in.screen_size).expand_from_center_mult(0.6f);
	o.aabb_with_border(window_bg_rect, cfg.background_color, cfg.border_color);

	const auto sz = window_bg_rect.get_size() - content_pad * 2;
	const auto lt = window_bg_rect.left_top() + content_pad;

	auto fmt = [&](const auto& text, const rgba col = white) {
		return formatted_string(text, style(in.gui_fonts.gui, col));
	};

	auto fmt_large = [&](const auto& text, const rgba col = white) {
		return formatted_string(text, style(in.gui_fonts.large_numbers, col));
	};

	auto calc_size = [&](const auto& text) {
		if constexpr(std::is_same_v<const formatted_string&, decltype(text)>) { 
			return get_text_bbox(text);
		}
		else {
			return get_text_bbox(fmt(text));
		}
	};

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

	auto text_stroked = [&](const auto& ss, const rgba col, vec2i where, augs::center_flags flags = {}) {
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

	auto text_stroked_large = [&](const auto& ss, const rgba col, vec2i where, augs::center_flags flags = {}) {
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

	const auto window_name = "Scoreboard";
	const auto window_name_size = calc_size(window_name);

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

	std::vector<column> columns = {
		{ calc_size("9999").x, "Ping", true },
		{ 22, " " },
		{ sz.x, "Player" },
		{ calc_size("999999$").x, "Money", true },

		{ calc_size("999").x, "K", true },
		{ calc_size("999").x, "A", true },
		{ calc_size("999").x, "D", true },

		{ calc_size("999999").x, "Score", true },
	};

	const auto font_h = in.gui_fonts.gui.metrics.get_height();
	const auto cell_h = font_h + cell_pad.y * 2;

	pen.y += cell_h;

	auto& player_col = columns[2];

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

	auto print_col_text = [&](const column& c, const auto& text, const auto& col) {
		const auto& pp = cell_pad;

		if (c.align_right) {
			text_stroked(text, col, vec2i(c.r - pp.x - calc_size(text).x, pp.y));
		}
		else {
			text_stroked(text, col, vec2i(c.l + pp.x, pp.y));
		}
	};

	for (const auto& c : columns) {
		print_col_text(c, c.label, gray);
	}

	pen.y += cell_h;

	auto aabb = [&](auto orig, rgba col) {
		orig.l += pen.x;
		orig.t += pen.y;
		orig.r += pen.x;
		orig.b += pen.y;
		col.a *= cfg.elements_alpha;

		o.aabb(orig, col);
	};

	auto aabb_img = [&](auto img, auto orig, rgba col = white) {
		orig.l += pen.x;
		orig.t += pen.y;
		orig.r += pen.x;
		orig.b += pen.y;
		col.a *= cfg.elements_alpha;

		o.base::aabb(img, orig, col);
	};

	const auto max_score = typed_mode.calc_max_faction_score();

	auto print_faction = [&](const faction_type faction, const bool on_top) {
		(void)on_top;
		auto& colors = in.config.faction_view.colors[faction];

		const auto& cosm = mode_input.cosm;

		std::vector<std::pair<bomb_mode_player, mode_player_id>> sorted_players;

		typed_mode.for_each_player_in(faction, [&](
			const auto& id, 
			const auto& player
		) {
			sorted_players.emplace_back(player, id);
		});

		sort_range(sorted_players);

		const auto& bg_dark = colors.background_dark;

		const auto faction_state = typed_mode.factions[faction];

		auto draw_headline = [&]() {
			const auto bg_height = cell_h * 3;
			const auto faction_bg_orig = ltrbi(vec2i::zero, vec2i(sz.x, bg_height));

			aabb(faction_bg_orig, bg_dark);

			auto prev_pen_x = pen.x;

			pen.x += cell_pad.x * 2;

			if (sorted_players.size() > 0) {
				if (const auto& head_image = mode_input.vars.logos[faction]; head_image.is_set()) {
					if (const auto& entry = draw_in.images_in_atlas.at(head_image).diffuse; entry.exists()) {
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

						aabb_img(entry, head_orig, rgba(white).mult_alpha(cfg.faction_logo_alpha_mult));

						pen.x += size.x + cell_pad.x * 2;
					}
				}
			}

			const auto score_text_max_w = calc_size(fmt_large((max_score >= 10 ? "99" : "9"))).x;
			auto score_text = typesafe_sprintf("%x", faction_state.score);

			if (max_score >= 10 && score_text.size() == 1) {
				score_text = "0" + score_text;
			}

			//const auto score_text_size = calc_size(fmt_large(score_text));

			const auto score_text_pos = vec2i(cell_pad.x, 0);
			//+ score_text_max_w - score_text_size.x

			text_stroked_large(score_text, colors.standard, score_text_pos);

			//const auto score_text_r = score_text_pos.x + score_text_size.x;

			pen.x += score_text_max_w + cell_pad.x * 4;
			//head_orig.r + cell_pad.x * 2

			text_stroked(std::string("for ") + format_enum(faction), colors.standard, vec2i { 0, faction_bg_orig.get_center().y } , { augs::center::Y });

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

		for (const auto& p : sorted_players) {
			const auto& player_id = p.second;
			const auto& player_data = p.first;

			const auto player_handle = cosm[player_data.guid];
			const auto is_conscious = player_handle.alive() && player_handle.template get<components::sentience>().is_conscious();

			const auto faction_bg_col = [&]() {
				auto col = colors.standard;

				auto total_lumi = cfg.bg_lumi_mult;
				auto total_alpha = 1.f;

				const bool is_local = player_id == draw_in.local_player;

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

				if (player_id == draw_in.local_player) {
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

			const auto ping = 0;
			const auto ping_str = typesafe_sprintf("%x", ping);

			const auto& stats = player_data.stats;

			col_text(ping_str);
			next_col();
			next_col();
			col_text(player_data.chosen_name);
			next_col();

			{
				const auto local_player_faction = [&]() {
					if (const auto p = typed_mode.find(draw_in.local_player)) {
						return p->faction;
					}

					return faction_type::SPECTATOR;
				}();

				const bool hide_money = mode_input.vars.hide_money_of_opposing_factions && faction != local_player_faction;

				if (!hide_money) {
					col_text(typesafe_sprintf("%x$", stats.money));
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

	const auto participants = typed_mode.calc_participating_factions(mode_input);

	print_faction(participants.defusing, true);
	print_faction(participants.bombing, false);
}


template void arena_scoreboard_gui::draw_gui(
	const draw_setup_gui_input&,
	const draw_mode_gui_input&, 

	const bomb_mode& mode, 
	const typename bomb_mode::input&
) const;
