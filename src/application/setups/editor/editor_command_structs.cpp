#include "application/setups/editor/editor_command_structs.h"
#include "application/setups/editor/editor_entity_selector.h"
#include "application/setups/editor/editor_folder.h"

void editor_command_input::purge_selections() const {
	folder.view.selected_entities.clear();
	selector.clear();
}
