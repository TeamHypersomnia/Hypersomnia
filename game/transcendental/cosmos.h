#pragma once
#include "augs/build_settings/setting_empty_bases.h"

#include "augs/templates/introspection_utilities.h"

#include "augs/misc/randomization.h"
#include "augs/misc/streams.h"
#include "augs/misc/delta.h"
#include "augs/misc/enum_bitset.h"
#include "augs/misc/subscript_operator_for_get_handle_mixin.h"
#include "augs/misc/enum_associative_array.h"

#include "augs/entity_system/operations_on_all_components_mixin.h"
#include "augs/entity_system/storage_for_systems.h"

#include "game/build_settings.h"
#include "game/simulation_settings/all_simulation_settings.h"

#include "game/assets/physical_material_id.h"

#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/cosmic_profiler.h"
#include "game/transcendental/types_specification/all_components_declaration.h"
#include "game/transcendental/types_specification/all_messages_declaration.h"
#include "game/transcendental/types_specification/all_systems_declaration.h"
#include "game/transcendental/cosmos_structs.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/inventory/item_slot_transfer_request_declaration.h"
#include "game/detail/inventory/item_slot_transfer_request.h"

#include "game/systems_inferred/tree_of_npo_system.h"
#include "game/systems_inferred/physics_system.h"
#include "game/systems_inferred/processing_lists_system.h"

#include "game/assets/behaviour_tree.h"

class EMPTY_BASES cosmos : 
	private cosmos_base,
	public augs::subscript_operator_for_get_handle_mixin<cosmos>
{
public:
	storage_for_all_systems_inferred systems_inferred;

	mutable cosmic_profiler profiler;
	augs::stream reserved_memory_for_serialization;

	cosmos_significant_state significant;
private:
	std::vector<std::string> entity_debug_names;
	augs::enum_associative_array<assets::behaviour_tree_id, behaviour_tree> unserializable_behaviour_trees;

#if COSMOS_TRACKS_GUIDS
	std::map<entity_guid, entity_id> guid_map_for_transport;

	friend class cosmic_delta;

	template<class T>
	friend void transform_component_guids_to_ids_in_place(T&, const cosmos&);

	void delete_debug_name(const entity_id);

	void assign_next_guid(const entity_handle);
	void clear_guid(const entity_handle);
	entity_guid get_guid(const const_entity_handle) const;
public:
	void remap_guids();
private:
	entity_handle create_entity_with_specific_guid(
		const std::string& debug_name,
		const entity_guid specific_guid
	);
#endif

	void advance_deterministic_schemata_and_queue_destructions(const logic_step step_state);
	void perform_deletions(const logic_step);

	void destroy_inferred_state_completely();
	void create_inferred_state_completely();

	entity_handle allocate_new_entity();

public:
	cosmos(const unsigned reserved_entities = 0);
	cosmos(const cosmos&);

	cosmos& operator=(const cosmos&);
	cosmos& operator=(const cosmos_significant_state&);

	bool operator==(const cosmos&) const;
	bool operator!=(const cosmos&) const;

	template <class Pre, class Post>
	void advance_deterministic_schemata(
		const logic_step_input input,
		Pre pre_solve,
		Post post_solve
	) {
		thread_local data_living_one_step queues;
		logic_step step(*this, input, queues);

		pre_solve(step);
		advance_deterministic_schemata_and_queue_destructions(step);
		post_solve(const_logic_step(step));
		
		perform_deletions(step);
		queues.clear_all();
	}

	void advance_deterministic_schemata(const logic_step_input input);

	void reserve_storage_for_entities(const size_t);

	entity_handle create_entity(const std::string& debug_name);
	entity_handle clone_entity(const entity_id);
	void delete_entity(const entity_id);

	void refresh_for_new_significant_state();

	void complete_reinference();
	void complete_reinference(const const_entity_handle);
	void destroy_inferred_state_for_entity(const const_entity_handle);
	void create_inferred_state_for_entity(const const_entity_handle);
	
	const std::string& get_debug_name(entity_id) const;
	void set_debug_name(const entity_id, const std::string& new_debug_name);

	template <class System>
	void partial_reinference(const entity_handle handle) {
		auto& sys = systems_inferred.get<System>();

		sys.destroy_inferred_state(handle);

		if (handle.has<components::inferred_state>()) {
			sys.create_inferred_state(handle);
		}
	}

	behaviour_tree& get_handle(const assets::behaviour_tree_id);
	const behaviour_tree& get_handle(const assets::behaviour_tree_id) const;

#if COSMOS_TRACKS_GUIDS
	entity_handle get_handle(const entity_guid);
	const_entity_handle get_handle(const entity_guid) const;
	bool entity_exists_with_guid(const entity_guid) const;
#endif

	entity_handle get_handle(const entity_id);
	const_entity_handle get_handle(const entity_id) const;
	entity_handle get_handle(const unversioned_entity_id);
	const_entity_handle get_handle(const unversioned_entity_id) const;
	inventory_slot_handle get_handle(const inventory_slot_id);
	const_inventory_slot_handle get_handle(const inventory_slot_id) const;

#if COSMOS_TRACKS_GUIDS
	template <template <class> class Guidized, class source_id_type>
	Guidized<entity_guid> guidize(const Guidized<source_id_type>& id_source) const {
		return rewrite_members_and_transform_templated_type_into<entity_guid>(
			id_source,
			[this](auto& guid_member, const auto& id_member) {
				const auto handle = get_handle(id_member);
			
				if (handle.alive()) {
					guid_member = get_handle(id_member).get_guid();
				}
				else {
					guid_member = entity_guid();
				}
			}
		);
	}

	template <template <class> class Deguidized, class source_id_type>
	Deguidized<entity_id> deguidize(const Deguidized<source_id_type>& guid_source) const {
		return rewrite_members_and_transform_templated_type_into<entity_id>(
			guid_source,
			[this](auto& id_member, const auto& guid_member) {
				if (guid_member != entity_guid()) {
					id_member = guid_map_for_transport.at(guid_member);
				}
			}
		);
	}
#endif

	si_scaling get_si() const;

	randomization get_rng_for(const entity_id) const;
	size_t get_rng_seed_for(const entity_id) const;

	template <class F>
	decltype(auto) operator()(const entity_id subject, F callback) {
		callback(get_handle(subject));
	}

	template <class F>
	decltype(auto) operator()(const entity_id subject, F callback) const {
		callback(get_handle(subject));
	}

	size_t get_count_of(const processing_subjects list_type) const {
		return systems_inferred.get<processing_lists_system>().get(list_type).size();
	}

	template <class F>
	void for_each(
		const processing_subjects list_type, 
		F callback,
		augs::enum_bitset<subjects_iteration_flag> flags = {}
	) {
		if (flags.test(subjects_iteration_flag::POSSIBLE_ITERATOR_INVALIDATION)) {
			const auto targets = systems_inferred.get<processing_lists_system>().get(list_type);

			for (const auto& subject : targets) {
				operator()(subject, callback);
			}
		}
		else {
			for (const auto& subject : systems_inferred.get<processing_lists_system>().get(list_type)) {
				operator()(subject, callback);
			}
		}
	}

	template <class F>
	void for_each(const processing_subjects list_type, F callback) const {
		for (const auto& subject : systems_inferred.get<processing_lists_system>().get(list_type)) {
			operator()(subject, callback);
		}
	}
	
	size_t entities_count() const;
	size_t get_maximum_entities() const;
	std::wstring summary() const;

	double get_total_time_passed_in_seconds(const double view_interpolation_ratio) const;
	double get_total_time_passed_in_seconds() const;
	unsigned get_total_steps_passed() const;

	augs::stepped_timestamp get_timestamp() const;

	const augs::delta& get_fixed_delta() const;
	void set_fixed_delta(const augs::delta&);
	void set_fixed_delta(const unsigned steps_per_second);

	/* saving procedure is not const due to possible reinference of the universe */
	void save_to_file(const std::string&);
	bool load_from_file(const std::string&);

	template <class D>
	void for_each_entity_id(D pred) {
		get_aggregate_pool().for_each_id(pred);
	}

	template <class D>
	void for_each_entity_id(D pred) const {
		get_aggregate_pool().for_each_id(pred);
	}

	auto& get_aggregate_pool() {
		return significant.pool_for_aggregates;
	}

	const auto& get_aggregate_pool() const {
		return significant.pool_for_aggregates;
	}

	template<class T>
	auto& get_component_pool() {
		return std::get<augs::pool<T>>(significant.pools_for_components);
	}

	template<class T>
	const auto& get_component_pool() const {
		return std::get<augs::pool<T>>(significant.pools_for_components);
	}
};

