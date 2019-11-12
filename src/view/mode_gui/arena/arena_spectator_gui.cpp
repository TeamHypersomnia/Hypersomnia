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
#include "game/detail/sentience/sentience_getters.h"

const auto secs_before_accepting_inputs_after_death = 1.0;
const auto secs_until_switching_dead_teammate = 1.0;

bool arena_spectator_gui::control(const general_gui_intent_input in) {
	using namespace augs::event;
	using namespace augs::event::keys;

	if (!accept_inputs) {
		return false;
	}

	const auto ch = in.e.get_key_change();

	if (ch == key_change::PRESSED) {
		const auto key = in.e.get_key();

		if (const auto it = mapped_or_nullptr(in.controls, key)) {
			if (*it == general_gui_intent_type::SPECTATE_NEXT) {
				key_requested_offset = 1;
				return true;
			}

			if (*it == general_gui_intent_type::SPECTATE_PREV) {
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
	(void)draw_in;

	if (!should_be_drawn(typed_mode)) {
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

	const auto top_caption = "Currently spectating:";

	const auto s = in.screen_size;

	auto draw_text_indicator_at = [&](const auto& val, const auto t, const rgba stroke_col = black) {
		print_stroked(
			in.get_drawer(),
			{ s.x / 2, static_cast<int>(t) },
			val,
			{ augs::ralign::CX },
			stroke_col
		);
	};

	const auto num_conscious = typed_mode.num_conscious_players_in(mode_input.cosm, spectated->faction);

	const auto one_sixth_t = in.screen_size.y / 5;

	draw_text_indicator_at(fmt(top_caption, yellow), one_sixth_t);
	draw_text_indicator_at(fmt(spectated->chosen_name, white), one_sixth_t + cell_h);

	const auto& key_map = in.config.general_gui_controls;

	const auto bound_left = key_or_default(key_map, general_gui_intent_type::SPECTATE_PREV);
	const auto bound_right = key_or_default(key_map, general_gui_intent_type::SPECTATE_NEXT);

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
			+ fmt(key_to_string_shortened(bound_left), white)
			+ fmt("   (change player)   ", gray4)
			+ fmt(key_to_string_shortened(bound_right), white)
			+ fmt(" >>>", white)
		;
			
		draw_text_indicator_at(instructions, one_sixth_t + 2 * cell_h, rgba(0, 0, 0, 150));
	}
}

void arena_spectator_gui::hide() {
	accept_inputs = false;
	active = false;

	now_spectating = {};
	cached_order = {};
	when_local_player_knocked_out = std::nullopt;
}

template <class M>
void arena_spectator_gui::advance(
	const mode_player_id& local_player,
	const M& mode, 
	const typename M::const_input& in
) {
	const auto& cosm = in.cosm;

	auto local_faction = faction_type::DEFAULT;

	auto hide_if_local_is_conscious = [&]() {
		const auto player_data = mode.find(local_player);

		if (player_data == nullptr) {
			hide();
			return true;
		}

		local_faction = player_data->faction;

		if (const auto controlled = cosm[player_data->controlled_character_id]) {
			if (::sentient_and_conscious(controlled)) {
				hide();
				return true;
			}
		}

		return false;
	};

	(void)local_faction;

	auto hide_if_match_summary = [&]() {
		if (mode.get_state() == arena_mode_state::MATCH_SUMMARY) {
			hide();
			return true;
		}

		return false;
	};

	auto show_or_unshow_if_spectating_local = [&]() {
		active = now_spectating != local_player;
	};

	auto cache_order_of = [&](const mode_player_id& id) {
		if (const auto data = mode.find(id)) {
			cached_order = data->get_order();
		}
		else {
			cached_order = {};
		}
	};

	auto init_order_and_timer_if_showing_for_the_first_time = [&]() {
		auto& when = when_local_player_knocked_out;

		auto init_spectator = [&]() {
			when = augs::timer();
			now_spectating = local_player;
			cache_order_of(local_player);
		};

		if (when == std::nullopt) {
			init_spectator();
		}
	};

	auto allow_inputs_if_knocked_out_for_long_enough = [&]() {
		auto& when = when_local_player_knocked_out;

		if (when.value().get<std::chrono::seconds>() > secs_before_accepting_inputs_after_death) {
			accept_inputs = true;
		}
	};

	auto switch_spectated_by = [&](const int off) {
		auto reference_order = cached_order;

		if (const auto data = mode.find(now_spectating)) {
			reference_order = data->get_order();
		}

		now_spectating = mode.get_next_to_spectate(
			in, 
			reference_order, 
			local_player, 
			off, 
			secs_until_switching_dead_teammate
		);

		cache_order_of(now_spectating);
	};

	auto switch_if_current_unsuitable_already = [&]() {
		const bool spectating_local = now_spectating == local_player;
		const auto limit_secs = spectating_local ? 10000.f : secs_until_switching_dead_teammate;

		if (!mode.suitable_for_spectating(in, now_spectating, local_player, limit_secs)) {
			switch_spectated_by(1);
		}
	};

	auto switch_if_requested_by_key = [&]() {
		auto& off = key_requested_offset;

		if (off) {
			switch_spectated_by(off);
			off = 0;
		}
	};

	accept_inputs = false;

	if (hide_if_local_is_conscious()) {
		return;
	}

	if (hide_if_match_summary()) {
		return;
	}

	init_order_and_timer_if_showing_for_the_first_time();
	allow_inputs_if_knocked_out_for_long_enough();

	show_or_unshow_if_spectating_local();
	switch_if_current_unsuitable_already();
	switch_if_requested_by_key();
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
