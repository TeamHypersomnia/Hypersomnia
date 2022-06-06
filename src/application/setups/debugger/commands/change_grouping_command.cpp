#include "application/setups/debugger/commands/change_grouping_command.h"
#include "application/setups/debugger/debugger_folder.h"
#include "application/setups/debugger/debugger_selection_groups.hpp"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

std::size_t change_grouping_command::size() const {
	return affected_entities.size();
}

bool change_grouping_command::empty() const {
	return affected_entities.empty();
}

void change_grouping_command::push_entry(const entity_id id) {
	affected_entities.push_back(id);
}

void change_grouping_command::sanitize(debugger_command_input in) {
	std::vector<bool> to_erase;
	to_erase.resize(affected_entities.size());

	const auto& cosm = in.get_cosmos();

	for (std::size_t i = 0; i < affected_entities.size(); ++i) {
		if (cosm[affected_entities[i]].dead()) {
			to_erase[i] = true;
		}
	}

	auto cleaner = [&](auto& where) {
		erase_if(where, [&](const auto& what) {
			return to_erase[index_in(where, what)];
		});
	};

	cleaner(affected_entities);
	cleaner(group_indices_after);
}

void change_grouping_command::clear_undo_state() {
	group_indices_before.clear();
}

void change_grouping_command::redo(const debugger_command_input in) {
	clear_undo_state();

	/* First, ungroup the affected entities */
	auto& groups = in.folder.commanded->view_ids.selection_groups;

	const auto pusher = [this](const auto index, auto& group, const auto it) {
		group_indices_before.push_back(static_cast<unsigned>(index));
		group.entries.erase(it);
	};

	for (const auto& e : affected_entities) {
		if (!groups.on_group_entry_of(e, pusher)) {
			group_indices_before.push_back(-1);	
		}
	}

	ensure_eq(group_indices_before.size(), affected_entities.size());

	if (all_to_new_group) {
		assign_begin_end(groups.new_group().entries, affected_entities);
	}
	else if (const auto num_indices = group_indices_after.size(); num_indices > 0) {
		ensure_eq(group_indices_before.size(), num_indices);

		for (std::size_t i = 0; i < num_indices; ++i) {
			const auto idx = group_indices_after[i];
			groups.set_group(affected_entities[i], idx); 
		}
	}
}

void change_grouping_command::undo(const debugger_command_input in) {
	ensure_eq(group_indices_before.size(), affected_entities.size());

	auto& groups = in.folder.commanded->view_ids.selection_groups;

	const auto eraser = [&](const auto, auto& group, const auto it) {
		group.entries.erase(it);
	};

	for (const auto& e : affected_entities) {
		const auto found_group = groups.on_group_entry_of(e, eraser);
		(void)found_group;

		if (all_to_new_group || group_indices_after.size() > 0) {
			/* 
				We're undoing creation of new group or assigning to some, 
				so the group MUST have been found on undoing. 
			*/

			ensure(found_group);
		}
		else {
			/* 
				We're undoing ungrouping some collection of entities,
				so NO GROUP must have been found on undoing.
			*/

			ensure(!found_group);
		}
	}

	for (std::size_t i = 0; i < affected_entities.size(); ++i) {
		if (const auto previous_group_index = group_indices_before[i];
			previous_group_index != static_cast<unsigned>(-1)
		) {
			const auto id = affected_entities[i];
			groups.set_group(id, previous_group_index); 
		}
	}

	clear_undo_state();
}

std::string change_grouping_command::describe() const {
	return built_description;
}

