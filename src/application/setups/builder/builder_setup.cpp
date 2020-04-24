#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_lua_table.h"
#include "application/setups/builder/builder_setup.h"

builder_setup::builder_setup() {

}

void builder_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Hypersomnia test scene";
}

void builder_setup::accept_game_gui_events(const game_gui_entropy_type& events) {
	control(events);
}

custom_imgui_result builder_setup::perform_custom_imgui(const perform_custom_imgui_input in) {
	(void)in;
	return custom_imgui_result::NONE;
}