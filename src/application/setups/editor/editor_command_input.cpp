#include "application/intercosm.h"

#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/editor_folder.h"

#include "application/setups/editor/gui/editor_entity_selector.h"
#include "application/setups/editor/gui/editor_entity_mover.h"

#include "application/setups/editor/gui/editor_fae_gui.h"
#include "game/cosmos/entity_handle.h"
#include "application/setups/editor/editor_player.h"
#include "application/setups/editor/gui/editor_fae_gui.h"

editor_history& editor_command_input::get_history() const {
	return folder.history;
}

editor_player& editor_command_input::get_player() const {
	return folder.player;
}

augs::snapshotted_player_step_type editor_command_input::get_current_step() const {
	return get_player().get_current_step();
}

all_viewables_defs& editor_command_input::get_viewable_defs() const {
	return folder.commanded->work.viewables;
}

const all_logical_assets& editor_command_input::get_logical_assets() const {
	return get_cosmos().get_logical_assets();
}

cosmos& editor_command_input::get_cosmos() const {
	return folder.commanded->work.world;
}

void editor_command_input::interrupt_tweakers() const {
	fae_gui.interrupt_tweakers();
	selected_fae_gui.interrupt_tweakers();
}

void editor_command_input::purge_selections() const {
	folder.commanded->view_ids.selected_entities.clear();
	selector.clear();
	mover.escape();
}

void editor_command_input::clear_dead_entity(const entity_id id) const {
	erase_element(folder.commanded->view_ids.selected_entities, id);

	selector.clear_dead_entity(id);
}

void editor_command_input::clear_dead_entities() const {
	const auto& cosm = get_cosmos();
	selector.clear_dead_entities(cosm);

	auto& view_ids = folder.commanded->view_ids;
	
	cosm.erase_dead(view_ids.selected_entities);

	view_ids.selection_groups.clear_dead_entities(cosm);

	fae_gui.clear_dead_entities(cosm);
	selected_fae_gui.clear_dead_entities(cosm);
}

bool editor_command_input::allow_new_commands() const {
	return !folder.player.is_replaying();
}


editor_command_input editor_command_input::make_dummy_for(sol::state& lua, editor_folder& folder) {
	/* Create dummies */
	struct dummies {
		editor_entity_mover mover;
		editor_entity_selector selector;
		editor_settings settings;
		editor_fae_gui fae_gui = std::string();
		editor_selected_fae_gui selected_fae_gui = std::string();
	};

	thread_local dummies d;

	LOG_NVPS(sizeof(dummies));

	d.settings.player.snapshot_interval_in_steps = 0;

	return editor_command_input {
		lua,
		d.settings,
		folder,
		d.selector,
		d.fae_gui,
		d.selected_fae_gui,
		d.mover
	};
}
