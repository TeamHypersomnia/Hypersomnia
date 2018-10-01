#pragma once
#include <map>

#include "augs/misc/timing/delta.h"

#if STATICALLY_ALLOCATE_ENTITIES
#include "game/organization/all_component_includes.h"
#else
#include "game/organization/all_components_declaration.h"
#endif

#include "game/cosmos/cosmos_solvable_inferred.h"
#include "game/cosmos/cosmos_solvable_significant.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_creation_error.h"

struct entity_creation_input {
	raw_entity_flavour_id flavour_id;
};

class cosmos_solvable {
	using guid_cache = std::map<entity_guid, entity_id>;

	static const cosmos_solvable zero;

	template <class A, class B>
	struct allocation_result {
		A key;
		B& object;
	};

	template <class E>
	auto allocate_new_entity(const entity_guid new_guid, entity_creation_input);

	template <class E, class... Args>
	auto detail_undo_free_entity(Args&&... args);

	entity_guid clear_guid(const entity_id);

	template <class C, class F>
	static decltype(auto) on_entity_meta_impl(
		C& self,
		const entity_id id,
		F callback	
	);

	template <class... MustHaveComponents, class S, class F>
	static void for_each_entity_impl(S& self, F callback);

	entity_guid get_guid(const entity_id&) const;

	guid_cache guid_to_id;

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
	auto allocate_entity_with_specific_guid(const entity_guid specific_guid, entity_creation_input);

	std::optional<cosmic_pool_undo_free_input> free_entity(entity_id);
	void undo_last_allocate_entity(entity_id);

	void destroy_all_caches();

	void increment_step();
	void clear();

	void set_steps_per_second(const unsigned steps_per_second);

	entity_id to_versioned(const unversioned_entity_id&) const;

	entity_id get_entity_id_by(const entity_guid&) const;

	template <template <class> class Guidized>
	Guidized<entity_guid> guidize(const Guidized<entity_guid>& id_source) const {
		return id_source;
	}

	template <template <class> class Deguidized>
	Deguidized<entity_id> deguidize(const Deguidized<entity_id>& id_source) const {
		return id_source;
	}

	template <template <class> class Guidized, class source_id_type>
	Guidized<entity_guid> guidize(const Guidized<source_id_type>& id_source) const;

	template <template <class> class Deguidized, class source_id_type>
	Deguidized<entity_id> deguidize(const Deguidized<source_id_type>& guid_source) const;

	template <class E>
	const auto& get_entities_by_flavour_id(const typed_entity_flavour_id<E>& id) const {
		return inferred.name.get_entities_by_flavour_id(id);
	}
	
	std::size_t get_entities_count() const;
	
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

	template <class... MustHaveComponents, class F>
	void for_each_entity(F&& callback);
	
	template <class... MustHaveComponents, class F>
	void for_each_entity(F&& callback) const;

	bool empty() const;

	void remap_guids();

	const auto& get_guid_to_id() const {
		return guid_to_id;
	}

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

	template <class F>
	decltype(auto) on_entity_meta(const entity_guid id, F&& callback);

	template <class F>
	decltype(auto) on_entity_meta(const entity_guid id, F&& callback) const;
};

inline entity_id cosmos_solvable::get_entity_id_by(const entity_guid& guid) const {
	if (const auto id = mapped_or_nullptr(guid_to_id, guid)) {
		return *id;
	}

	return {};
}

inline entity_id cosmos_solvable::to_versioned(const unversioned_entity_id& id) const {
	return { 
		significant.on_pool(
			id.type_id, 
			[id](const auto& p){ return p.to_versioned(id.raw); }
		), 
		id.type_id 
	};
}
