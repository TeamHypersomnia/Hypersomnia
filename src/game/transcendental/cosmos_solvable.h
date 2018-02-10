#pragma once
#include <map>

#include "augs/misc/timing/delta.h"
#include "augs/misc/randomization_declaration.h"

#include "augs/templates/introspection_utils/rewrite_members.h"
#include "augs/templates/for_each_std_get.h"

#include "augs/entity_system/storage_for_systems.h"

#include "game/assets/ids/behaviour_tree_id.h"
#include "game/assets/behaviour_tree.h"

#include "game/transcendental/cosmic_profiler.h"

#if STATICALLY_ALLOCATE_ENTITIES
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

	template <class A, class B>
	struct allocation_result {
		A key;
		B& object;
	};

	template <class E, class... Args>
	auto allocate_new_entity(const entity_guid new_guid, Args&&... args) {
		auto& pool = significant.get_pool<E>();

		if (pool.full()) {
			throw std::runtime_error("Entities should be controllably reserved to avoid invalidation of entity_handles.");
		}

		const auto result = pool.allocate(new_guid, std::forward<Args>(args)...);

		allocation_result<typed_entity_id<E>, decltype(result.object)> output {
			result.key, result.object
		};

		return output;
	}

	void clear_guid(const entity_id);

	template <class C, class F>
	static decltype(auto) on_entity_impl(
		C& self,
		const entity_id id,
		F callback	
	) {
		return get_by_dynamic_id(
			self.significant.entity_pools,
			id.type_id.get_index(),
			[&](auto& pool) {	
				return callback(pool.find(id.basic()));
			}
		);
	}

	entity_guid get_guid(const entity_id) const;

	guid_cache guid_to_id;

public:
	cosmos_solvable_significant significant;
	cosmos_solvable_inferred inferred;

	cosmos_solvable() = default;
	explicit cosmos_solvable(const cosmic_pool_size_type reserved_entities);

	void reserve_storage_for_entities(const cosmic_pool_size_type);

	template <class E, class... Args>
	auto allocate_next_entity(Args&&... args) {
		const auto next_guid = significant.clock.next_entity_guid.value++;
		return allocate_entity_with_specific_guid<E>(next_guid, std::forward<Args>(args)...);
	}

	template <class E, class... Args>
	auto allocate_entity_with_specific_guid(const entity_guid specific_guid, Args&&... args) {
		const auto result = allocate_new_entity<E>(specific_guid, std::forward<Args>(args)...);
		guid_to_id[specific_guid] = entity_id(result.key);
		return result;
	}

	void free_entity(entity_id);

	void destroy_all_caches();

	void increment_step();
	void clear();

	void set_steps_per_second(const unsigned steps_per_second);

	entity_id to_versioned(const unversioned_entity_id) const;

	entity_id get_entity_id_by(const entity_guid) const;

	template <template <class> class Guidized, class source_id_type>
	Guidized<entity_guid> guidize(const Guidized<source_id_type>& id_source) const {
		return rewrite_members_and_transform_templated_type_into<entity_guid>(
			id_source,
			[this](auto& guid_member, const auto& id_member) {
				on_entity(id_member, [&](const auto* e){ 
					if (e) {
						guid_member = e->guid;
					}
					else {
						guid_member = entity_guid();
					}
				});
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

	std::unordered_set<entity_id> get_entities_by_flavour_id(const entity_flavour_id&) const;
	
	std::size_t get_entities_count() const;
	
	template <class E>
	auto get_count_of() const {
		return significant.template get_pool<E>().size();
	}
	
	auto get_count_of(const processing_subjects list_type) const {
		return inferred.processing.get(list_type).size();
	}

	template <class E>
	std::size_t get_maximum_count_of() const {
		return significant.get_pool<E>().capacity();
	}

	double get_total_seconds_passed(const double view_interpolation_ratio) const;
	double get_total_seconds_passed() const;
	decltype(augs::stepped_timestamp::step) get_total_steps_passed() const;

	augs::stepped_timestamp get_timestamp() const;

	augs::delta get_fixed_delta() const;
	unsigned get_steps_per_second() const;

	template <class F>
	void for_each_pool(F&& callback) {
		for_each_through_std_get(
			significant.entity_pools,
			std::forward<F>(callback)
		);
	}

	template <class F>
	void for_each_pool(F&& callback) const {
		for_each_through_std_get(
			significant.entity_pools,
			std::forward<F>(callback)
		);
	}

	bool empty() const;

	const auto& get_guid_to_id() const {
		return guid_to_id;
	}

	template <class F>
	decltype(auto) on_entity(const entity_id id, F&& callback) {
		return on_entity_impl(*this, id, std::forward<F>(callback));
	}	

	template <class F>
	decltype(auto) on_entity(const entity_id id, F&& callback) const {
		return on_entity_impl(*this, id, std::forward<F>(callback));
	}	

	template <class F>
	decltype(auto) on_entity(const entity_guid id, F&& callback) {
		return on_entity_impl(*this, get_entity_id_by(id), std::forward<F>(callback));
	}	

	template <class F>
	decltype(auto) on_entity(const entity_guid id, F&& callback) const {
		return on_entity_impl(*this, get_entity_id_by(id), std::forward<F>(callback));
	}	
};

inline entity_id cosmos_solvable::get_entity_id_by(const entity_guid guid) const {
	if (const auto id = mapped_or_nullptr(guid_to_id, guid)) {
		return *id;
	}

	return {};
}

inline std::unordered_set<entity_id> cosmos_solvable::get_entities_by_flavour_id(const entity_flavour_id& id) const {
	return inferred.name.get_entities_by_flavour_id(id);
}

inline entity_id cosmos_solvable::to_versioned(const unversioned_entity_id id) const {
	return { 
		significant.on_pool(
			id.type_id, 
			[id](const auto& p){ return p.to_versioned(id); }
		), 
		id.type_id 
	};
}

inline std::size_t cosmos_solvable::get_entities_count() const {
	std::size_t total = 0u;

	for_each_pool([&total](const auto& p) { total += p.size(); } );

	return total;
}

inline entity_guid cosmos_solvable::get_guid(const entity_id id) const {
	return on_entity(id, [](const auto* const e) { return e->guid; });
}

