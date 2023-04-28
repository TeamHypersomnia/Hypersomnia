#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"
#include "view/client_arena_type.h"

struct client_demo_player;

struct demo_player_gui : standard_window_mixin<demo_player_gui> {
	using base = standard_window_mixin<demo_player_gui>;
	using base::base;
	using introspect_base = base;

	bool show_spectator_overlay = true;
	bool pending_interpolation_snap = false;
	bool pending_dump = false;

	client_arena_type shown_arena_type = client_arena_type::REFERENTIAL;

	void perform(augs::window& window, client_demo_player& player);

	bool requires_cursor() const {
		return show;
	}
};
