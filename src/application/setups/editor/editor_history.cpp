#include "game/transcendental/entity_handle.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_history.h"
#include "application/setups/editor/editor_folder.h"

void delete_entities_command::push_entry(const const_entity_handle handle) {
	handle.dispatch([&](const auto typed_handle) {
		using E = entity_type_of<decltype(typed_handle)>;
		using vector_type = make_entity_vector<E>;

		std::get<vector_type>(deleted_entities).push_back(typed_handle.get());
	});
}

bool delete_entities_command::empty() const {
	std::size_t total = 0;

	for_each_through_std_get(deleted_entities, [&](const auto& v) {
		total += v.size();
	});

	return total == 0;
}

void delete_entities_command::redo(editor_folder& f) const {
	auto& cosm = f.work->world;

	for_each_through_std_get(deleted_entities, [&](const auto& v) {
		for (const auto& e : v) {
			delete_entity_with_children(cosm[e.guid]);
		}	
	});
}

void delete_entities_command::undo(editor_folder& f) const {
	auto& cosm = f.work->world;

	for_each_through_std_get(deleted_entities, [&](const auto& v) {
		for (const auto& e : v) {
			cosmic::undelete_entity(cosm, e);
		}	
	});
}
