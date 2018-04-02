#include "game/transcendental/entity_handle.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/editor_folder.h"

#include "application/setups/editor/commands/paste_entities_command.h"

std::string paste_entities_command::describe() const {
	return built_description;
}

void paste_entities_command::push_entry(const const_entity_handle handle) {
	handle.dispatch([&](const auto typed_handle) {
		using E = entity_type_of<decltype(typed_handle)>;
		using vector_type = make_data_vector<E>;

		auto solvable = typed_handle.get();
		cosmic::make_suitable_for_cloning(solvable);

		pasted_entities.get<vector_type>().push_back({ solvable });
	});
}

bool paste_entities_command::empty() const {
	return size() == 0;
}

void paste_entities_command::redo(const editor_command_input in) {
	in.purge_selections();
	in.interrupt_tweakers();

	auto& cosm = in.get_cosmos();

	{
		auto& f = in.folder;

		auto& selections = f.view.selected_entities;
		selections.clear();

		pasted_entities.for_each([&](auto& e) {
			using E = entity_type_of<decltype(e.content)>;

			const auto pasted = cosmic::specific_paste_entity(
				cosm, typed_entity_flavour_id<E>(e.content.flavour_id), e.content.components
			);

			e.content.guid = pasted.get_guid();
			selections.emplace(pasted.get_id());
		});
	}

	cosmic::reinfer_all_entities(cosm);
}

void paste_entities_command::undo(const editor_command_input in) const {
	in.purge_selections();
	in.interrupt_tweakers();

	auto& cosm = in.get_cosmos();

	pasted_entities.for_each_reverse([&](auto& e) {
		cosmic::undo_last_create_entity(cosm[e.content.guid]);
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
