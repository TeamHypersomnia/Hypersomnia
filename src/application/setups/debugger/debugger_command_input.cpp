#include "application/intercosm.h"

#include "application/setups/debugger/debugger_command_input.h"
#include "application/setups/debugger/debugger_folder.h"

#include "application/setups/debugger/gui/debugger_entity_selector.h"
#include "application/setups/debugger/gui/debugger_entity_mover.h"

#include "application/setups/debugger/gui/debugger_fae_gui.h"
#include "game/cosmos/entity_handle.h"
#include "application/setups/debugger/debugger_player.h"
#include "application/setups/debugger/gui/debugger_fae_gui.h"

debugger_history& debugger_command_input::get_history() const {
	return folder.history;
}

debugger_player& debugger_command_input::get_player() const {
	return folder.player;
}

augs::snapshotted_player_step_type debugger_command_input::get_current_step() const {
	return get_player().get_current_step();
}

all_viewables_defs& debugger_command_input::get_viewable_defs() const {
	return folder.commanded->work.viewables;
}

const all_logical_assets& debugger_command_input::get_logical_assets() const {
	return get_cosmos().get_logical_assets();
}

cosmos& debugger_command_input::get_cosmos() const {
	return folder.commanded->work.world;
}

void debugger_command_input::interrupt_tweakers() const {
	fae_gui.interrupt_tweakers();
	selected_fae_gui.interrupt_tweakers();
}

void debugger_command_input::purge_selections() const {
	folder.commanded->view_ids.selected_entities.clear();
	selector.clear();
	mover.escape();
}

void debugger_command_input::clear_dead_entity(const entity_id id) const {
	erase_element(folder.commanded->view_ids.selected_entities, id);

	selector.clear_dead_entity(id);
}

void debugger_command_input::clear_dead_entities() const {
	const auto& cosm = get_cosmos();
	selector.clear_dead_entities(cosm);

	auto& view_ids = folder.commanded->view_ids;
	
	cosm.erase_dead(view_ids.selected_entities);

	view_ids.selection_groups.clear_dead_entities(cosm);

	fae_gui.clear_dead_entities(cosm);
	selected_fae_gui.clear_dead_entities(cosm);
}

bool debugger_command_input::allow_new_commands() const {
	return !folder.player.is_replaying();
}


debugger_command_input debugger_command_input::make_dummy_for(sol::state& lua, debugger_folder& folder) {
	/* Create dummies */
	struct dummies {
		debugger_entity_mover mover;
		debugger_entity_selector selector;
		debugger_settings settings;
		debugger_fae_gui fae_gui = std::string();
		debugger_selected_fae_gui selected_fae_gui = std::string();
	};

	thread_local dummies d;

	d.settings.player.snapshot_interval_in_steps = 0;

	return debugger_command_input {
		lua,
		d.settings,
		folder,
		d.selector,
		d.fae_gui,
		d.selected_fae_gui,
		d.mover
	};
}
