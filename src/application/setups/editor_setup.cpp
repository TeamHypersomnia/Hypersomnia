#include "augs/templates/string_templates.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/log_color.h"

#include "application/setups/editor_setup.h"

#include "generated/introspectors.h"

editor_setup::editor_setup(const editor_settings settings) {
	edited_world.set_steps_per_second(60);
}

void editor_setup::control(
	const cosmic_entropy& entropy
) {

}

void editor_setup::accept_game_gui_events(
	const cosmic_entropy& entropy
) {

}