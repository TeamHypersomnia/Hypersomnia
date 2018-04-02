#include "game/transcendental/entity_handle.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/editor_folder.h"

#include "application/setups/editor/commands/duplicate_entities_command.h"

std::string duplicate_entities_command::describe() const {
	return built_description;
}

void duplicate_entities_command::push_entry(const const_entity_handle handle) {
	handle.dispatch([&](const auto typed_handle) {
		using E = entity_type_of<decltype(typed_handle)>;
		using vector_type = make_data_vector<E>;

		duplicated_entities.get<vector_type>().push_back({ typed_handle.get_id() });
	});
}

bool duplicate_entities_command::empty() const {
	return size() == 0;
}

void duplicate_entities_command::redo(const editor_command_input in) {
	in.purge_selections();
	in.interrupt_tweakers();

	auto& cosm = in.get_cosmos();

	{
		auto& f = in.folder;

		auto& selections = f.view.selected_entities;
		selections.clear();

		duplicated_entities.for_each([&](auto& e) {
			const auto duplicated = cosmic::specific_clone_entity(cosm[e.source_id]);
			e.duplicated_id = duplicated.get_id();
			selections.emplace(e.duplicated_id);
		});
	}

	cosmic::reinfer_all_entities(cosm);
}

void duplicate_entities_command::undo(const editor_command_input in) const {
	in.purge_selections();
	in.interrupt_tweakers();

	auto& cosm = in.get_cosmos();

	duplicated_entities.for_each_reverse([&](const auto& e) {
		cosmic::undo_last_create_entity(cosm[e.duplicated_id]);
	});

	/* 
		At this point, some audiovisual systems might have dead ids with valid indirectors.
		However, the real_index fields inside relevant indirectors will be correctly set to -1,
		indicating that the entity is dead.

		After creating another entity via a different method than a redo of the just redone command,
		it might so happen that the audiovisual systems start pointing to a completely unrelated entity.
		
		We could fix this by always incrementing the id versions on creating via redoing,
		but the same problem will nevertheless persist in networked environments.
	*/
}
