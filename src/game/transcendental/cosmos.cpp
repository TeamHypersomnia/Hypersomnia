#include <sol2/sol.hpp>

#include "augs/readwrite/memory_stream.h"

#include "augs/misc/randomization.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/organization/for_each_component_type.h"

#include "augs/readwrite/lua_readwrite.h"
#include "augs/readwrite/byte_readwrite.h"

cosmos::cosmos(const cosmic_pool_size_type reserved_entities) 
	: solvable(reserved_entities) 
{}

const cosmos cosmos::zero;

void cosmos::clear() {
	*this = zero;
}

void cosmos::reinfer_all_caches() {
	auto scope = measure_scope(profiler.reinfer_all_caches_for);
	
	solvable.destroy_all_caches();
	infer_all_caches();
}

void cosmos::infer_all_caches() {
	for (const auto& ordered_pair : solvable.get_guid_to_id()) {
		infer_cache_for(operator[](ordered_pair.second));
	}

#if TODO_NAMES
	inferred.for_each([this](auto& sys) {
		sys.infer_additional_cache(common);
	});
#endif
}

void cosmos::destroy_cache_of(const const_entity_handle h) {
	auto destructor = [h](auto& sys) {
		sys.destroy_cache_of(h);
	};

	solvable.inferred.for_each(destructor);
}

void cosmos::infer_cache_for(const const_entity_handle h) {
	if (h.is_inferred_state_activated()) {
		auto constructor = [h](auto& sys) {
			sys.infer_cache_for(h);
		};

		solvable.inferred.for_each(constructor);
	}
}

bool cosmos::operator==(const cosmos& b) const {
	if (!augs::equal_by_introspection(common, b.common)) {
		return false;
	}

	return solvable == b.solvable;
}

bool cosmos::operator!=(const cosmos& b) const {
	return !operator==(b);
}

cosmos& cosmos::operator=(const cosmos_solvable_significant& b) {
	{
		auto scope = measure_scope(profiler.duplication);
		solvable.significant = b;
	}

	refresh_for_new_significant();
	return *this;
}


void cosmos::refresh_for_new_significant() {
	solvable.remap_guids();
	reinfer_all_caches();
}

