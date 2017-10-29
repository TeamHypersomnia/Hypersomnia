#pragma once
#include <map>
#include <sol2/sol/forward.hpp>
#include "augs/build_settings/platform_defines.h"

#include "augs/templates/exception_templates.h"

#include "augs/misc/randomization.h"
#include "augs/readwrite/streams.h"
#include "augs/misc/timing/delta.h"
#include "augs/misc/enum/enum_boolset.h"
#include "augs/templates/subscript_handle_getters_mixin.h"
#include "augs/templates/introspection_utils/rewrite_members.h"
#include "augs/misc/enum/enum_associative_array.h"

#include "augs/entity_system/operations_on_all_components_mixin.h"
#include "augs/entity_system/storage_for_systems.h"

#include "game/build_settings.h"
#include "game/assets/ids/behaviour_tree_id.h"
#include "game/assets/behaviour_tree.h"

#include "game/organization/all_fundamental_component_includes.h"
#include "augs/entity_system/component_aggregate.h"

#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/cosmic_profiler.h"

#if STATICALLY_ALLOCATE_ENTITIES_NUM
#include "game/organization/all_component_includes.h"
#else
#include "game/organization/all_components_declaration.h"
#endif

#include "game/organization/all_messages_declaration.h"
#include "game/organization/all_inferred_systems.h"
#include "game/transcendental/cosmos_significant_state.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/assets/behaviour_tree.h"

using rng_seed_type = unsigned;

enum class subjects_iteration_flag {
	POSSIBLE_ITERATOR_INVALIDATION,

	COUNT
};

struct cosmos_loading_error : error_with_typesafe_sprintf {
	using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
};

template <class C>
auto subscript_handle_getter(C& cosm, const entity_id id) {
	return basic_entity_handle<std::is_const_v<C>>{ cosm, id };
}

template <class C>
auto subscript_handle_getter(C& cosm, const child_entity_id id) {
	return subscript_handle_getter(cosm, entity_id(id));
}

template <class C>
auto subscript_handle_getter(C& cosm, const unversioned_entity_id id) {
	return basic_entity_handle<std::is_const_v<C>>{ cosm, cosm.make_versioned(id) };
}

#if COSMOS_TRACKS_GUIDS
template <class C>
auto subscript_handle_getter(C& cosm, const entity_guid guid) {
	return subscript_handle_getter(cosm, cosm.get_entity_id_by(guid));
}
#endif

