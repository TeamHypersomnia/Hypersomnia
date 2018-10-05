#include "game/cosmos/entity_handle.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/editor_folder.h"
#include "game/cosmos/create_entity.hpp"

#include "application/setups/editor/commands/delete_entities_command.h"

std::string delete_entities_command::describe() const {
	return built_description;
}

void delete_entities_command::push_entry(const const_entity_handle handle) {
	handle.dispatch([&](const auto typed_handle) {
		using E = entity_type_of<decltype(typed_handle)>;
		deleted_entities.get_for<E>().push_back({ typed_handle.get(), {} });
	});

	deleted_grouping.push_entry(handle.get_id());
}

bool delete_entities_command::empty() const {
	return size() == 0;
}

void delete_entities_command::redo(const editor_command_input in) {
	deleted_grouping.redo(in);

	in.interrupt_tweakers();

	auto& cosm = in.get_cosmos();

	deleted_entities.for_each([&](auto& e) {
		const auto handle = cosm[e.content.guid];
		in.clear_selection_of(handle.get_id());
		e.undo_delete_input = *cosmic::delete_entity(handle);
	});
}

void delete_entities_command::undo(const editor_command_input in) {
	deleted_grouping.undo(in);

	auto& f = in.folder;
	auto& cosm = in.get_cosmos();

	in.purge_selections();

	{
		auto& selections = f.view.ids.selected_entities;

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
