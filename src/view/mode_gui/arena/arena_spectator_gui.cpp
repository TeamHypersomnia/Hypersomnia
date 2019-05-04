#include <unordered_map>
#include "view/mode_gui/arena/arena_spectator_gui.h"

#include "augs/gui/text/printer.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/modes/bomb_mode.hpp"
#include "application/setups/draw_setup_gui_input.h"
#include "application/config_lua_table.h"
#include "game/modes/mode_helpers.h"
#include "view/viewables/images_in_atlas_map.h"

bool arena_spectator_gui::control(const app_ingame_intent_input in) {
	using namespace augs::event;
	using namespace augs::event::keys;

	if (!show) {
		return false;
	}

	const auto ch = in.e.get_key_change();

	if (ch == key_change::PRESSED) {
		const auto key = in.e.get_key();

		if (const auto it = mapped_or_nullptr(in.controls, key)) {
			if (*it == app_ingame_intent_type::SPECTATE_NEXT) {
				key_requested_offset = 1;
				return true;
			}

			if (*it == app_ingame_intent_type::SPECTATE_PREV) {
				key_requested_offset = -1;
				return true;
			}
		}
	}

	return false;
}

template <class M>
void arena_spectator_gui::draw_gui(
	const draw_setup_gui_input& in,
	const draw_mode_gui_input& draw_in,

	const M& typed_mode, 
	const typename M::const_input& mode_input
) const {
	using namespace augs::gui::text;

	(void)typed_mode;
	(void)mode_input;
	(void)draw_in;

	const bool should_show = show;

	if (!should_show) {
		return;
	}

	const auto spectated = typed_mode.find(now_spectating);

	if (spectated == nullptr) {
		return;
	}

	const auto& cfg = in.config.arena_mode_gui.scoreboard_settings;

	const auto cell_pad = cfg.player_row_inner_padding;

	auto fmt = [&](const auto& text, const rgba col = white) {
		return formatted_string(text, style(in.gui_fonts.gui, col));
	};

	const auto font_h = in.gui_fonts.gui.metrics.get_height();
	const auto cell_h = font_h + cell_pad.y * 2;

	const auto window_name = "Currently spectating:";

	//const auto estimated_window_height = window_name_size * 2;

	const auto s = in.screen_size;

	auto draw_text_indicator_at = [&](const auto& val, const auto t, const rgba stroke_col = black) {
		print_stroked(
			in.drawer,
			{ s.x / 2, static_cast<int>(t) },
			val,
			{ augs::ralign::CX },
			stroke_col
		);
	};

	const auto num_conscious = typed_mode.num_conscious_players_in(mode_input.cosm, spectated->faction);

	const auto one_sixth_t = in.screen_size.y / 5;

	draw_text_indicator_at(fmt(window_name, yellow), one_sixth_t);
	draw_text_indicator_at(fmt(spectated->chosen_name, white), one_sixth_t + cell_h);

	const auto& key_map = in.config.app_ingame_controls;

	const auto bound_left = key_or_default(key_map, app_ingame_intent_type::SPECTATE_PREV);
	const auto bound_right = key_or_default(key_map, app_ingame_intent_type::SPECTATE_NEXT);

	const bool spectated_is_conscious = typed_mode.on_player_handle(mode_input.cosm, now_spectating, [&](const auto& player_handle) {
		if constexpr(!is_nullopt_v<decltype(player_handle)>) {
			const auto& sentience = player_handle.template get<components::sentience>();

			return sentience.is_conscious();
		}

		return false;
	});

	if (num_conscious == 1 && spectated_is_conscious) {
		draw_text_indicator_at(fmt("Last man standing!", orange), one_sixth_t + 2 * cell_h, rgba(200, 0, 0, 255));
	}
	else if (num_conscious > 0) {
		const auto instructions = 
			fmt("<<< ", white)
			+ fmt(key_to_string(bound_left), white)
			+ fmt("   (change player)   ", gray4)
			+ fmt(key_to_string(bound_right), white)
			+ fmt(" >>>", white)
		;
			
		draw_text_indicator_at(instructions, one_sixth_t + 2 * cell_h, rgba(0, 0, 0, 150));
	}
}

template <class M>
void arena_spectator_gui::advance(
	const mode_player_id& local_player,
	const M& mode, 
	const typename M::const_input& in
) {
	auto hide = [&]() {
		show = false;
		now_spectating = {};
		cached_order = {};
	};

	if (mode.get_state() == arena_mode_state::MATCH_SUMMARY) {
		hide();
		return;
	}

	const auto& clk = in.cosm.get_clock();
	const auto max_secs = in.rules.view.can_spectate_dead_body_for_secs;

	auto cache_order_of = [&](const mode_player_id& id) {
		if (const auto data = mode.find(id)) {
			cached_order = data->get_order();
		}
		else {
			cached_order = {};
		}
	};

	if (mode.conscious_or_can_still_spectate(in, local_player)) {
		hide();
	}
	else {
		LOG("FIRST unc and cant");

		mode.on_player_handle(in.cosm, local_player, [&](const auto& player_handle) {
			if constexpr(!is_nullopt_v<decltype(player_handle)>) {
				const auto& sentience = player_handle.template get<components::sentience>();

				LOG_NVPS(sentience.is_conscious());
				LOG_NVPS(clk.lasts(max_secs * 1000,sentience.when_knocked_out));
				LOG_NVPS(sentience.when_knocked_out.step);
				LOG_NVPS(clk.now.step);
				LOG_NVPS(player_handle);
			}
		});

		if (!show) {
			cache_order_of(local_player);
			show = true;
		}
	}

	if (!show) {
		return;
	}

	auto switch_spectated_by = [&](const int off) {
		auto reference_order = cached_order;

		if (const auto data = mode.find(now_spectating)) {
			reference_order = data->get_order();
		}

		now_spectating = mode.get_next_to_spectate(in, reference_order, local_player, off);
		cache_order_of(now_spectating);
	};

	if (!mode.suitable_for_spectating(in, now_spectating, local_player)) {
		if (!now_spectating.is_set()) {
			LOG("Initializing spect from dead ids");
		}
		else {
			if (!mode.conscious_or_can_still_spectate(in, now_spectating)) {
				LOG("unc and cant");

				mode.on_player_handle(in.cosm, now_spectating, [&](const auto& player_handle) {
					if constexpr(!is_nullopt_v<decltype(player_handle)>) {
						const auto& sentience = player_handle.template get<components::sentience>();

						LOG_NVPS(sentience.is_conscious());
						LOG_NVPS(clk.lasts(max_secs * 1000,sentience.when_knocked_out));
						LOG_NVPS(sentience.when_knocked_out.step);
						LOG_NVPS(clk.now.step);
					}
				});
			}

			LOG("Unsuitable for spect");
		}

		switch_spectated_by(1);
	}

	auto& off = key_requested_offset;

	if (off) {
		switch_spectated_by(off);
		off = 0;
	}
}

template void arena_spectator_gui::draw_gui(
	const draw_setup_gui_input&,
	const draw_mode_gui_input&, 

	const bomb_mode& mode, 
	const typename bomb_mode::const_input&
) const;

template void arena_spectator_gui::advance(
	const mode_player_id& local_player,
	const bomb_mode& mode, 
	const typename bomb_mode::const_input& in
);
