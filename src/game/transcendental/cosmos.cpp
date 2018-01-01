#include <sol2/sol.hpp>

#include "augs/readwrite/memory_stream.h"

#include "augs/misc/randomization.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/organization/for_each_component_type.h"
#include "game/stateless_systems/destroy_system.h"
#include "game/detail/inventory/perform_transfer.h"

#include "augs/readwrite/lua_readwrite.h"
#include "augs/readwrite/byte_readwrite.h"

cosmos::cosmos(const cosmic_pool_size_type reserved_entities) 
	: solvable(reserved_entities) 
{}

const cosmos cosmos::zero = {};

void cosmos::clear() {
	*this = zero;
}

bool cosmos::operator==(const cosmos& b) const {
	if (!augs::equal_by_introspection(common.significant, b.common.significant)) {
		return false;
	}

	return get_solvable() == b.get_solvable();
}

bool cosmos::operator!=(const cosmos& b) const {
	return !operator==(b);
}

std::wstring cosmos::summary() const {
	return typesafe_sprintf(L"Entities: %x\n", get_entities_count());
}

rng_seed_type cosmos::get_rng_seed_for(const entity_id id) const {
	rng_seed_type transform_hash = 0;
	const auto tr = operator[](id).get_logic_transform();
	transform_hash = static_cast<rng_seed_type>(std::abs(tr.pos.x)*100.0);
	transform_hash += static_cast<rng_seed_type>(std::abs(tr.pos.y)*100.0);
	transform_hash += static_cast<rng_seed_type>(std::abs(tr.rotation)*100.0);

	return operator[](id).get_guid() + transform_hash;
}

entity_handle cosmos::create_entity(const entity_type_id type_id) {
	const auto new_handle = entity_handle { *this, get_solvable({}).allocate_next_entity() };
	new_handle.get<components::type>().change_type_to(type_id);
	return new_handle; 
}

entity_handle cosmos::create_entity_with_specific_guid(const entity_guid specific_guid) {
	return { *this, get_solvable({}).allocate_entity_with_specific_guid(specific_guid) };
}

entity_handle cosmos::clone_entity(const entity_id source_entity_id) {
	entity_handle source_entity = operator[](source_entity_id);

	if (source_entity.dead()) {
		return operator[](entity_id());
	}

	ensure(
		!source_entity.get<components::child>().parent.is_set() 
		&& "Cloning of entities that are children is not yet supported"
	);

	const auto new_entity = create_entity(source_entity.get_type_id());
	
	get_solvable({}).get_aggregate(new_entity).clone_components_except<
		/*
			These components will be cloned shortly,
			with due care to each of them.
		*/
		components::guid,
		components::fixtures,
		components::child,

		/*
			Let us keep the inferred state of the new entity disabled for a while,
			to avoid unnecessary regeneration.
		*/
		components::all_inferred_state
	>(get_solvable({}).get_aggregate(source_entity), get_solvable({}));

	if (new_entity.has<components::item>()) {
		new_entity.get<components::item>().current_slot.unset();
	}

	{
		for_each_component_type([&](auto c) {
			using component_type = decltype(c);

			if (const auto cloned_to_component = new_entity.get({}).find<component_type>(get_solvable({}))) {
				const auto& cloned_from_component = source_entity.get({}).template get<component_type>(get_solvable());

				if constexpr(allows_nontriviality_v<component_type>) {
					component_type::clone_children(
						*this,
						*cloned_to_component, 
						cloned_from_component
					);
				}
				else {
					augs::introspect(
						augs::recursive([&](auto&& self, auto, auto& into, const auto& from) {
							if constexpr(std::is_same_v<decltype(into), child_entity_id&>) {
								into = clone_entity(from);
							}
							else {
								augs::introspect_if_not_leaf(augs::recursive(self), into, from);
							}
						}),
						*cloned_to_component,
						cloned_from_component
					);
				}
			}
		});
	}

	
	if (source_entity.has<components::fixtures>()) {
		/*
			Copy all properties from the component of the source entity except the owner_body field.
			Exactly as if we were creating that component by hand.
		*/

		components::fixtures fixtures = source_entity.get<components::fixtures>().get_raw_component();
		const auto owner_of_the_source = fixtures.owner_body;
		fixtures.owner_body = entity_id();

		new_entity += fixtures;

		/*
			Only now assign the owner_body in a controllable manner.
		*/

		const bool source_owns_itself = owner_of_the_source == source_entity;

		if (source_owns_itself) {
			/*
				If the fixtures of the source entity were owned by the same entity,
				let the cloned entity also own itself
			*/
			new_entity.set_owner_body(new_entity);
		}
		else {
			/*
				If the fixtures of the source entity were owned by a different entity,
				let the cloned entity also be owned by that different entity
			*/
			new_entity.set_owner_body(owner_of_the_source);
		}
	}

	if (source_entity.is_inferred_state_activated()) {
		new_entity.get<components::all_inferred_state>().set_activated(true);
	}

	return new_entity;
}

