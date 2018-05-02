#include "game/transcendental/cosmos_solvable_access.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/commands/move_entities_command.h"
#include "application/setups/editor/gui/editor_entity_selector.h"
#include "application/setups/editor/gui/find_aabb_of.h"
#include "application/setups/editor/detail/editor_transform_utils.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

using delta_type = move_entities_command::delta_type;
using moved_entities_type = move_entities_command::moved_entities_type;

static void save_old_values(
	cosmos& cosm,
	const moved_entities_type& subjects,
	augs::ref_memory_stream& into
) {
	on_each_independent_transform(
		cosm,
		subjects,
		[&into](const auto& tr, auto&) {
			augs::write_bytes(into, tr);
		}
	);
}

static void unmove_entities(
	const cosmos_solvable_access key,
	cosmos& cosm,
	const moved_entities_type& subjects,
	augs::cref_memory_stream& from
) {
	on_each_independent_transform(
		cosm,
		subjects,
		[&from](auto& tr, auto&) {
			augs::read_bytes(from, tr);
		},
		key
	);
}

static void move_entities(
	const cosmos_solvable_access key,
	cosmos& cosm,
	const moved_entities_type& subjects,
	const delta_type& dt,
	const std::optional<vec2> rotation_center
) {
	const auto si = cosm.get_si();
	const auto dt_si = dt.to_si_space(si);

	if (rotation_center) {
		const auto center = *rotation_center;
		const auto center_meters = si.get_meters(center);

		auto rotator = 
			[&](auto& tr, auto&) {
				using T = remove_cref<decltype(tr)>;

				if constexpr(std::is_same_v<T, physics_engine_transforms>) {
					auto new_transform = tr.get();
					new_transform.rotate_radians_with_90_multiples(dt_si.rotation, center_meters);

					fix_pixel_imperfections(new_transform, si);

					tr.set(new_transform);
				}
				else if constexpr(std::is_same_v<T, components::transform>) {
					tr.rotate_degrees_with_90_multiples(dt.rotation, center);
					fix_pixel_imperfections(tr);
				}
				else if constexpr(std::is_same_v<T, vec2>) {
					augs::rotate_degrees_with_90_multiples(tr, center, dt.rotation);
					fix_pixel_imperfections(tr);
				}
				else {
					static_assert(always_false_v<T>, "Unknown transform type.");
				}
			}
		;

		on_each_independent_transform(cosm, subjects, rotator, key);
	}
	else {
		auto mover = 
			[&](auto& tr, auto&) {
				using T = remove_cref<decltype(tr)>;

				if constexpr(std::is_same_v<T, physics_engine_transforms>) {
					tr.set(tr.get() + dt_si);
				}
				else if constexpr(std::is_same_v<T, components::transform>) {
					tr += dt;
				}
				else if constexpr(std::is_same_v<T, vec2>) {
					tr += dt.pos;
				}
				else {
					static_assert(always_false_v<T>, "Unknown transform type.");
				}
			}
		;

		on_each_independent_transform(cosm, subjects, mover, key);
	}
}

std::string move_entities_command::describe() const {
	if (rotation_center) {
		return "Rotated by " + std::to_string(move_by.rotation) + "*: " + built_description;
	}

	return typesafe_sprintf("Moved by %x: ", move_by.pos) + built_description;
}

void move_entities_command::push_entry(const const_entity_handle handle) {
	handle.dispatch([&](const auto typed_handle) {
		using E = entity_type_of<decltype(typed_handle)>;
		using vector_type = make_data_vector<E>;

		const auto id = typed_handle.get_id();

		moved_entities.get<vector_type>().push_back({ typed_entity_id<E>(id.basic()) });
	});
}

void move_entities_command::unmove_entities(cosmos& cosm) {
	auto before_change_data = augs::cref_memory_stream(values_before_change);

	/* For improved determinism, unmove the entities first... */
	::unmove_entities({}, cosm, moved_entities, before_change_data);
}

void move_entities_command::reinfer_moved(cosmos& cosm) {
	moved_entities.for_each([&](const auto id){
		const auto handle = cosm[id];	
		handle.infer_transform();
	});
}

void move_entities_command::rewrite_change(
	const delta_type& new_value,
	const editor_command_input in
) {
	/* 
		Reproducing delta from existent values is not immediately obvious,
		so let's just store the delta. 
	*/

	move_by = new_value;

	auto& cosm = in.get_cosmos();

	/* For improved determinism, client of this function should unmove the entities first... */
	/* ...and only now move by the new delta, exactly as if we were moving the entities for the first time. */

	move_entities({}, cosm, moved_entities, new_value, rotation_center);
	reinfer_moved(cosm);
}

void move_entities_command::redo(const editor_command_input in) {
	auto& cosm = in.get_cosmos();

	auto before_change_data = augs::ref_memory_stream(values_before_change);
	ensure(values_before_change.empty());

	save_old_values(cosm, moved_entities, before_change_data);
	move_entities({}, cosm, moved_entities, move_by, rotation_center);

	cosmic::reinfer_all_entities(cosm);

	auto& selections = in.folder.view.selected_entities;
	selections.clear();

	moved_entities.for_each([&](const auto id) {
		selections.emplace(id);
	});
}

void move_entities_command::undo(const editor_command_input in) {
	auto& cosm = in.get_cosmos();

	auto before_change_data = augs::cref_memory_stream(values_before_change);
	::unmove_entities({}, cosm, moved_entities, before_change_data);
	values_before_change.clear();

	cosmic::reinfer_all_entities(cosm);

	auto& selections = in.folder.view.selected_entities;
	selections.clear();

	moved_entities.for_each([&](const auto id) {
		selections.emplace(id);
	});
}
