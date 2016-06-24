#pragma once
#include "misc/machine_entropy.h"

#include "entity_system/storage_for_components_and_aggregates.h"
#include "entity_system/storage_for_message_queues.h"
#include "entity_system/storage_for_stateful_systems.h"

#include "game/types_specification/all_components_declaration.h"
#include "game/types_specification/all_messages_declaration.h"
#include "game/types_specification/all_stateful_systems_declaration.h"

#include "game/lists_of_processing_subjects.h"

#include "misc/delta.h"
#include "game/entity_id.h"
#include "game/entity_handle_declaration.h"
#include "game/detail/inventory_slot_handle_declaration.h"
#include "game/global/all_settings.h"
#include "game/cosmic_profiler.h"
#include "game/step.h"

struct inventory_slot_id;
class cosmos {
	storage_for_all_components_and_aggregates components_and_aggregates;

public:
	storage_for_all_stateful_systems stateful_systems;
	lists_of_processing_subjects lists_of_processing_subjects;
	all_settings settings;

	mutable cosmic_profiler profiler;

	unsigned long long current_step_number = 0;
	double seconds_passed = 0.0;

	augs::fixed_delta delta;

	cosmos();

	void reserve_storage_for_entities(size_t);

	entity_handle create_entity(std::string debug_name);
	void construct_entity(entity_id);
	
	entity_handle clone_entity(entity_id);
	entity_handle clone_and_construct_entity(entity_id);
	
	void delete_entity(entity_id);

	void advance_deterministic_schemata(augs::machine_entropy input, step_state& initial_step_state);
	void call_rendering_schemata(augs::variable_delta delta, step_state& initial_step_state) const;

	entity_handle get_handle(entity_id);
	const_entity_handle get_handle(entity_id) const;
	inventory_slot_handle get_handle(inventory_slot_id);
	const_inventory_slot_handle get_handle(inventory_slot_id) const;

	template<class T>
	decltype(auto) to_handle_vector(std::vector<T> vec) {
		std::vector<decltype(get_handle())> handles;
		
		for (auto v : vec)
			handles.emplace_back(get_handle(v));
		
		return std::move(handles);
	}

	entity_handle dead_entity_handle();
	const_entity_handle dead_entity_handle() const;
	inventory_slot_handle dead_inventory_handle();
	const_inventory_slot_handle dead_inventory_handle() const;

	template<class T>
	decltype(auto) operator [](T id) {
		return get_handle(id);
	}

	template<class T>
	decltype(auto) operator [](T id) const {
		return get_handle(id);
	}

	randomization get_rng_for(entity_id) const;

	bool is_in(entity_id, processing_subjects) const;

	std::vector<entity_handle> get(processing_subjects);
	std::vector<const_entity_handle> get(processing_subjects) const;

	size_t entities_count() const;
	std::wstring summary() const;

	const storage_for_all_components_and_aggregates::aggregate_pool_type& get_pool() const;
	storage_for_all_components_and_aggregates::aggregate_pool_type& get_pool();

	template <class component>
	const augs::object_pool<component>& get_component_pool() const {
		return components_and_aggregates.get_component_pool<component>();
	}

	template <class component>
	augs::object_pool<component>& get_component_pool() {
		return components_and_aggregates.get_component_pool<component>();
	}
};