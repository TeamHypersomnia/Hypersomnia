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
// #include "game/step_state.h"

class cosmic_profiler;

struct inventory_slot_id;
class cosmos {
	storage_for_all_components_and_aggregates components_and_aggregates;

	entity_id substantialize(entity_id);

public:
	storage_for_all_stateful_systems stateful_systems;
	lists_of_processing_subjects lists_of_processing_subjects;
	all_settings settings;

	mutable cosmic_profiler profiler;

	unsigned long long current_step_number = 0;
	double seconds_passed = 0.0;

	augs::fixed_delta delta;

	cosmos();

	void reserve_storage_for_aggregates(size_t);

	entity_handle create_entity(std::string debug_name);

	template<class... configured_components>
	entity_handle create_from_definition(augs::configurable_components<configured_components...> definition, std::string debug_name = std::string()) {
		return substantialize(components_and_aggregates.allocate_configured_components(definition, debug_name));
	}

	template<class... configured_components>
	entity_handle create_from_components(configured_components... args, std::string debug_name = std::string()) {
		augs::configurable_components<configured_components...> definition;
		definition.add(args...);
		return create_from_definition(definition, debug_name);
	}

	entity_handle clone_entity(entity_id);
	void delete_entity(entity_id);


	template<class F>
	void special_deterministic_step(augs::machine_entropy input, F pred) {
		step_state step;
		pred(*this, step);

		advance_deterministic_schemata(input, step);
	}

	template<class F>
	void special_rendering_step(augs::machine_entropy input, F pred) {
		step_state step;
		pred(*this, step);

		advance_deterministic_schemata(input, step);
	}

	void advance_deterministic_schemata(augs::machine_entropy input, step_state initial_step_state = step_state());
	void call_rendering_schemata(augs::variable_delta delta, step_state initial_step_state = step_state()) const;

	entity_handle get_handle(entity_id);
	const_entity_handle get_handle(entity_id) const;
	inventory_slot_handle get_handle(inventory_slot_id);
	const_inventory_slot_handle get_handle(inventory_slot_id) const;

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