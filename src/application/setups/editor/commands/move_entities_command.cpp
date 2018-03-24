#include "game/transcendental/cosmos_solvable_access.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/editor_command_input.h"
#include "application/setups/editor/commands/move_entities_command.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

using delta_type = move_entities_command::delta_type;
using moved_entities_type = move_entities_command::moved_entities_type;

template <class F, class... K>
static void on_each_independent_transform(
	cosmos& cosm,
	const moved_entities_type& subjects,
	F&& callback,
	K... keys	
) {
	subjects.for_each(
		[&](const auto& i) {
			const auto typed_handle = cosm[i];

			typed_handle.access_independent_transform(
				std::forward<F>(callback),
				keys...
			);
		}
	);
}

static void save_old_values(
	cosmos& cosm,
	const moved_entities_type& subjects,
	augs::ref_memory_stream& into
) {
	on_each_independent_transform(
		cosm,
		subjects,
		[&into](const auto& tr) {
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
		[&from](auto& tr) {
			augs::read_bytes(from, tr);
		},
		key
	);
}

static void move_entities(
	const cosmos_solvable_access key,
	cosmos& cosm,
	const moved_entities_type& subjects,
	const delta_type& dt
) {
	const auto si = cosm.get_si();
	const auto si_delta = dt.to_si_space(si);

	on_each_independent_transform(
		cosm,
		subjects,
		[si, si_delta, dt](auto& tr) {
			using T = std::decay_t<decltype(tr)>;

			if constexpr(std::is_same_v<T, physics_engine_transforms>) {
				tr.set(tr.get() + si_delta);
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
		},
		key
	);
}

std::string move_entities_command::describe() const {
	return built_description;
}

void move_entities_command::push_entry(const const_entity_handle handle) {
	handle.dispatch([&](const auto typed_handle) {
		using E = entity_type_of<decltype(typed_handle)>;
		using vector_type = make_data_vector<E>;

		const auto id = typed_handle.get_id();

		moved_entities.get<vector_type>().push_back({ typed_entity_id<E>(id.basic()) });
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

	delta = new_value;

	auto before_change_data = augs::cref_memory_stream(values_before_change);

	auto& cosm = in.get_cosmos();

	/* For improved determinism, unmove the entities first... */
	unmove_entities({}, cosm, moved_entities, before_change_data);

	/* ...and only now move by the new delta, exactly as if we were moving the entities for the first time. */
	move_entities({}, cosm, moved_entities, new_value);


	moved_entities.for_each([&](const auto id){
		const auto h = cosm[id];	

		cosmic::infer_caches_for(h);
	});
}

void move_entities_command::redo(const editor_command_input in) {
	auto& cosm = in.get_cosmos();

	auto before_change_data = augs::ref_memory_stream(values_before_change);
	ensure(values_before_change.empty());

	save_old_values(cosm, moved_entities, before_change_data);
	move_entities({}, cosm, moved_entities, delta);

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
	unmove_entities({}, cosm, moved_entities, before_change_data);
	values_before_change.clear();

	cosmic::reinfer_all_entities(cosm);

	auto& selections = in.folder.view.selected_entities;
	selections.clear();

	moved_entities.for_each([&](const auto id) {
		selections.emplace(id);
	});
}
