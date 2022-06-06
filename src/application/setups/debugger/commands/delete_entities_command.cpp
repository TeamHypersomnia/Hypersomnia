#include "game/cosmos/entity_handle.h"

#include "application/intercosm.h"
#include "application/setups/debugger/debugger_command_input.h"
#include "application/setups/debugger/debugger_folder.h"
#include "game/cosmos/create_entity.hpp"

#include "application/setups/debugger/commands/delete_entities_command.h"
#include "application/setups/debugger/commands/debugger_command_sanitizer.h"

std::string delete_entities_command::describe() const {
	return built_description;
}

void delete_entities_command::push_entry(const const_entity_handle handle) {
	handle.dispatch([&](const auto typed_handle) {
		using E = entity_type_of<decltype(typed_handle)>;
		deleted_entities.get_for<E>().push_back({ typed_handle.get(), handle.get_id(), {} });
	});

	deleted_grouping.push_entry(handle.get_id());
}

bool delete_entities_command::empty() const {
	return size() == 0;
}

void delete_entities_command::redo(const debugger_command_input in) {
	deleted_grouping.redo(in);

	in.interrupt_tweakers();

	auto& cosm = in.get_cosmos();

	deleted_entities.for_each([&](auto& e) {
		const auto handle = cosm[e.id];
		in.clear_dead_entity(handle.get_id());
		e.undo_delete_input = *cosmic::delete_entity(handle);
	});
}

void delete_entities_command::undo(const debugger_command_input in) {
	deleted_grouping.undo(in);

	auto& f = in.folder;
	auto& cosm = in.get_cosmos();

	in.purge_selections();

	{
		auto& selections = f.commanded->view_ids.selected_entities;

		/* 
			NOTE: Pools should be independent, but to be theoretically pure,
			we should implement reverse_for_each_through_std_get inside for_each_reverse.
		*/

		deleted_entities.for_each_reverse([&](const auto& e) {
			const auto undeleted = cosmic::undo_delete_entity(cosm, e.undo_delete_input, e.content, reinference_type::NONE);
			selections.emplace(undeleted.get_id());
		});
	}

	cosmic::reinfer_all_entities(cosm);
}

void delete_entities_command::sanitize(const debugger_command_input in) {
	deleted_grouping.sanitize(in);

	sanitize_affected_entities(in, deleted_entities, [](const auto& entry) {
		return entry.id;
	});
}
