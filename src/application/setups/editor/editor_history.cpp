#include "game/transcendental/entity_handle.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_history.h"
#include "application/setups/editor/editor_folder.h"

std::string delete_entities_command::describe() const {
	return typesafe_sprintf("Deleted %x entities", count_deleted());
}

void delete_entities_command::push_entry(const const_entity_handle handle) {
	handle.dispatch([&](const auto typed_handle) {
		using E = entity_type_of<decltype(typed_handle)>;
		using vector_type = make_data_vector<E>;

		const auto id = typed_handle.get_id();

		std::get<vector_type>(deleted_entities).push_back({ typed_handle.get() });
	});
}

std::size_t delete_entities_command::count_deleted() const {
	std::size_t total = 0;

	for_each_through_std_get(deleted_entities, [&](const auto& v) {
		total += v.size();
	});

	return total;
}

bool delete_entities_command::empty() const {
	return count_deleted() == 0;
}

void delete_entities_command::redo(editor_folder& f) {
	auto& cosm = f.work->world;

	for_each_through_std_get(deleted_entities, [&](auto& v) {
		for (auto& e : v) {
			e.undo_delete_input = *cosmic::delete_entity(cosm[e.content.guid]);
		}	
	});
}

void delete_entities_command::undo(editor_folder& f) const {
	auto& cosm = f.work->world;

	/* 
		NOTE: Pools should be independent, but to be theoretically pure,
		we should implement reverse_for_each_through_std_get.
	*/

	for_each_through_std_get(deleted_entities, [&](const auto& v) {
		for (const auto& e : reverse(v)) {
			cosmic::undo_delete_entity(cosm, e.undo_delete_input, e.content, reinference_type::NONE);
		}	
	});

	cosmic::reinfer_all_entities(cosm);
}