inline si_scaling cosmos::get_si() const {
	return significant.meta.settings.si;
}

inline behaviour_tree& cosmos::get_handle(const assets::behaviour_tree_id id) {
	return unserializable_behaviour_trees[id];
}

inline const behaviour_tree& cosmos::get_handle(const assets::behaviour_tree_id id) const {
	return unserializable_behaviour_trees[id];
}

#if COSMOS_TRACKS_GUIDS
inline entity_handle cosmos::get_handle(const entity_guid guid) {
	return get_handle(guid_map_for_transport.at(guid));
}

inline const_entity_handle cosmos::get_handle(const entity_guid guid) const {
	return get_handle(guid_map_for_transport.at(guid));
}

inline bool cosmos::entity_exists_with_guid(const entity_guid guid) const {
	return guid_map_for_transport.find(guid) != guid_map_for_transport.end();
}

inline entity_guid cosmos::get_guid(const const_entity_handle handle) const {
	return handle.get_guid();
}
#endif

inline entity_handle cosmos::get_handle(const entity_id id) {
	return { *this, id };
}

inline const_entity_handle cosmos::get_handle(const entity_id id) const {
	return { *this, id };
}

inline entity_handle cosmos::get_handle(const unversioned_entity_id id) {
	return { *this, get_aggregate_pool().make_versioned(id) };
}

inline const_entity_handle cosmos::get_handle(const unversioned_entity_id id) const {
	return { *this, get_aggregate_pool().make_versioned(id) };
}

inline inventory_slot_handle cosmos::get_handle(const inventory_slot_id id) {
	return { *this, id };
}

inline const_inventory_slot_handle cosmos::get_handle(const inventory_slot_id id) const {
	return { *this, id };
}

inline randomization cosmos::get_rng_for(const entity_id id) const {
	return{ get_rng_seed_for(id) };
}

inline size_t cosmos::entities_count() const {
	return significant.pool_for_aggregates.size();
}

inline size_t cosmos::get_maximum_entities() const {
	return significant.pool_for_aggregates.capacity();
}