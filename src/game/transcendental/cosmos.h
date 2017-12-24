#pragma once
#include <map>
#include "3rdparty/sol2/sol/forward.hpp"

#include "augs/build_settings/platform_defines.h"

#include "augs/templates/exception_templates.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/misc/enum/enum_boolset.h"
#include "augs/misc/randomization_declaration.h"

#include "game/transcendental/cosmos_common.h"
#include "game/transcendental/cosmos_solvable.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/cosmos_solvable_access.h"
#include "game/transcendental/cosmos_solvable_inferred_access.h"

#include "game/assets/behaviour_tree.h"

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

template <class C>
auto subscript_handle_getter(C& cosm, const entity_guid guid) {
	return subscript_handle_getter(cosm, cosm.get_solvable().get_entity_id_by(guid));
}

class cosmos {
	void infer_all_entities();
	void reinfer_solvable();

	cosmos_common common;
	cosmos_solvable solvable;

public: 
	/* A detail only for performance benchmarks */
	mutable cosmic_profiler profiler;

	static const cosmos zero;

	cosmos() = default;
	explicit cosmos(const cosmic_pool_size_type reserved_entities);

	void reserve_storage_for_entities(const cosmic_pool_size_type s) {
		solvable.reserve_storage_for_entities(s);
	}

	entity_handle create_entity(const std::wstring& name);
	entity_handle create_entity(const std::string& name);
	entity_handle create_entity_with_specific_guid(
		const entity_guid specific_guid
	);

	entity_handle clone_entity(const entity_id);
	void delete_entity(const entity_id);

	template <class F>
	void for_each(
		const processing_subjects list_type, 
		F callback,
		augs::enum_boolset<subjects_iteration_flag> flags = {}
	) {
		if (flags.test(subjects_iteration_flag::POSSIBLE_ITERATOR_INVALIDATION)) {
			const auto targets = solvable.inferred.processing_lists.get(list_type);

			for (const auto& subject : targets) {
				operator()(subject, callback);
			}
		}
		else {
			for (const auto& subject : solvable.inferred.processing_lists.get(list_type)) {
				operator()(subject, callback);
			}
		}
	}

	template <class F>
	void for_each(const processing_subjects list_type, F callback) const {
		for (const auto& subject : solvable.inferred.processing_lists.get(list_type)) {
			operator()(subject, callback);
		}
	}

	void increment_step() {
		solvable.increment_step();
	}

	void infer_caches_for(const const_entity_handle);
	void destroy_caches_of(const const_entity_handle);
	void reinfer_caches_of(const const_entity_handle);
	
	template <class cache_type>
	void reinfer_cache(const entity_handle handle) {
		solvable.inferred.for_each([&](auto& sys) {
			using T = std::decay_t<decltype(sys)>;

			if constexpr(std::is_same_v<T, cache_type>) {
				sys.destroy_cache_of(handle);

				if (handle.is_inferred_state_activated()) {
					sys.infer_cache_for(handle);
				}
			}
		});
	}

	void reinfer_all_entities();

	/* 
		If exception is thrown during alteration,
		these metods will properly refresh inferred caches with what state was left.
	*/

	template <class F>
	void change_common_significant(F&& callback) {
		auto status = changer_callback_result::INVALID;

		auto refresh_when_done = augs::make_scope_guard([&]() {
			if (status != changer_callback_result::DONT_REFRESH) {
				reinfer_all_entities();
			}
		});

		status = callback(common.significant);
	}

	template <class F>
	void change_solvable_significant(F&& callback) {
		auto status = changer_callback_result::INVALID;

		auto refresh_when_done = augs::make_scope_guard([&]() {
			if (status != changer_callback_result::DONT_REFRESH) {
				reinfer_solvable();
			}
		});

		status = callback(solvable.significant);
	}

	void set(const cosmos_solvable_significant& signi) {
		change_solvable_significant([&](cosmos_solvable_significant& s){ 
			{
				auto scope = measure_scope(profiler.duplication);
				s = signi; 
			}

			return changer_callback_result::REFRESH; 
		});
	}

	si_scaling get_si() const;

	randomization get_rng_for(const entity_id) const;
	rng_seed_type get_rng_seed_for(const entity_id) const;
	
	std::wstring summary() const;

	auto& get_solvable(cosmos_solvable_access) {
		return solvable;
	}

	const auto& get_solvable(cosmos_solvable_access) const {
		return solvable;
	}

	const auto& get_solvable() const {
		return solvable;
	}

	auto& get_solvable_inferred(cosmos_solvable_inferred_access) {
		return solvable.inferred;
	}

	const auto& get_solvable_inferred(cosmos_solvable_inferred_access) const {
		return solvable.inferred;
	}

	const auto& get_solvable_inferred() const {
		return solvable.inferred;
	}

	const cosmos_common_significant& get_common_significant() const;
	const common_assets& get_common_assets() const;

	bool operator==(const cosmos&) const;
	bool operator!=(const cosmos&) const;

	template <class id_type>
	decltype(auto) operator[](const id_type id) {
		return subscript_handle_getter(*this, id);
	}

	template <class id_type>
	decltype(auto) operator[](const id_type id) const {
		return subscript_handle_getter(*this, id);
	}

	template <class F>
	decltype(auto) operator()(const entity_id subject, F callback) {
		callback(operator[](subject));
	}

	template <class F>
	decltype(auto) operator()(const entity_id subject, F callback) const {
		callback(operator[](subject));
	}
	
	void clear();
	bool empty() const;

	/*
		Shortcuts for heavily used functions for sanity
	*/

	auto get_entities_count() const {
		return solvable.get_entities_count();
	}

	auto get_total_seconds_passed(const double v) const {
		return solvable.get_total_seconds_passed(v);
	}

	auto get_total_seconds_passed() const {
		return solvable.get_total_seconds_passed();
	}

	auto get_total_steps_passed() const {
		return solvable.get_total_steps_passed();
	}

	auto get_timestamp() const {
		return solvable.get_timestamp();
	}

	auto get_fixed_delta() const {
		return solvable.get_fixed_delta();
	}

	auto make_versioned(const unversioned_entity_id id) const {
		return solvable.make_versioned(id);
	}

};

inline si_scaling cosmos::get_si() const {
	return get_common_significant().si;
}

inline const cosmos_common_significant& cosmos::get_common_significant() const {
	return common.significant;
}

inline const common_assets& cosmos::get_common_assets() const {
	return get_common_significant().assets;
}

#if READWRITE_OVERLOAD_TRAITS_INCLUDED || LUA_READWRITE_OVERLOAD_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

namespace augs {
	template <class Archive>
	void write_object_bytes(Archive& ar, const cosmos& cosm);
	
	template <class Archive>
	void read_object_bytes(Archive& ar, cosmos& cosm);

	void write_object_lua(sol::table ar, const cosmos& cosm);
	void read_object_lua(sol::table ar, cosmos& cosm);
}
