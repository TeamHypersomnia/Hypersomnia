#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_lua_table.h"
#include "application/setups/project_selector/project_selector_setup.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/byte_file.h"

#include "application/setups/builder/builder_paths.h"

const entropy_accumulator entropy_accumulator::zero;

project_selector_setup::project_selector_setup() {
	augs::create_directories(BUILDER_DIR);
}

project_selector_setup::~project_selector_setup() {

}

void project_selector_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Arena builder - project selector";
}

custom_imgui_result project_selector_setup::perform_custom_imgui(const perform_custom_imgui_input in) {
	(void)in;

	return custom_imgui_result::NONE;
}
