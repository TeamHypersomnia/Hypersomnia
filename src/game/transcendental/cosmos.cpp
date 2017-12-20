#include <sol2/sol.hpp>

#include "augs/readwrite/memory_stream.h"

#include "augs/misc/randomization.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/organization/for_each_component_type.h"

#include "augs/readwrite/lua_readwrite.h"
#include "augs/readwrite/byte_readwrite.h"

cosmos::cosmos(const cosmic_pool_size_type reserved_entities) {
	reserve_storage_for_entities(reserved_entities);
}

const cosmos cosmos::zero;

void cosmos::clear() {
	*this = zero;
}

void cosmos::increment_step() {
	++significant.common.meta.now.step;
}

void cosmos::reinfer_all_caches() {
	auto scope = measure_scope(profiler.reinfer_all_caches_for);
	
	destroy_all_caches();
	infer_all_caches();
}

void cosmos::destroy_all_caches() {
	inferred.~all_inferred_caches();
	new (&inferred) all_inferred_caches;

	const auto n = significant.entity_pool.capacity();

	inferred.for_each([n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	});
}

void cosmos::infer_all_caches() {
	for (const auto& ordered_pair : guid_to_id) {
		infer_cache_for(operator[](ordered_pair.second));
	}

	inferred.for_each([this](auto& sys) {
		sys.infer_additional_cache(significant.common);
	});
}

void cosmos::destroy_cache_of(const const_entity_handle h) {
	auto destructor = [h](auto& sys) {
		sys.destroy_cache_of(h);
	};

	inferred.for_each(destructor);
}

void cosmos::infer_cache_for(const const_entity_handle h) {
	if (h.is_inferred_state_activated()) {
		auto constructor = [h](auto& sys) {
			sys.infer_cache_for(h);
		};

		inferred.for_each(constructor);
	}
}

template <class T>
static bool components_equal_in_entities(
	const const_entity_handle e1, 
	const const_entity_handle e2
) {
	const auto maybe_1 = e1.find<T>();
	const auto maybe_2 = e2.find<T>();

	if (!maybe_1 && !maybe_2) {
		return true;
	}

	if (maybe_1) {
		if (maybe_2) {
			bool difference_found = false;

			if constexpr(is_component_synchronized_v<T>) {
				if (!augs::equal_by_introspection(maybe_1.get_raw_component(), maybe_2.get_raw_component())) {
					difference_found = true;
				}
			}
			else {
				if (!augs::equal_by_introspection(*maybe_1, *maybe_2)) {
					difference_found = true;
				}
			}

			if (!difference_found) {
				return true;
			}
		}
	}

	return false;
};

bool cosmos::operator==(const cosmos& b) const {
	ensure(guid_to_id.size() == get_entities_count());
	ensure(b.guid_to_id.size() == b.get_entities_count());

	if (get_entities_count() != b.get_entities_count()) {
		return false;
	}

	if (!augs::equal_by_introspection(significant.common, b.significant.common)) {
		return false;
	}

	for (const auto& it : guid_to_id) {
		const auto guid = it.first;

		const auto left = operator[](it.second);
		const auto right = b[guid];

		if (right.dead()) {
			return false;
		}

		bool difference_found = false;

		for_each_component_type([&](auto c) {
			if (!components_equal_in_entities<decltype(c)>(left, right)) {
				difference_found = true;
			}
		});

		if (difference_found) {
			return false;
		}
	}

	return true;
}

bool cosmos::operator!=(const cosmos& b) const {
	return !operator==(b);
}

cosmos& cosmos::operator=(const cosmos_significant_state& b) {
	{
		auto scope = measure_scope(profiler.duplication);
		significant = b;
	}

	refresh_for_new_significant_state();
	return *this;
}

void cosmos::assign_next_guid(const entity_handle new_entity) {
	const auto this_guid = significant.common.meta.next_entity_guid.value++;

	guid_to_id[this_guid] = new_entity;
	new_entity.get<components::guid>().value = this_guid;
}

void cosmos::clear_guid(const entity_handle cleared) {
	guid_to_id.erase(get_guid(cleared));
}

void cosmos::remap_guids() {
	guid_to_id.clear();

	for_each_entity_id([this](const entity_id id) {
		guid_to_id[get_guid(operator[](id))] = id;
	});
}

void cosmos::refresh_for_new_significant_state() {
	remap_guids();
	reinfer_all_caches();
}

void cosmos::reinfer_all_caches_for(const const_entity_handle h) {
	destroy_cache_of(h);
	infer_cache_for(h);
}