void cosmos::delete_entity(const entity_id e) {
	const auto handle = operator[](e);
	
	if (handle.dead()) {
		return;
	}

	if (const auto container = handle.find<components::container>()) {
		drop_from_all_slots(*container, handle, [](const auto&){});
	}
	
	if (const auto current_slot = handle.get_current_slot()) {
		detail_unset_current_slot(handle);
	}

	const bool should_deactivate_inferred_state_to_avoid_repeated_regeneration 
		= handle.is_inferred_state_activated()
	;

	if (should_deactivate_inferred_state_to_avoid_repeated_regeneration) {
		handle.get<components::all_inferred_state>().set_activated(false);
	}

	// now manipulation of an entity without all_inferred_state component won't trigger redundant regeneration

	const auto maybe_fixtures = handle.find<components::fixtures>();

	if (maybe_fixtures != nullptr) {
		const auto owner_body = maybe_fixtures.get_owner_body();

		const bool should_release_dependency = owner_body != handle;

		if (should_release_dependency) {
			maybe_fixtures.set_owner_body(handle);
		}
	}

	/*
		Unregister that id as a parent from the relational system
	*/

	get_solvable_inferred({}).relational.handle_deletion_of_potential_parent(e);

	get_solvable({}).free_entity(e);
}

void cosmos::delete_entity_with_children(const entity_id id) {
	const auto handle = operator[](id);

	if (handle.dead()) {
		return;
	}

	destruction_queue q {id};
	deletion_queue d;

	destroy_system().mark_queued_entities_and_their_children_for_deletion(q, d, *this);
	destroy_system().perform_deletions(d, *this);
}

randomization cosmos::get_rng_for(const entity_id id) const {
	return{ get_rng_seed_for(id) };
}

bool cosmos::empty() const {
	return get_solvable().empty();
}

namespace augs {
	template <class Archive>
	void write_object_bytes(Archive& into, const cosmos& cosm) {
		auto& profiler = cosm.profiler;

		if constexpr(can_reserve_v<Archive>) {
			augs::byte_counter_stream counter_stream;

			{
				auto scope = measure_scope(profiler.size_calculation_pass);
				augs::write_bytes(counter_stream, cosm.get_common_significant());
				augs::write_bytes(counter_stream, cosm.get_solvable().significant);
			}

			auto scope = measure_scope(profiler.memory_allocation_pass);
			
			into.reserve(into.get_write_pos() + counter_stream.size());
		}

		{
			auto scope = measure_scope(profiler.serialization_pass);
			augs::write_bytes(into, cosm.get_common_significant());
			augs::write_bytes(into, cosm.get_solvable().significant);
		}
	}

	template <class Archive>
	void read_object_bytes(Archive& from, cosmos& cosm) {
		ensure(cosm.empty());

		auto& profiler = cosm.profiler;

		auto scope = measure_scope(profiler.deserialization_pass);

		cosm.change_common_significant([&](cosmos_common_significant& common) {
			augs::read_bytes(from, common);
			return changer_callback_result::DONT_REFRESH;
		});

		cosmic::change_solvable_significant(cosm, [&](cosmos_solvable_significant& significant) {
			augs::read_bytes(from, significant);
			return changer_callback_result::REFRESH;
		});
	}

	void write_object_lua(sol::table ar, const cosmos& cosm) {
		{
			auto common_table = ar.create();
			ar["common"] = common_table;

			write_lua(common_table, cosm.get_common_significant());
		}
		
		{
			auto pool_meta_table = ar.create();
			ar["pool_meta"] = pool_meta_table;
			pool_meta_table["reserved_entities_count"] = static_cast<unsigned>(cosm.get_solvable().get_maximum_entities());
		}

		auto entities_table = ar.create();
		ar["entities"] = entities_table;
		
		int entity_table_counter = 1;

		for (const auto& ent : cosm.get_solvable().get_entity_pool()) {
			auto this_entity_table = entities_table.create();
	
			ent.for_each_component(
				[&](const auto& comp) {
					using component_type = std::decay_t<decltype(comp)>;

					const auto this_component_name = get_type_name_strip_namespace<component_type>();

					auto this_component_table = this_entity_table.create();
					this_entity_table[this_component_name] = this_component_table;

					write_lua(this_component_table, comp);
				},
				cosm.get_solvable()
			);

			entities_table[entity_table_counter++] = this_entity_table;
		}
	}

	void read_object_lua(sol::table ar, cosmos& cosm) {
		ensure(cosm.empty());

		ensure(false);
#if TODO
		/* TODO: Fix it to use tuples of initial values when creating entities */
		/* TODO: Fix it to read guids properly instead of entity ids */
		auto refresh_when_done = augs::make_scope_guard([&cosm]() {
			cosm.reinfer_solvable();
		});

		{
			sol::object reserved_count = ar["pool_meta"]["reserved_entities_count"];
			cosm.reserve_storage_for_entities(reserved_count.as<unsigned>());
		}

		cosm.change_common_significant([&](cosmos_common_significant& common) {
			read_lua(ar["common"], common);

			return changer_callback_result::DONT_REFRESH;
		});

		int entity_table_counter = 1;
		auto entities_table = ar["entities"];

		while (true) {
			sol::table maybe_next_entity = entities_table[entity_table_counter];

			if (maybe_next_entity.valid()) {
				const auto new_entity = cosm.create_entity();

				for (auto key_value_pair : maybe_next_entity) {
					const auto component_name = key_value_pair.first.as<std::string>();

					for_each_component_type(
						[&](auto c) {
							using component_type = decltype(c);

							const auto this_component_name = get_type_name_strip_namespace<component_type>();

							if (this_component_name == component_name) {
								component_type c;
								read_lua(key_value_pair.second, c);
								new_entity.get({}).add<component_type>(c, cosm.get_solvable());
							}
						}
					);
				}
			}
			else {
				break;
			};

			++entity_table_counter;
		}
#endif
	}
}

template void augs::write_object_bytes(augs::memory_stream&, const cosmos&);
template void augs::read_object_bytes(augs::memory_stream&, cosmos&);

template void augs::write_object_bytes(std::ofstream&, const cosmos&);
template void augs::read_object_bytes(std::ifstream&, cosmos&);

