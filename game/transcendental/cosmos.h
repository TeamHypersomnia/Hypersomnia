#pragma once
#include <thread>
#include "game/build_settings.h"
#include "augs/misc/streams.h"
#include "game/transcendental/cosmic_entropy.h"

#include "augs/entity_system/operations_on_all_components_mixin.h"
#include "augs/entity_system/storage_for_message_queues.h"
#include "augs/entity_system/storage_for_systems.h"

#include "game/transcendental/types_specification/all_components_declaration.h"
#include "game/transcendental/types_specification/all_messages_declaration.h"
#include "game/transcendental/types_specification/all_systems_declaration.h"

#include "game/systems_insignificant/interpolation_system.h"
#include "game/systems_insignificant/past_infection_system.h"
#include "game/systems_insignificant/light_system.h"
#include "game/systems_temporary/dynamic_tree_system.h"
#include "game/systems_temporary/physics_system.h"
#include "game/systems_temporary/processing_lists_system.h"

#include "augs/misc/delta.h"
#include "augs/misc/easier_handle_getters_mixin.h"

#include "game/transcendental/entity_id.h"
#include "game/detail/inventory_slot_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/detail/inventory_slot_handle_declaration.h"
#include "game/detail/item_slot_transfer_request_declaration.h"
#include "game/global/all_settings.h"
#include "game/transcendental/cosmic_profiler.h"

class cosmic_delta;
struct data_living_one_step;

class cosmos : 
	private put_all_components_into<augs::operations_on_all_components_mixin, cosmos>::type, 
	public augs::easier_handle_getters_mixin<cosmos>
{
	void advance_deterministic_schemata(logic_step& step_state);

#if COSMOS_TRACKS_GUIDS
	friend class cosmic_delta;

	template<class T>
	friend void transform_component_guids_to_ids(T&, const cosmos&);

	std::map<unsigned, entity_id> guid_map_for_transport;

	void assign_next_guid(const entity_handle&);
	void clear_guid(const entity_handle&);
	unsigned get_guid(const const_entity_handle&) const;
public:
	void remap_guids();
private:
	entity_handle create_entity_with_specific_guid(const std::string& debug_name, const unsigned specific_guid);
#endif

	void destroy_substance_completely();
	void create_substance_completely();

	entity_handle allocate_new_entity();
public:
	storage_for_all_systems_temporary systems_temporary;
	mutable storage_for_all_systems_insignificant systems_insignificant;

	mutable cosmic_profiler profiler;
	augs::stream reserved_memory_for_serialization;

	class significant_state {
	public:
		class metadata {
			friend class cosmos;

			augs::fixed_delta delta;
			unsigned total_steps_passed = 0;

#if COSMOS_TRACKS_GUIDS
			unsigned next_entity_guid = 1;
#endif
		public:
			all_settings settings;

		} meta;

		aggregate_pool_type pool_for_aggregates;
		component_pools_type pools_for_components;

		bool operator==(const significant_state&) const;
		bool operator!=(const significant_state&) const;

	} significant;

	template <class Archive>
	void serialize(Archive& ar) {
		ar(CEREAL_NVP(significant));
		complete_resubstantiation();
	}

	cosmos(const unsigned reserved_entities = 0);
	cosmos(const cosmos&);

	cosmos& operator=(const cosmos&);
	cosmos& operator=(const significant_state&);

	bool operator==(const cosmos&) const;
	bool operator!=(const cosmos&) const;

	template<class Pre, class Post>
	void advance_deterministic_schemata(const cosmic_entropy& input, Pre pre_solve, Post post_solve) {
		data_living_one_step queues;
		logic_step step(*this, input, queues);

		pre_solve(step);
		advance_deterministic_schemata(step);
		post_solve(step);
	}

	void integrate_interpolated_transforms(const float seconds) const;
	void advance_deterministic_schemata(const cosmic_entropy& input);

	void reserve_storage_for_entities(const size_t);

	entity_handle create_entity(const std::string& debug_name);
	entity_handle clone_entity(const entity_id&);
	void delete_entity(const entity_id&);

	void refresh_for_new_significant_state();

	void complete_resubstantiation();
	void complete_resubstantiation(const const_entity_handle&);
	void destroy_substance_for_entity(const const_entity_handle&);
	void create_substance_for_entity(const const_entity_handle&);

	template<class System>
	void partial_resubstantiation(entity_handle handle) {
		auto& sys = systems_temporary.get<System>();

		sys.destruct(handle);

		if (handle.has<components::substance>())
			sys.construct(handle);
	}

#if COSMOS_TRACKS_GUIDS
	entity_handle get_entity_by_guid(unsigned);
	const_entity_handle get_entity_by_guid(unsigned) const;
	bool entity_exists_with_guid(unsigned) const;
#endif

	entity_handle get_handle(const entity_id&);
	const_entity_handle get_handle(const entity_id&) const;
	entity_handle get_handle(unversioned_entity_id);
	const_entity_handle get_handle(unversioned_entity_id) const;
	inventory_slot_handle get_handle(const inventory_slot_id&);
	const_inventory_slot_handle get_handle(const inventory_slot_id&) const;
	item_slot_transfer_request get_handle(const item_slot_transfer_request_data&);
	const_item_slot_transfer_request get_handle(const item_slot_transfer_request_data&) const;

	randomization get_rng_for(const entity_id&) const;

	std::vector<entity_handle> get(const processing_subjects);
	std::vector<const_entity_handle> get(const processing_subjects) const;

	size_t entities_count() const;
	size_t get_maximum_entities() const;
	std::wstring summary() const;

	float get_total_time_passed_in_seconds(const float view_interpolation_ratio) const;
	float get_total_time_passed_in_seconds() const;
	unsigned get_total_steps_passed() const;

	augs::stepped_timestamp get_timestamp() const;

	const augs::fixed_delta& get_fixed_delta() const;
	void set_fixed_delta(const augs::fixed_delta&);

	/* saving procedure is not const due to possible resubstantiation of the universe */
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

namespace augs {
	template<class A>
	bool read_object(A& ar, cosmos::significant_state& significant) {
		return read_object(ar, significant.meta) &&
		read_object(ar, significant.pools_for_components) &&
		read_object(ar, significant.pool_for_aggregates);
	}

	template<class A>
	void write_object(A& ar, const cosmos::significant_state& significant) {
		write_object(ar, significant.meta);
		write_object(ar, significant.pools_for_components);
		write_object(ar, significant.pool_for_aggregates);
	}
}