void cosmos::reserve_storage_for_entities(const cosmic_pool_size_type n) {
	get_entity_pool().reserve(n);
	reserve_all_components(n);

	inferred.for_each([n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	});
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

double cosmos::get_total_seconds_passed(const double view_interpolation_ratio) const {
	return get_total_seconds_passed() + get_fixed_delta().per_second(view_interpolation_ratio);
}

double cosmos::get_total_seconds_passed() const {
	return significant.common.meta.now.step * get_fixed_delta().in_seconds<double>();
}

decltype(augs::stepped_timestamp::step) cosmos::get_total_steps_passed() const {
	return significant.common.meta.now.step;
}

augs::stepped_timestamp cosmos::get_timestamp() const {
	return significant.common.meta.now;
}

augs::delta cosmos::get_fixed_delta() const {
	return significant.common.meta.delta;
}

void cosmos::set_steps_per_second(const unsigned steps) {
	significant.common.meta.delta = augs::delta::steps_per_second(steps);
}

unsigned cosmos::get_steps_per_second() const {
	return get_fixed_delta().in_steps_per_second();
}

entity_handle cosmos::allocate_new_entity() {
	if (get_entity_pool().full()) {
		throw std::runtime_error("Entities should be controllably reserved to avoid invalidation of entity_handles.");
	}

	return { *this, get_entity_pool().allocate() };
}

entity_handle cosmos::create_entity(const std::string& name) {
	return create_entity(to_wstring(name));
}

entity_handle cosmos::create_entity(const std::wstring& name_str) {
	auto new_entity = allocate_new_entity();
	new_entity.set_name(name_str);

	assign_next_guid(new_entity);
	return new_entity;
}

entity_handle cosmos::create_entity_with_specific_guid(const entity_guid specific_guid) {
	const auto new_entity = allocate_new_entity();

	guid_to_id[specific_guid] = new_entity;
	new_entity.get<components::guid>().value = specific_guid;
	return new_entity;
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

	const auto new_entity = allocate_new_entity();
	
	clone_all_components_except<
		/*
			These components will be cloned shortly,
			with due care to each of them.
		*/
		components::fixtures,
		components::child,

		/*
			Let us keep the inferred state of the new entity disabled for a while,
			to avoid unnecessary regeneration.
		*/
		components::all_inferred_state
	>(new_entity, source_entity);

	assign_next_guid(new_entity);

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

	clear_guid(handle);
	// now manipulation of an entity without all_inferred_state component won't trigger redundant regeneration

	const auto maybe_fixtures = handle.find<components::fixtures>();

	if (maybe_fixtures != nullptr) {
		const auto owner_body = maybe_fixtures.get_owner_body();

		const bool should_release_dependency = owner_body != handle;

		if (should_release_dependency) {
			maybe_fixtures.set_owner_body(handle);
		}
	}

	free_all_components(operator[](e));
	get_entity_pool().free(e);

	/*
		Unregister that id as a parent from the relational system
	*/

	inferred.relational.handle_deletion_of_potential_parent(e);
}

randomization cosmos::get_rng_for(const entity_id id) const {
	return{ get_rng_seed_for(id) };
}

bool cosmos::empty() const {
	return get_entities_count() == 0 && guid_to_id.empty();
}

namespace augs {
	template <class Archive>
	void write_object_bytes(Archive& into, const cosmos& cosm) {
		auto& profiler = cosm.profiler;

		if constexpr(can_reserve_v<Archive>) {
			augs::byte_counter_stream counter_stream;

			{
				auto scope = measure_scope(profiler.size_calculation_pass);
				augs::write_bytes(counter_stream, cosm.significant);
			}

			auto scope = measure_scope(profiler.memory_allocation_pass);
			
			into.reserve(into.get_write_pos() + counter_stream.size());
		}

		{
			auto scope = measure_scope(profiler.serialization_pass);
			augs::write_bytes(into, cosm.significant);
		}
	}

	template <class Archive>
	void read_object_bytes(Archive& from, cosmos& cosm) {
		ensure(cosm.empty());

		auto& profiler = cosm.profiler;

		auto refresh_when_done = augs::make_scope_guard([&cosm]() {
			cosm.refresh_for_new_significant_state();
		});

		auto scope = measure_scope(profiler.deserialization_pass);
		augs::read_bytes(from, cosm.significant);
	}

	void write_object_lua(sol::table ar, const cosmos& cosm) {
		{
			auto common_table = ar.create();
			ar["common"] = common_table;

			write_lua(common_table, cosm.significant.common);
		}
		
		{
			auto pool_meta_table = ar.create();
			ar["pool_meta"] = pool_meta_table;
			pool_meta_table["reserved_entities_count"] = static_cast<unsigned>(cosm.get_maximum_entities());
		}

		auto entities_table = ar.create();
		ar["entities"] = entities_table;
		
		int entity_table_counter = 1;

		for (const auto& ent : cosm.get_entity_pool()) {
			auto this_entity_table = entities_table.create();
	
			for_each_component_type(
				[&](auto c) {
					using component_type = decltype(c);

					auto create_component_table = [&](){
						const auto this_component_name = get_type_name_strip_namespace<component_type>();

						auto this_component_table = this_entity_table.create();
						this_entity_table[this_component_name] = this_component_table;
						
						return this_component_table;
					};
					
					if constexpr(is_component_fundamental_v<component_type>) {
						write_lua(create_component_table(), std::get<component_type>(ent.fundamentals)); 
					}
					else {
						const auto& pool = cosm.get_component_pool<component_type>();

						if (const auto maybe_component = pool.find(ent.get_id<component_type>())) {
							write_lua(create_component_table(), *maybe_component);
						}
					}
				}
			);

			entities_table[entity_table_counter++] = this_entity_table;
		}
	}

	void read_object_lua(sol::table ar, cosmos& cosm) {
		ensure(cosm.empty());

		auto refresh_when_done = augs::make_scope_guard([&cosm]() {
			cosm.refresh_for_new_significant_state();
		});

		{
			sol::object reserved_count = ar["pool_meta"]["reserved_entities_count"];
			cosm.reserve_storage_for_entities(reserved_count.as<unsigned>());
		}

		read_lua(ar["common"], cosm.significant.common);

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
								new_entity.allocator::template add<component_type>(c);
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

