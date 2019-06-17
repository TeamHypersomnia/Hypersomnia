#pragma once
#include "game/modes/mode_player_id.h"
#include "game/modes/arena_player_order.h"
#include "game/modes/bomb_mode.h"
#include "application/app_intent_type.h"
#include "augs/misc/timing/timer.h"

struct app_ingame_intent_input;
struct draw_setup_gui_input;
struct draw_mode_gui_input;

struct arena_spectator_gui {
	mode_player_id now_spectating;
	arena_player_order_info cached_order;

	int key_requested_offset = 0;

	bool show = false;
	bool accept_inputs = false;

	std::optional<augs::timer> when_local_player_knocked_out;

	bool control(app_ingame_intent_input);
	void hide();

	template <class M>
	void advance(
		const mode_player_id& local_player,
		const M& mode, 
		const typename M::const_input& in
	);

	template <class M>
	void draw_gui(
		const draw_setup_gui_input&,
		const draw_mode_gui_input&,

		const M& mode, 
		const typename M::const_input&
	) const;
};
