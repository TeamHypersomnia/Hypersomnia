#pragma once
#include <map>

#include "augs/misc/timing/delta.h"
#include "augs/misc/randomization_declaration.h"
#include "augs/templates/introspection_utils/rewrite_members.h"

#include "augs/entity_system/storage_for_systems.h"

#include "game/assets/ids/behaviour_tree_id.h"
#include "game/assets/behaviour_tree.h"

#include "game/organization/all_fundamental_component_includes.h"
#include "augs/entity_system/component_aggregate.h"

#include "game/transcendental/cosmic_profiler.h"

#if STATICALLY_ALLOCATE_ENTITIES_NUM
#include "game/organization/all_component_includes.h"
#else
#include "game/organization/all_components_declaration.h"
#endif

#include "game/organization/all_messages_declaration.h"
#include "game/transcendental/cosmos_solvable_inferred.h"
#include "game/transcendental/cosmos_solvable_significant.h"
#include "game/transcendental/entity_id.h"

#include "game/assets/behaviour_tree.h"

class cosmos_solvable {
	using guid_cache = std::map<entity_guid, entity_id>;

	static const cosmos_solvable zero;

	entity_id allocate_new_entity();
	void clear_guid(const entity_id);

	guid_cache guid_to_id;

public:
	cosmos_solvable_significant significant;
	cosmos_solvable_inferred inferred;

	cosmos_solvable() = default;
	explicit cosmos_solvable(const cosmic_pool_size_type reserved_entities);

	void reserve_storage_for_entities(const cosmic_pool_size_type);

	entity_id allocate_next_entity();
	entity_id allocate_entity_with_specific_guid(const entity_guid specific_guid);

	void clone_components_except_sensitives(
		cosmic_entity& into,
		const cosmic_entity& from
	);

	void free_entity(entity_id);

	template <class D>
	void for_each_entity_id(D pred) const {
		get_entity_pool().for_each_id(pred);
	}

	void destroy_all_caches();

	void increment_step();
	void remap_guids();
	void clear();

	void set_steps_per_second(const unsigned steps_per_second);

	entity_guid get_guid(const entity_id) const;
	entity_id make_versioned(const unversioned_entity_id) const;

	entity_id get_entity_id_by(const entity_guid) const;

	template <template <class> class Guidized, class source_id_type>
	Guidized<entity_guid> guidize(const Guidized<source_id_type>& id_source) const {
		return rewrite_members_and_transform_templated_type_into<entity_guid>(
			id_source,
			[this](auto& guid_member, const auto& id_member) {
				if (get_entity_pool().alive(id_member)) {
					guid_member = get_guid(id_member);
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
					id_member = guid_to_id.at(guid_member);
				}
			}
		);
	}

	std::size_t get_count_of(const processing_subjects list_type) const {
		return inferred.processing_lists.get(list_type).size();
	}
	
	std::unordered_set<entity_id> get_entities_by_type_id(const entity_type_id&) const;
	
	std::size_t get_entities_count() const;
	std::size_t get_maximum_entities() const;

	double get_total_seconds_passed(const double view_interpolation_ratio) const;
	double get_total_seconds_passed() const;
	decltype(augs::stepped_timestamp::step) get_total_steps_passed() const;

	augs::stepped_timestamp get_timestamp() const;

	augs::delta get_fixed_delta() const;
	unsigned get_steps_per_second() const;

	auto& get_entity_pool() {
		return significant.entity_pool;
	}

	const auto& get_entity_pool() const {
		return significant.entity_pool;
	}

	template <class T>
	auto& get_component_pool() {
		return std::get<cosmic_object_pool<T>>(significant.component_pools);
	}

	template <class T>
	const auto& get_component_pool() const {
		return std::get<cosmic_object_pool<T>>(significant.component_pools);
	}

	bool operator==(const cosmos_solvable&) const;
	bool operator!=(const cosmos_solvable&) const;

	bool empty() const;

	const auto& get_guid_to_id() const {
		return guid_to_id;
	}

	const auto& get_aggregate(const entity_id id) const {
		return get_entity_pool().get(id);
	}	

	auto& get_aggregate(const entity_id id) {
		return get_entity_pool().get(id);
	}	

	auto& get_aggregate(const entity_guid guid) {
		return get_aggregate(get_entity_id_by(guid));
	}	

	const auto& get_aggregate(const entity_guid guid) const {
		return get_aggregate(get_entity_id_by(guid));
	}	
};

inline entity_id cosmos_solvable::get_entity_id_by(const entity_guid guid) const {
	if (const auto id = mapped_or_nullptr(guid_to_id, guid)) {
		return *id;
	}

	return {};
}

inline std::unordered_set<entity_id> cosmos_solvable::get_entities_by_type_id(const entity_type_id& id) const {
	return inferred.name.get_entities_by_type_id(id);
}

inline entity_id cosmos_solvable::make_versioned(const unversioned_entity_id id) const {
	return get_entity_pool().make_versioned(id);
}

inline std::size_t cosmos_solvable::get_entities_count() const {
	return significant.entity_pool.size();
}

inline std::size_t cosmos_solvable::get_maximum_entities() const {
	return significant.entity_pool.capacity();
}

inline entity_guid cosmos_solvable::get_guid(const entity_id id) const {
	return get_aggregate(id).get<components::guid>(*this).get_value();
}

