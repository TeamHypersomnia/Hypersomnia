#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/editor_entity_selector.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/gui/editor_all_entities_gui.h"

void editor_command_input::interrupt_tweakers() const {
	all_entities_gui.interrupt_tweakers();
}

void editor_command_input::purge_selections() const {
	folder.view.selected_entities.clear();
	selector.clear();
}
