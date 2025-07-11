#pragma once
#include <cstddef>
#include <cstdint>
#include <map>
#include "augs/templates/identity_templates.h"

#include "augs/misc/timing/delta.h"

#include "game/organization/all_component_includes.h"

#include "game/cosmos/cosmos_solvable_inferred.h"
#include "game/cosmos/cosmos_solvable_significant.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_creation_error.h"
#include "game/cosmos/allocate_new_entity_access.h"

struct entity_creation_input {
	allocate_new_entity_access access;
	raw_entity_flavour_id flavour_id;
};

class cosmos_solvable {
	static const cosmos_solvable zero;

	template <class A, class B>
	struct allocation_result {
		A key;
		B& object;
	};

	template <class E>
	auto allocate_new_entity(entity_creation_input);

	template <class E, class... Args>
	auto detail_undo_free_entity(Args&&... args);

	template <class C, class F>
	static decltype(auto) on_entity_meta_impl(
		C& self,
		const entity_id id,
		F callback	
	);

	template <template <class> class Predicate, class S, class F>
	static void for_each_entity_impl(S& self, F callback);

public:
	cosmos_solvable_significant significant;
	cosmos_solvable_inferred inferred;

	cosmos_solvable() = default;
	explicit cosmos_solvable(const cosmic_pool_size_type reserved_entities);

	void reserve_storage_for_entities(const cosmic_pool_size_type);

	template <class E>
	auto allocate_next_entity(entity_creation_input);

	template <class E, class... Args>
	auto undo_free_entity(Args&&... undo_free_args);

	template <class E>
	auto allocate_entity_impl(entity_creation_input);

	std::optional<cosmic_pool_undo_free_input> free_entity(entity_id);
	void undo_last_allocate_entity(entity_id);

	void destroy_all_caches();

	void increment_step();
	void clear();

	void set_steps_per_second(const unsigned steps_per_second);

	entity_id get_versioned(const unversioned_entity_id&) const;
	entity_id find_versioned(const unversioned_entity_id&) const;

	template <class E>
	const auto& get_entities_by_flavour_id(const typed_entity_flavour_id<E>& id) const {
		return inferred.flavour_ids.get_entities_by_flavour_id(id);
	}
	
	uint32_t get_entities_count() const;
	
	template <class E>
	auto get_count_of() const {
		return significant.template get_pool<E>().size();
	}
	
	template <class E>
	auto get_max_count_of() const {
		return significant.template get_pool<E>().max_size();
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
	const augs::stepped_clock& get_clock() const;

	augs::delta get_fixed_delta() const;
	unsigned get_steps_per_second() const;

	template <template <class> class Predicate = always_true, class F>
	void for_each_entity(F&& callback);
	
	template <template <class> class Predicate = always_true, class F>
	void for_each_entity(F&& callback) const;

	bool empty() const;

	template <class E>
	auto* dereference_entity(const typed_entity_id<E> id) {
		return significant.get_pool<E>().find(id.raw);
	}

	template <class E>
	const auto* dereference_entity(const typed_entity_id<E> id) const {
		return significant.get_pool<E>().find(id.raw);
	}

	template <class F>
	decltype(auto) on_entity_meta(const entity_id id, F&& callback);

	template <class F>
	decltype(auto) on_entity_meta(const entity_id id, F&& callback) const;
};

inline entity_id cosmos_solvable::get_versioned(const unversioned_entity_id& id) const {
	return { 
		significant.on_pool(
			id.type_id, 
			[id](const auto& p){ return p.get_versioned(id.raw); }
		), 
		id.type_id 
	};
}

inline entity_id cosmos_solvable::find_versioned(const unversioned_entity_id& id) const {
	return { 
		significant.on_pool(
			id.type_id, 
			[id](const auto& p){ return p.find_versioned(id.raw); }
		), 
		id.type_id 
	};
}