void cosmos::reinfer_all_caches_for(const const_entity_handle h) {
	destroy_cache_of(h);
	infer_cache_for(h);
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

entity_handle cosmos::create_entity(const std::string& name) {
	return create_entity(to_wstring(name));
}

entity_handle cosmos::create_entity(const std::wstring& name_str) {
	return { *this, solvable.allocate_next_entity() };
}

entity_handle cosmos::create_entity_with_specific_guid(const entity_guid specific_guid) {
	return { *this, solvable.allocate_entity_with_specific_guid(specific_guid) };
}

entity_handle cosmos::clone_entity(const entity_id source_entity_id) {
	entity_handle source_entity = operator[](source_entity_id);

	if (source_entity.dead()) {
		return operator[](entity_id());
	}

	ensure(
		!source_entity.has<components::child>() 
		&& "Cloning of entities with child component is not yet supported"
	);

	const auto new_entity = create_entity(L"");
	
	new_entity.get().clone_components_except<
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
	>(source_entity.get(), solvable);

	if (new_entity.has<components::item>()) {
		new_entity.get<components::item>().current_slot.unset();
	}

	new_entity.make_cloned_child_entities_recursive(source_entity_id);
	
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

	solvable.inferred.relational.handle_deletion_of_potential_parent(e);

	solvable.free_entity(e);
}

randomization cosmos::get_rng_for(const entity_id id) const {
	return{ get_rng_seed_for(id) };
}

bool cosmos::empty() const {
	return solvable.empty();
}

namespace augs {
	template <class Archive>
	void write_object_bytes(Archive& into, const cosmos& cosm) {
		auto& profiler = cosm.profiler;

		if constexpr(can_reserve_v<Archive>) {
			augs::byte_counter_stream counter_stream;

			{
				auto scope = measure_scope(profiler.size_calculation_pass);
				augs::write_bytes(counter_stream, cosm.get_common_state());
				augs::write_bytes(counter_stream, cosm.solvable.significant);
			}

			auto scope = measure_scope(profiler.memory_allocation_pass);
			
			into.reserve(into.get_write_pos() + counter_stream.size());
		}

		{
			auto scope = measure_scope(profiler.serialization_pass);
			augs::write_bytes(into, cosm.get_common_state());
			augs::write_bytes(into, cosm.solvable.significant);
		}
	}

	template <class Archive>
	void read_object_bytes(Archive& from, cosmos& cosm) {
		ensure(cosm.empty());

		auto& profiler = cosm.profiler;

		auto refresh_when_done = augs::make_scope_guard([&cosm]() {
			cosm.refresh_for_new_significant();
		});

		auto scope = measure_scope(profiler.deserialization_pass);

		cosm.change_common_state([&](cosmos_common_state& common) {
			augs::read_bytes(from, common);

			return changer_callback_result::DONT_REFRESH;
		});

		augs::read_bytes(from, cosm.solvable.significant);
	}

	void write_object_lua(sol::table ar, const cosmos& cosm) {
		{
			auto common_table = ar.create();
			ar["common"] = common_table;

			write_lua(common_table, cosm.get_common_state());
		}
		
		{
			auto pool_meta_table = ar.create();
			ar["pool_meta"] = pool_meta_table;
			pool_meta_table["reserved_entities_count"] = static_cast<unsigned>(cosm.solvable.get_maximum_entities());
		}

		auto entities_table = ar.create();
		ar["entities"] = entities_table;
		
		int entity_table_counter = 1;

		for (const auto& ent : cosm.solvable.get_entity_pool()) {
			auto this_entity_table = entities_table.create();
	
			ent.for_each_component(
				[&](const auto& comp) {
					using component_type = std::decay_t<decltype(comp)>;

					const auto this_component_name = get_type_name_strip_namespace<component_type>();

					auto this_component_table = this_entity_table.create();
					this_entity_table[this_component_name] = this_component_table;

					write_lua(this_component_table, comp);
				},
				cosm.solvable
			);

			entities_table[entity_table_counter++] = this_entity_table;
		}
	}

	void read_object_lua(sol::table ar, cosmos& cosm) {
		ensure(cosm.empty());

		auto refresh_when_done = augs::make_scope_guard([&cosm]() {
			cosm.refresh_for_new_significant();
		});

		{
			sol::object reserved_count = ar["pool_meta"]["reserved_entities_count"];
			cosm.solvable.reserve_storage_for_entities(reserved_count.as<unsigned>());
		}

		cosm.change_common_state([&](cosmos_common_state& common) {
			read_lua(ar["common"], common);

			return changer_callback_result::DONT_REFRESH;
		});

		int entity_table_counter = 1;
		auto entities_table = ar["entities"];

		while (true) {
			sol::table maybe_next_entity = entities_table[entity_table_counter];

			if (maybe_next_entity.valid()) {
				const auto new_entity = cosm.create_entity("");

				/* guid entry must always exist */
				//components::guid g;
				//read_lua(maybe_next_entity["guid"], g);
				//cosm.

				for (auto key_value_pair : maybe_next_entity) {
					const auto component_name = key_value_pair.first.as<std::string>();

					for_each_component_type(
						[&](auto c) {
							using component_type = decltype(c);

							const auto this_component_name = get_type_name_strip_namespace<component_type>();

							if (this_component_name == component_name) {
								component_type c;
								read_lua(key_value_pair.second, c);
								new_entity.get().add<component_type>(c, cosm.solvable);
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
	}
}

template void augs::write_object_bytes(augs::memory_stream&, const cosmos&);
template void augs::read_object_bytes(augs::memory_stream&, cosmos&);

template void augs::write_object_bytes(std::ofstream&, const cosmos&);
template void augs::read_object_bytes(std::ifstream&, cosmos&);

