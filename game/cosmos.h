#pragma once
#include "misc/machine_entropy.h"
#include "game/types_specification/components_instantiation.h"
#include "game/types_specification/stateful_systems_instantiation.h"

#include "game/lists_of_processing_subjects.h"

#include "misc/delta.h"
#include "game/entity_id.h"

class cosmic_profiler;

class cosmos {
public:
	storage_for_all_components_and_aggregates components_and_aggregates;

	entity_id substantialize(aggregate_id);
public:
	storage_for_all_stateful_systems stateful_systems;
	lists_of_processing_subjects lists_of_processing_subjects;

	unsigned long long current_step_number = 0;
	double seconds_passed = 0.0;

	augs::fixed_delta delta;

	cosmos();

	void reserve_storage_for_aggregates(size_t);

	template<class... configured_components>
	entity_id create_from_definition(augs::configurable_components<configured_components...> definition, std::string debug_name = std::string()) {
		return substantialize(components_and_aggregates.allocate_configured_components(definition, debug_name));
	}

	template<class... configured_components>
	entity_id create_from_components(configured_components... args, std::string debug_name = std::string()) {
		augs::configurable_components<configured_components...> definition;
		definition.add(args...);
		return create_from_definition(definition, debug_name);
	}

	entity_id clone_entity(entity_id);
	void delete_entity(entity_id);

	void advance_deterministic_schemata(augs::machine_entropy input, cosmic_profiler&);
	void call_rendering_schemata(augs::variable_delta delta, cosmic_profiler&) const;

	std::vector<entity_id> get_list(list_of_processing_subjects) const;
	size_t entities_count() const;
	std::wstring summary() const;
};