class EMPTY_BASES cosmos : private cosmos_base,
	public augs::subscript_handle_getters_mixin<cosmos>
{
	friend augs::subscript_handle_getters_mixin<cosmos>;
	friend cosmos_base;

	/* State of the cosmos begins here ***************************/
#if COSMOS_TRACKS_GUIDS
	std::map<entity_guid, entity_id> guid_to_id;
#endif

public:
	static const cosmos empty;

	cosmos_significant_state significant;
	all_inferential_systems inferential;

	augs::stream reserved_memory_for_serialization;
	mutable cosmic_profiler profiler;
	
	/* State of the cosmos ends here *****************************/

	cosmos() = default;
	explicit cosmos(const cosmic_pool_size_type reserved_entities);
	cosmos& operator=(const cosmos_significant_state&);

	void reserve_storage_for_entities(const cosmic_pool_size_type);

	entity_handle create_entity(const std::wstring& name);
	entity_handle create_entity(const std::string& name);
	entity_handle clone_entity(const entity_id);
	void delete_entity(const entity_id);

	template <class Pre, class Post, class PostCleanup>
	void advance(
		const logic_step_input input,
		Pre pre_solve,
		Post post_solve,
		PostCleanup post_cleanup
	) {
		thread_local data_living_one_step queues;
		logic_step step(*this, input, queues);

		pre_solve(step);
		advance_and_queue_destructions(step);
		post_solve(const_logic_step(step));
		perform_deletions(step);
		post_cleanup(const_logic_step(step));

		queues.clear();
	}

	template <class F>
	void for_each(
		const processing_subjects list_type, 
		F callback,
		augs::enum_boolset<subjects_iteration_flag> flags = {}
	) {
		if (flags.test(subjects_iteration_flag::POSSIBLE_ITERATOR_INVALIDATION)) {
			const auto targets = inferential.processing_lists.get(list_type);

			for (const auto& subject : targets) {
				operator()(subject, callback);
			}
		}
		else {
			for (const auto& subject : inferential.processing_lists.get(list_type)) {
				operator()(subject, callback);
			}
		}
	}

	template <class F>
	void for_each(const processing_subjects list_type, F callback) const {
		for (const auto& subject : inferential.processing_lists.get(list_type)) {
			operator()(subject, callback);
		}
	}

	template <class D>
	void for_each_entity_id(D pred) {
		get_entity_pool().for_each_id(pred);
	}

	template <class D>
	void for_each_entity_id(D pred) const {
		get_entity_pool().for_each_id(pred);
	}

	void complete_reinference();
	void complete_reinference(const const_entity_handle);

	void create_inferred_state_for(const const_entity_handle);
	void destroy_inferred_state_of(const const_entity_handle);
	
	void refresh_for_new_significant_state();
#if COSMOS_TRACKS_GUIDS
	void remap_guids();
#endif
	void clear();

	template <class System>
	void partial_reinference(const entity_handle handle) {
		inferential.for_each([&](auto& sys) {
			using T = std::decay_t<decltype(sys)>;

			if constexpr(std::is_same_v<T, System>) {
				sys.destroy_inferred_state_of(handle);

				if (handle.is_inferred_state_activated()) {
					sys.create_inferred_state_for(handle);
				}
			}
		});
	}

	entity_id make_versioned(const unversioned_entity_id) const;

#if COSMOS_TRACKS_GUIDS
	entity_id get_entity_id_by(const entity_guid) const;
	bool entity_exists_by(const entity_guid) const;

	template <template <class> class Guidized, class source_id_type>
	Guidized<entity_guid> guidize(const Guidized<source_id_type>& id_source) const {
		return rewrite_members_and_transform_templated_type_into<entity_guid>(
			id_source,
			[this](auto& guid_member, const auto& id_member) {
				const auto handle = operator[](id_member);
			
				if (handle.alive()) {
					guid_member = operator[](id_member).get_guid();
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
#endif

	si_scaling get_si() const;

	randomization get_rng_for(const entity_id) const;
	rng_seed_type get_rng_seed_for(const entity_id) const;

	std::size_t get_count_of(const processing_subjects list_type) const {
		return inferential.processing_lists.get(list_type).size();
	}
	
	std::unordered_set<entity_id> get_entities_by_name(const entity_name_type&) const;
	std::unordered_set<entity_id> get_entities_by_name_id(const entity_name_id&) const;
	
	entity_handle get_entity_by_name(const entity_name_type&);
	const_entity_handle get_entity_by_name(const entity_name_type&) const;
	
	std::size_t get_entities_count() const;
	std::size_t get_maximum_entities() const;
	std::wstring summary() const;

	double get_total_seconds_passed(const double view_interpolation_ratio) const;
	double get_total_seconds_passed() const;
	decltype(augs::stepped_timestamp::step) get_total_steps_passed() const;

	augs::stepped_timestamp get_timestamp() const;

	augs::delta get_fixed_delta() const;
	void set_steps_per_second(const unsigned steps_per_second);
	unsigned get_steps_per_second() const;

	cosmos_common_state& get_common_state();
	const cosmos_common_state& get_common_state() const;

	common_assets& get_common_assets();
	const common_assets& get_common_assets() const;

	auto& get_entity_pool() {
		return significant.entity_pool;
	}

	const auto& get_entity_pool() const {
		return significant.entity_pool;
	}

	template<class T>
	auto& get_component_pool() {
		return std::get<cosmic_object_pool<T>>(significant.component_pools);
	}

	template<class T>
	const auto& get_component_pool() const {
		return std::get<cosmic_object_pool<T>>(significant.component_pools);
	}

	/* TODO: Make comparisons somehow work with debug name pointers */
#if !(DEBUG_TRACK_ENTITY_NAME && STATICALLY_ALLOCATE_ENTITIES_NUM)
	bool operator==(const cosmos&) const;
	bool operator!=(const cosmos&) const;
#endif

	template <class F>
	decltype(auto) operator()(const entity_id subject, F callback) {
		callback(operator[](subject));
	}

	template <class F>
	decltype(auto) operator()(const entity_id subject, F callback) const {
		callback(operator[](subject));
	}

private:
#if COSMOS_TRACKS_GUIDS
	friend class cosmic_delta;

	template <class T>
	friend void transform_component_guids_to_ids_in_place(T&, const cosmos&);

	void assign_next_guid(const entity_handle);
	void clear_guid(const entity_handle);
	entity_guid get_guid(const const_entity_handle) const;

	entity_handle create_entity_with_specific_guid(
		const entity_guid specific_guid
	);
#endif

	void advance_and_queue_destructions(const logic_step step_state);
	void perform_deletions(const logic_step);

	void destroy_inferred_state_completely();
	void create_inferred_state_completely();

	entity_handle allocate_new_entity();
};

inline si_scaling cosmos::get_si() const {
	return significant.meta.global.si;
}

#if COSMOS_TRACKS_GUIDS
inline entity_id cosmos::get_entity_id_by(const entity_guid guid) const {
	return guid_to_id.at(guid);
}

inline bool cosmos::entity_exists_by(const entity_guid guid) const {
	return guid_to_id.find(guid) != guid_to_id.end();
}

inline entity_guid cosmos::get_guid(const const_entity_handle handle) const {
	return handle.get_guid();
}
#endif

inline cosmos_common_state& cosmos::get_common_state() {
	return significant.meta.global;
}

inline const cosmos_common_state& cosmos::get_common_state() const {
	return significant.meta.global;
}

inline common_assets& cosmos::get_common_assets() {
	return get_common_state().assets;
}

inline const common_assets& cosmos::get_common_assets() const {
	return get_common_state().assets;
}

inline std::unordered_set<entity_id> cosmos::get_entities_by_name(const entity_name_type& name) const {
	return inferential.name.get_entities_by_name(name);
}

inline std::unordered_set<entity_id> cosmos::get_entities_by_name_id(const entity_name_id& id) const {
	return inferential.name.get_entities_by_name_id(id);
}

inline entity_handle cosmos::get_entity_by_name(const entity_name_type& name) {
	const auto entities = get_entities_by_name(name);
	ensure(entities.size() <= 1);

	if (entities.empty()) {
		return operator[](entity_id());		
	}
	else {
		return operator[](*entities.begin());
	}
}

inline const_entity_handle cosmos::get_entity_by_name(const entity_name_type& name) const {
	const auto entities = get_entities_by_name(name);
	ensure(entities.size() <= 1);

	if (entities.empty()) {
		return operator[](entity_id());
	}
	else {
		return operator[](*entities.begin());
	}
}

inline entity_id cosmos::make_versioned(const unversioned_entity_id id) const {
	return get_entity_pool().make_versioned(id);
}

inline randomization cosmos::get_rng_for(const entity_id id) const {
	return{ static_cast<std::size_t>(get_rng_seed_for(id)) };
}

inline std::size_t cosmos::get_entities_count() const {
	return significant.entity_pool.size();
}

inline std::size_t cosmos::get_maximum_entities() const {
	return significant.entity_pool.capacity();
}

#if READWRITE_OVERLOAD_TRAITS_INCLUDED || LUA_READWRITE_OVERLOAD_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

namespace augs {
	void write_object_bytes(augs::stream& ar, const cosmos& cosm);
	void read_object_bytes(augs::stream& ar, cosmos& cosm);

	void write_object_lua(sol::table ar, const cosmos& cosm);
	void read_object_lua(sol::table ar, cosmos& cosm);
}