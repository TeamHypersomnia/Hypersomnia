#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_lua_table.h"
#include "application/setups/builder/builder_setup.h"
#include "application/setups/builder/builder_paths.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/byte_file.h"
#include "augs/log.h"

builder_setup::builder_setup() {
	augs::create_directories(BUILDER_PROJECTS_DIR);
	LOG("Loading the last opened builder project.");

	load_gui_state();
}

builder_setup::builder_setup(const augs::path_type& project_path) {
	augs::create_directories(BUILDER_PROJECTS_DIR);
	LOG("Loading builder project at: %x", project_path);

	load_gui_state();
}

builder_setup::~builder_setup() {
	save_gui_state();
}

void builder_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Arena builder";
}

void builder_setup::load_gui_state() {
	// TODO: Read/write as yaml

	try {
		augs::load_from_bytes(gui, get_builder_gui_state_path());
	}
	catch (const augs::file_open_error&) {
		// We don't care if it does not exist
	}
}

void builder_setup::save_gui_state() {
	augs::save_as_bytes(gui, get_builder_gui_state_path());
}
