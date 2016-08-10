#pragma once
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "game/transcendental/multiverse.h"
#include "game/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_managers/testbed.h"
#include "game/scene_managers/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/entity_relations.h"
#include "game/transcendental/viewing_session.h"

#include "augs/filesystem/file.h"

int main(int argc, char** argv) {
	augs::global_libraries::init();
	augs::global_libraries::run_googletest();

	game_window window;

	window.call_window_script("config.lua");
	vec2i screen_size = vec2i(window.window.get_screen_rect());

	resource_manager.destroy_everything();
	resource_setups::load_standard_everything();

	multiverse hypersomnia;

	if (!hypersomnia.try_to_load_save()) {
		hypersomnia.main_cosmos.significant.meta.settings.screen_size = screen_size;
		hypersomnia.populate_cosmoi();
	}
	else 
		hypersomnia.main_cosmos.significant.meta.settings.screen_size = screen_size;
	
	hypersomnia.try_to_load_or_save_new_session();

	window.window.set_as_current();

	viewing_session session;
	session.camera.configure_size(screen_size);

	bool should_quit = false;

	while (!should_quit) {
		auto new_entropy = window.collect_entropy();

		for (auto& n : new_entropy.local) {
			if (n.key == augs::window::event::keys::ESC && n.key_event == augs::window::event::key_changed::PRESSED) {
				should_quit = true;
			}
		}

		hypersomnia.control(new_entropy);
		hypersomnia.simulate();
		hypersomnia.view(window, session);
	}

	augs::global_libraries::deinit();
	return 0;
}