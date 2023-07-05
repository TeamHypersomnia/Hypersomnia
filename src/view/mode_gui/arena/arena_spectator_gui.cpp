#include <unordered_map>
#include "view/mode_gui/arena/arena_spectator_gui.h"

#include "augs/gui/text/printer.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/modes/arena_mode.hpp"
#include "application/setups/draw_setup_gui_input.h"
#include "application/config_lua_table.h"
#include "game/modes/mode_helpers.h"
#include "view/viewables/images_in_atlas_map.h"
#include "game/detail/sentience/sentience_getters.h"
#include "augs/templates/logically_empty.h"
#include "augs/drawing/drawing.hpp"

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

			if (*it == general_gui_intent_type::SPECTATE_PREVIOUS) {
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

	const auto spectated = typed_mode.find(now_spectating);

	if (spectated == nullptr) {
		return;
	}

	if (!should_be_drawn(typed_mode)) {
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

	const auto num_conscious = typed_mode.num_conscious_players_in(mode_input.cosm, spectated->get_faction());

	const auto one_sixth_t = in.screen_size.y / 4.5;

	auto& avatar_triangles = in.renderer.dedicated[augs::dedicated_buffer::SPECTATOR_AVATAR].triangles;

	const auto avatar_atlas_entry = in.avatars_in_atlas.at(now_spectating.value); 
	const bool avatars_enabled = logically_set(in.general_atlas, in.avatar_atlas);
	const bool avatar_displayed = avatar_atlas_entry.exists() && avatars_enabled;

	const auto displayed_avatar_size = vec2i::square(avatar_displayed ? max_avatar_side_v / 2 : 0);
	const auto larger_font_h = in.gui_fonts.larger_gui.metrics.get_height();
	const auto larger_cell_h = std::max(static_cast<unsigned>(displayed_avatar_size.y), larger_font_h) + cell_pad.y * 2;

	const bool spectated_is_conscious = typed_mode.on_player_handle(mode_input.cosm, now_spectating, [&](const auto& player_handle) {
		if constexpr(!is_nullopt_v<decltype(player_handle)>) {
			const auto& sentience = player_handle.template get<components::sentience>();

			return sentience.is_conscious();
		}

		return false;
	});

	const bool last_man_standing = num_conscious == 1 && spectated_is_conscious;
	const auto name_text = formatted_string(spectated->get_nickname(), style(in.gui_fonts.larger_gui, white));

	const auto& key_map = in.config.general_gui_controls;

	const auto bound_left = key_or_default(key_map, general_gui_intent_type::SPECTATE_PREVIOUS);
	const auto bound_right = key_or_default(key_map, general_gui_intent_type::SPECTATE_NEXT);

	const auto instructions_text = 
		fmt("<<< ", white)
		+ fmt(key_to_string_shortened(bound_left), white)
		+ fmt("   (change player)   ", gray4)
		+ fmt(key_to_string_shortened(bound_right), white)
		+ fmt(" >>>", white)
	;

	const auto window_padding = vec2i(32, 16);
	const auto avatar_padding = window_padding / 4;

	const auto predicted_size = [&]() {
		const auto other_size = last_man_standing ? get_text_bbox(fmt("Last man standing!", orange)) : get_text_bbox(instructions_text);
		const auto nick_text_size = get_text_bbox(name_text);
		const auto nick_size = vec2i(displayed_avatar_size.x + nick_text_size.x, larger_cell_h);

		return window_padding + vec2i(avatar_padding.x * 2 + std::max(nick_size.x, other_size.x), nick_size.y + cell_h * 2);
	}();

	const auto window_bg_pos = vec2(s.x / 2 - predicted_size.x / 2, one_sixth_t - window_padding.y / 2);
	const auto window_bg_rect = ltrb(window_bg_pos, predicted_size);

	auto general_drawer = in.get_drawer();

	// TODO give it its own settings struct
	{
		const auto& cfg = in.config.arena_mode_gui.scoreboard_settings;

		general_drawer.aabb_with_border(window_bg_rect, cfg.background_color, cfg.border_color);
	}

	draw_text_indicator_at(fmt(top_caption, yellow), one_sixth_t);

	vec2i name_text_size;

	{
		const auto t = one_sixth_t + cell_h;

		name_text_size = print_stroked(
			in.get_drawer(),
			{ s.x / 2 + displayed_avatar_size.x / 2 + avatar_padding.x, static_cast<int>(t) },
			name_text,
			{ augs::ralign::CX }
		);
	}

	if (avatar_displayed) {
		const auto pos = vec2(s.x / 2 - name_text_size.x / 2 - displayed_avatar_size.x / 2 - avatar_padding.x, one_sixth_t + cell_h);
		const auto avatar_orig = ltrbi(pos, displayed_avatar_size);

		auto avatar_output = augs::drawer { avatar_triangles };
		avatar_output.aabb(avatar_atlas_entry, avatar_orig, white);
	}

	if (last_man_standing) {
		draw_text_indicator_at(fmt("Last man standing!", orange), one_sixth_t + cell_h + larger_cell_h, rgba(200, 0, 0, 255));
	}
	else if (num_conscious > 0) {
		draw_text_indicator_at(instructions_text, one_sixth_t + cell_h + larger_cell_h, rgba(0, 0, 0, 150));
	}

	if (avatar_triangles.size() > 0) {
		in.renderer.call_and_clear_triangles();

		in.avatar_atlas->set_as_current(in.renderer);
		in.renderer.call_triangles(std::move(avatar_triangles));

		augs::graphics::texture::set_current_to_previous(in.renderer);
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
	const typename M::const_input& in,
	const bool demo_replay_mode
) {
	const auto& cosm = in.cosm;

	auto local_faction = faction_type::DEFAULT;

	auto hide_if_local_is_conscious = [&]() {
		const auto player_data = mode.find(local_player);

		if (player_data == nullptr) {
			hide();
			return true;
		}

		local_faction = player_data->get_faction();

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

		if (demo_replay_mode || when.value().get<std::chrono::seconds>() > secs_before_accepting_inputs_after_death) {
			accept_inputs = true;
		}
	};

	auto switch_spectated_by = [&](const int off) {
		auto reference_order = cached_order;

		if (const auto data = mode.find(now_spectating)) {
			reference_order = data->get_order();
		}

		if (const auto data = mode.find(local_player)) {
			const auto faction = demo_replay_mode ? faction_type::SPECTATOR : data->get_faction();

			now_spectating = mode.get_next_to_spectate(
				in, 
				reference_order, 
				faction, 
				off, 
				secs_until_switching_dead_teammate
			);
		}
		else {
			now_spectating = {};
		}

		cache_order_of(now_spectating);
	};

	auto switch_if_current_unsuitable_already = [&]() {
		const bool spectating_local = now_spectating == local_player;
		const auto secs_until_switching_myself = demo_replay_mode ? secs_until_switching_dead_teammate : 10000.f;
		const auto limit_secs = spectating_local ? secs_until_switching_myself : secs_until_switching_dead_teammate;

		if (demo_replay_mode) {
			/* Skip team constraints */
			if (!mode.conscious_or_can_still_spectate(in, now_spectating, limit_secs)) {
				switch_spectated_by(1);
			}
		}
		else {
			if (!mode.suitable_for_spectating(in, now_spectating, local_player, limit_secs)) {
				switch_spectated_by(1);
			}
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

	if (!demo_replay_mode) {
		if (hide_if_local_is_conscious()) {
			return;
		}
	}

	if (hide_if_match_summary()) {
		return;
	}

	init_order_and_timer_if_showing_for_the_first_time();
	allow_inputs_if_knocked_out_for_long_enough();

	if (demo_replay_mode) {
		active = true;
	}
	else {
		show_or_unshow_if_spectating_local();
	}

	switch_if_current_unsuitable_already();
	switch_if_requested_by_key();
}

template void arena_spectator_gui::draw_gui(
	const draw_setup_gui_input&,
	const draw_mode_gui_input&, 

	const arena_mode& mode, 
	const typename arena_mode::const_input&
) const;

template void arena_spectator_gui::advance(
	const mode_player_id& local_player,
	const arena_mode& mode, 
	const typename arena_mode::const_input& in,
	bool demo_replay_mode
);
