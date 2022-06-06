#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_lua_table.h"
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/editor_paths.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/byte_file.h"
#include "augs/log.h"

editor_setup::editor_setup() {
	augs::create_directories(editor_PROJECTS_DIR);
	LOG("Loading the last opened editor project.");

	load_gui_state();
}

editor_setup::editor_setup(const augs::path_type& project_path) {
	augs::create_directories(editor_PROJECTS_DIR);
	LOG("Loading editor project at: %x", project_path);

	load_gui_state();
}

editor_setup::~editor_setup() {
	save_gui_state();
}

void editor_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Arena editor";
}

void editor_setup::load_gui_state() {
	// TODO: Read/write as yaml

	try {
		augs::load_from_bytes(gui, get_editor_gui_state_path());
	}
	catch (const augs::file_open_error&) {
		// We don't care if it does not exist
	}
}

void editor_setup::save_gui_state() {
	augs::save_as_bytes(gui, get_editor_gui_state_path());
}
