#pragma once
#include <map>

#include "augs/build_settings/platform_defines.h"

#include "augs/templates/exception_templates.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/misc/enum/enum_boolset.h"
#include "augs/misc/randomization_declaration.h"

#include "game/organization/all_entity_types_declaration.h"

#include "game/transcendental/cosmos_common.h"
#include "game/transcendental/cosmos_common_significant_access.h"

#include "game/transcendental/private_cosmos_solvable.h"
#include "game/transcendental/specific_entity_handle.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmic_functions.h"

#include "game/enums/processing_subjects.h"

#include "game/assets/behaviour_tree.h"

struct cosmos_loading_error : error_with_typesafe_sprintf {
	using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
};

template <class C>
auto subscript_handle_getter(C& cosm, const entity_id id) {
	static constexpr bool is_const = std::is_const_v<C>;

	const auto ptr = cosm.get_solvable({}).on_entity_meta(id, [&](auto* const agg) {
		return reinterpret_cast<maybe_const_ptr_t<is_const, void>>(agg);	
	});

	return basic_entity_handle<is_const>{ ptr, cosm, id };
}

template <class C, class E>
auto subscript_handle_getter(C& cosm, const typed_entity_id<E> id) {
	const auto ptr = cosm.get_solvable({}).dereference_entity(id);

	return basic_typed_entity_handle<std::is_const_v<C>, E>{ 
		cosm, 
		{ ptr, id } 
	};
}

template <class C>
auto subscript_handle_getter(C& cosm, const child_entity_id id) {
	return subscript_handle_getter(cosm, entity_id(id));
}

template <class C>
auto subscript_handle_getter(C& cosm, const unversioned_entity_id id) {
	return subscript_handle_getter(cosm, cosm.to_versioned(id));
}

template <class C>
auto subscript_handle_getter(C& cosm, const entity_guid guid) {
	return subscript_handle_getter(cosm, cosm.get_solvable().get_entity_id_by(guid));
}

class cosmos {
	template <class C, class F>
	static void for_each_in_impl(C& self, const processing_subjects f, F callback) {
		for (const auto subject : self.get_solvable_inferred().processing.get(f)) {
			callback(self[subject]);
		}
	}

	cosmos_common common;
	private_cosmos_solvable solvable;

public: 
	/* A detail only for performance benchmarks */
	mutable cosmic_profiler profiler;

	static const cosmos zero;

	cosmos() = default;
	explicit cosmos(const cosmic_pool_size_type reserved_entities);

	cosmos(cosmos&&) = delete;
	cosmos& operator=(cosmos&&) = delete;

	cosmos(const cosmos&) = delete;
	cosmos& operator=(const cosmos&) = delete;

	/* 
		If exception is thrown during alteration,
		these metods will properly refresh inferred caches with what state was left.
	*/

	template <class F>
	void change_common_significant(F&& callback) {
		auto status = changer_callback_result::INVALID;
		auto& self = *this;

		auto refresh_when_done = augs::scope_guard([&]() {
			if (status != changer_callback_result::DONT_REFRESH) {
				/*	
					Always first reinfer the common,
				   	only later the entities, as they might use the common inferred during their own reinference. 
				*/
				common.reinfer();
				cosmic::reinfer_all_entities(self);
			}
		});

		status = callback(common.significant);
	}

	void set(const cosmos_solvable_significant& signi) {
		cosmic::change_solvable_significant(*this, [&](cosmos_solvable_significant& s){ 
			{
				auto scope = measure_scope(profiler.duplication);
				s = signi; 
			}

			return changer_callback_result::REFRESH; 
		});
	}

	si_scaling get_si() const;

	randomization get_rng_for(const entity_id) const;
	fast_randomization get_fast_rng_for(const entity_id) const;
	rng_seed_type get_rng_seed_for(const entity_id) const;
	
	std::string summary() const;

	const cosmos_common_significant& get_common_significant() const;

	auto& get_common_significant(cosmos_common_significant_access) {
		return common.significant;
	}

	const auto& get_common_significant(cosmos_common_significant_access) const {
		return common.significant;
	}

	const common_assets& get_common_assets() const;

	/* Shortcuts */

	template <class entity_type>
	const auto& get_flavours() const {
		return get_common_significant().get_flavours<entity_type>();
	}

	template <class entity_type>
	const auto& get_flavour(const typed_entity_flavour_id<entity_type> id) const {
		return get_flavours<entity_type>().get_flavour(id);
	}

	template <class entity_type>
	const auto* find_flavour(const typed_entity_flavour_id<entity_type> id) const {
		return get_flavours<entity_type>().find_flavour(id);
	}

	template <class... Args>
	decltype(auto) on_flavour(Args&&... args) const {
		return get_common_significant().on_flavour(std::forward<Args>(args)...);
	}

	template <class E, class F>
	void for_each_id_and_flavour(F&& callback) const {
		get_flavours<E>().for_each(std::forward<F>(callback));
	}

	template <class E>
	auto get_flavours_count() const {
		return get_flavours<E>().count();
	}

	template <class T>
	const std::string* find_flavour_name(const T& id) const {
		if (id.is_set()) {
			return on_flavour(id, [](const auto& f){ return std::addressof(f.get_name()); });
		}

		return nullptr;
	}

	template <class... Constraints, class F>
	void for_each_having(F&& callback) {
		cosmic::for_each_entity<Constraints...>(*this, std::forward<F>(callback));
	}

	template <class... Constraints, class F>
	void for_each_having(F&& callback) const {
		cosmic::for_each_entity<Constraints...>(*this, std::forward<F>(callback));
	}

	template <class F>
	void for_each_in(const processing_subjects f, F&& callback) {
		for_each_in_impl(*this, f, std::forward<F>(callback));
	}

	template <class F>
	void for_each_in(const processing_subjects f, F&& callback) const {
		for_each_in_impl(*this, f, std::forward<F>(callback));
	}

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
	
	bool empty() const;

	/*
		Shortcuts for heavily used functions for sanity
	*/

	cosmos_solvable& get_solvable(cosmos_solvable_access k) {
		return solvable.get_solvable(k);
	}

	const cosmos_solvable& get_solvable(cosmos_solvable_access k) const {
		return solvable.get_solvable(k);
	}

	const cosmos_solvable& get_solvable() const {
		return solvable.get_solvable();
	}

	auto& get_solvable_inferred(cosmos_solvable_inferred_access k) {
		return solvable.get_solvable_inferred(k);
	}

	const auto& get_solvable_inferred(cosmos_solvable_inferred_access k) const {
		return solvable.get_solvable_inferred(k);
	}

	const auto& get_solvable_inferred() const {
		return solvable.get_solvable_inferred();
	}

	auto get_entities_count() const {
		return get_solvable().get_entities_count();
	}

	auto get_total_seconds_passed(const double v) const {
		return get_solvable().get_total_seconds_passed(v);
	}

	auto get_total_seconds_passed() const {
		return get_solvable().get_total_seconds_passed();
	}

	auto get_total_steps_passed() const {
		return get_solvable().get_total_steps_passed();
	}

	auto get_timestamp() const {
		return get_solvable().get_timestamp();
	}

	auto get_fixed_delta() const {
		return get_solvable().get_fixed_delta();
	}

	auto to_versioned(const unversioned_entity_id id) const {
		return get_solvable().to_versioned(id);
	}
	
	const auto& get_logical_assets() const {
		return get_common_significant().logical_assets;
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

	template <class Archive>
	void write_object_lua(Archive, const cosmos& cosm);

	template <class Archive>
	void read_object_lua(Archive, cosmos& cosm);
}
