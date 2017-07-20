#pragma once
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#include "game/view/viewing_session.h"

#include "augs/unit_tests.h"

int main(int argc, char** argv) {
	const std::string canon_config_path = "config.lua";
	const std::string local_config_path = "config.local.lua";

	augs::create_directories("generated/logs/");

	const auto cfg = config_lua_table(
		augs::switch_path(
			canon_config_path,
			local_config_path
		)
	);
	
	augs::run_unit_tests(argc, argv, cfg.unit_tests);
	augs::generate_alsoft_ini(cfg.audio.max_number_of_sound_sources);

	std::make_unique<const viewing_session>(cfg, local_config_path);
	
	global_log::save_complete_log("generated/logs/successful_exit_debug_log.txt");

	return 0;
}