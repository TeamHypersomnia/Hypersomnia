#pragma once
#include "application/setups/server/server_vars.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/gui/arena_chooser.h"
#include "application/arena/arena_paths.h"
#include "application/gui/config_nvp.h"

inline void do_server_vars(
	server_solvable_vars& vars,
	server_solvable_vars& last_saved_vars
) {
	using namespace augs::imgui;

	int field_id = 499999;

	auto& scope_cfg = vars;

	auto revert = make_revert_button_lambda(vars, last_saved_vars);

	thread_local arena_chooser chooser;

	chooser.perform(
		SCOPE_CFG_NVP(current_arena),
		augs::path_type(OFFICIAL_ARENAS_DIR),
		augs::path_type(COMMUNITY_ARENAS_DIR),
		[&](const auto& new_choice) {
			vars.current_arena = new_choice.path.string();
		}
	);

	revert(scope_cfg.current_arena);

	input_text(SCOPE_CFG_NVP(override_default_ruleset)); revert(scope_cfg.override_default_ruleset);
}

inline void do_server_vars(
	server_vars& vars,
	server_vars& last_saved_vars
) {
	using namespace augs::imgui;

	int field_id = 999999;

	auto& scope_cfg = vars;

	auto revert = make_revert_button_lambda(vars, last_saved_vars);

	auto revertable_slider = [&](auto l, auto& f, auto&&... args) {
		slider(l, f, std::forward<decltype(args)>(args)...);
		revert(f);
	};

	text_color("General", yellow);

	ImGui::Separator();

	if (auto node = scoped_tree_node("Time limits")) {
		revertable_slider(SCOPE_CFG_NVP(kick_if_no_messages_for_secs), 2u, 300u);
		revertable_slider(SCOPE_CFG_NVP(kick_if_away_from_keyboard_for_secs), 20u, 6000u);
		revertable_slider(SCOPE_CFG_NVP(time_limit_to_enter_game_since_connection), 5u, 300u);
	}

	ImGui::Separator();

	text_color("Dedicated server", yellow);

	ImGui::Separator();

	revertable_slider(SCOPE_CFG_NVP(sleep_mult), 0.0f, 0.9f);
}
