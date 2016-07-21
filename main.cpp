#pragma once
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "game/multiverse.h"
#include "game/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_managers/testbed.h"
#include "game/scene_managers/resource_setups/all.h"

#include "game/types_specification/all_component_includes.h"
#include "game/entity_relations.h"

int main(int argc, char** argv) {
	augs::global_libraries::init();
	augs::global_libraries::run_googletest();

	game_window window;

	window.call_window_script("config.lua");

	resource_manager.destroy_everything();
	resource_setups::load_standard_everything();

	multiverse hypersomnia;
	hypersomnia.main_cosmos.settings.screen_size = vec2i(window.window.get_screen_rect());
	hypersomnia.populate_cosmoi();

	window.window.set_as_current();

	bool should_quit = false;

	if (!hypersomnia.main_cosmos_player.try_to_load_and_replay_recording("recorded.inputs"))
		hypersomnia.main_cosmos_player.record_and_save_this_session("sessions/", "recorded.inputs");

	while (!should_quit) {
		auto new_entropy = window.collect_entropy();

		for (auto& n : new_entropy.local) {
			if (n.key == augs::window::event::keys::ESC && n.key_event == augs::window::event::key_changed::PRESSED) {
				should_quit = true;
			}
		}

		hypersomnia.control(new_entropy);
		hypersomnia.simulate();
		hypersomnia.view(window);
	}

	augs::global_libraries::deinit();
	return 0;
}