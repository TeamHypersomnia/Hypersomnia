#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_lua_table.h"
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/editor_paths.h"

#include "augs/filesystem/directory.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/byte_file.h"
#include "augs/log.h"

editor_setup::editor_setup(const augs::path_type& project_path) : paths(project_path) {
	augs::create_directories(EDITOR_PROJECTS_DIR);
	LOG("Loading editor project at: %x", project_path);

	load_gui_state();
	open_default_windows();

	on_window_activate();
}

editor_setup::~editor_setup() {
	save_gui_state();
}

void editor_setup::open_default_windows() {
	gui.inspector.open();
	gui.layers.open();
	gui.filesystem.open();
}

bool editor_setup::handle_input_before_imgui(
	handle_input_before_imgui_input in
) {
	using namespace augs::event;

	if (in.e.msg == message::activate) {
		on_window_activate();
	}

	if (in.e.msg == message::deactivate) {
		force_autosave_now();
	}

	return false;
}

bool editor_setup::handle_input_before_game(
	handle_input_before_game_input
) {
	return false;
}

void editor_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = typesafe_sprintf("Hypersomnia Editor - %x", project.meta.name);
}

void editor_setup::on_window_activate() {
	rebuild_filesystem();
}

void editor_setup::rebuild_filesystem() {
	files.rebuild_from(paths.folder_path);
}

void editor_setup::force_autosave_now() {

}

void editor_setup::load_gui_state() {
	/*
		To be decided what to do about it.
		Generally ImGui will save the important layouts on its own.
		The identifiers (e.g. currently inspected object id) might become out of date after reloading from json.
	*/
}

void editor_setup::save_gui_state() {

}
