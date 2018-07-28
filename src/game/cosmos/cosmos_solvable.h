#pragma once
#include <map>

#include "augs/enums/callback_result.h"
#include "augs/templates/traits/is_optional.h"
#include "augs/misc/timing/delta.h"
#include "augs/misc/randomization_declaration.h"

#include "augs/templates/type_templates.h"
#include "augs/templates/introspection_utils/rewrite_members.h"
#include "augs/templates/for_each_std_get.h"

#include "augs/entity_system/storage_for_systems.h"

#include "game/assets/ids/behaviour_tree_id.h"

#include "game/cosmos/cosmic_profiler.h"

#if STATICALLY_ALLOCATE_ENTITIES
#include "game/organization/all_component_includes.h"
#else
#include "game/organization/all_components_declaration.h"
#endif

#include "game/organization/all_messages_declaration.h"
#include "game/cosmos/cosmos_solvable_inferred.h"
#include "game/cosmos/cosmos_solvable_significant.h"
#include "game/cosmos/specific_entity_handle_declaration.h"
#include "game/cosmos/entity_id.h"

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

		if (pool.full_capacity()) {
			throw std::runtime_error("Entities should be controllably reserved to avoid invalidation of entity_handles.");
		}

		const auto result = pool.allocate(new_guid, std::forward<Args>(args)..., get_timestamp());

		allocation_result<typed_entity_id<E>, decltype(result.object)> output {
			typed_entity_id<E>(result.key), result.object
		};

		return output;
	}

	template <class E, class... Args>
	auto detail_undo_free_entity(Args&&... args) {
		auto& pool = significant.get_pool<E>();
		const auto result = pool.undo_free(std::forward<Args>(args)...);

		allocation_result<typed_entity_id<E>, decltype(result.object)> output {
			typed_entity_id<E>(result.key), result.object
		};

		return output;
	}

	entity_guid clear_guid(const entity_id);

	template <class C, class F>
	static decltype(auto) on_entity_meta_impl(
		C& self,
		const entity_id id,
		F callback	
	) {
		using meta_ptr = maybe_const_ptr_t<std::is_const_v<C>, entity_solvable_meta>;

		if (id.type_id.is_set()) {
			return self.significant.entity_pools.visit(
				id.type_id,
				[&](auto& pool) -> decltype(auto) {	
					return callback(static_cast<meta_ptr>(pool.find(id.raw)));
				}
			);
		}
		else {
			return callback(static_cast<meta_ptr>(nullptr));
		}
	}

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

	template <class E, class... Args>
	auto allocate_next_entity(Args&&... args) {
		const auto next_guid = significant.clock.next_entity_guid.value++;
		return allocate_entity_with_specific_guid<E>(next_guid, std::forward<Args>(args)...);
	}

	template <class E, class... Args>
	auto undo_free_entity(Args&&... undo_free_args) {
		const auto result = detail_undo_free_entity<E>(std::forward<Args>(undo_free_args)...);
		const auto it = guid_to_id.emplace(result.object.guid, result.key);
		ensure(it.second);
		return result;
	}

	template <class E, class... Args>
	auto allocate_entity_with_specific_guid(const entity_guid specific_guid, Args&&... args) {
		const auto result = allocate_new_entity<E>(specific_guid, std::forward<Args>(args)...);
		guid_to_id[specific_guid] = result.key;
		return result;
	}

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
	decltype(auto) on_entity_meta(const entity_id id, F&& callback) {
		return on_entity_meta_impl(*this, id, std::forward<F>(callback));
	}	

	template <class F>
	decltype(auto) on_entity_meta(const entity_id id, F&& callback) const {
		return on_entity_meta_impl(*this, id, std::forward<F>(callback));
	}	

	template <class F>
	decltype(auto) on_entity_meta(const entity_guid id, F&& callback) {
		return on_entity_meta_impl(*this, get_entity_id_by(id), std::forward<F>(callback));
	}	

	template <class F>
	decltype(auto) on_entity_meta(const entity_guid id, F&& callback) const {
		return on_entity_meta_impl(*this, get_entity_id_by(id), std::forward<F>(callback));
	}	
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

inline entity_guid cosmos_solvable::get_guid(const entity_id& id) const {
	return on_entity_meta(
		id, 
		[](const entity_solvable_meta* const e) { 
			if (e) {
				return e->guid;
			}	 

			return entity_guid();
		}
	);
}

