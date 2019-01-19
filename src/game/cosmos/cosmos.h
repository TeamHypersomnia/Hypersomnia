#pragma once
#include "augs/build_settings/compiler_defines.h"
#include "augs/misc/randomization_declaration.h"

#include "game/organization/all_entity_types_declaration.h"

#include "game/cosmos/cosmos_common.h"
#include "game/cosmos/cosmic_profiler.h"
#include "game/cosmos/cosmos_common_significant_access.h"
#include "game/cosmos/private_cosmos_solvable.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/handle_getters_declaration.h"

#include "game/enums/processing_subjects.h"

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

	/* 
		If exception is thrown during alteration,
		these metods will properly refresh inferred caches with what state was left.
	*/

	template <class F>
	void change_common_significant(F&& callback);

	void set(const cosmos_solvable_significant& signi);

	si_scaling get_si() const {
		return get_common_significant().si;
	}

	randomization get_rng_for(const entity_id) const;
	randomization get_nontemporal_rng_for(const entity_id) const;
	rng_seed_type get_rng_seed_for(const entity_id) const;
	
	std::string summary() const;

	const cosmos_common_significant& get_common_significant() const {
		return common.significant;
	}

	cosmos_common_significant& get_common_significant(cosmos_common_significant_access) {
		return common.significant;
	}

	const cosmos_common_significant& get_common_significant(cosmos_common_significant_access) const {
		return common.significant;
	}

	const common_assets& get_common_assets() const {
		return get_common_significant().assets;
	}

	/* Shortcuts */

	template <class entity_type>
	const auto& get_flavours() const {
		return get_common_significant().flavours.get_for<entity_type>();
	}

	template <class entity_type>
	auto& get_flavours(cosmos_common_significant_access key) {
		return get_common_significant(key).flavours.get_for<entity_type>();
	}

	template <class entity_type>
	const auto& get_flavours(cosmos_common_significant_access key) const {
		return get_common_significant(key).flavours.get_for<entity_type>();
	}

	template <class entity_type>
	const auto& get_flavour(const typed_entity_flavour_id<entity_type> id) const {
		return get_flavours<entity_type>().get(id.raw);
	}

	template <class entity_type>
	auto& get_flavour(cosmos_common_significant_access key, const typed_entity_flavour_id<entity_type> id) {
		return get_flavours<entity_type>(key).get(id.raw);
	}

	template <class entity_type>
	const auto& get_flavour(cosmos_common_significant_access key, const typed_entity_flavour_id<entity_type> id) const {
		return get_flavours<entity_type>(key).get(id.raw);
	}

	template <class entity_type>
	const auto* find_flavour(const typed_entity_flavour_id<entity_type> id) const {
		return get_flavours<entity_type>().find(id.raw);
	}

	template <class... Args>
	decltype(auto) on_flavour(Args&&... args) const {
		return get_common_significant().on_flavour(std::forward<Args>(args)...);
	}

	template <class E, class F>
	void for_each_id_and_flavour(F&& callback) const {
		for_each_id_and_object(
			get_flavours<E>(), 
			[&callback](const raw_entity_flavour_id& id, const entity_flavour<E>& flavour) {
				callback(typed_entity_flavour_id<E>(id), flavour);
			}
		);
	}

	template <class E>
	auto get_flavours_count() const {
		return get_flavours<E>().size();
	}

	template <class T>
	const std::string* find_flavour_name(const T& id) const {
		if (id.is_set()) {
			return on_flavour(id, [](const auto& f){ return std::addressof(f.get_name()); });
		}

		return nullptr;
	}

	template <template <class> class Predicate = always_true, class F>
	void for_each_entity(F&& callback);

	template <template <class> class Predicate = always_true, class F>
	void for_each_entity(F&& callback) const;

	template <class... MustHaveComponents, class F>
	void for_each_having(F&& callback);

	template <class... MustHaveComponents, class F>
	void for_each_having(F&& callback) const;

	template <class... MustHaveInvariants, class F>
	void for_each_flavour_having(F&& callback) const;

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

	template <class id_type, class F>
	decltype(auto) operator()(const id_type& subject, F callback) {
		callback(operator[](subject));
	}

	template <class id_type, class F>
	decltype(auto) operator()(const id_type& subject, F callback) const {
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

	cosmos_solvable_inferred& get_solvable_inferred(cosmos_solvable_inferred_access k) {
		return solvable.get_solvable_inferred(k);
	}

	const cosmos_solvable_inferred& get_solvable_inferred(cosmos_solvable_inferred_access k) const {
		return solvable.get_solvable_inferred(k);
	}

	const cosmos_solvable_inferred& get_solvable_inferred() const {
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

	decltype(auto) get_clock() const {
		return get_solvable().get_clock();
	}

	auto get_timestamp() const {
		return get_solvable().get_timestamp();
	}

	auto get_fixed_delta() const {
		return get_solvable().get_fixed_delta();
	}

	auto get_versioned(const unversioned_entity_id& id) const {
		return get_solvable().get_versioned(id);
	}
	
	auto find_versioned(const unversioned_entity_id& id) const {
		return get_solvable().find_versioned(id);
	}

	auto& get_logical_assets(cosmos_common_significant_access k) {
		return get_common_significant(k).logical_assets;
	}

	const auto& get_logical_assets() const {
		return get_common_significant().logical_assets;
	}

	const auto& get_logical_assets(cosmos_common_significant_access k) const {
		return get_common_significant(k).logical_assets;
	}

	auto& get_global_solvable() {
		return solvable.get_global_solvable();
	}

	const auto& get_global_solvable() const {
		return solvable.get_global_solvable();
	}

	template <class M>
	auto measure_raycasts(M& measurement) const {
		const auto& num_ray_casts = get_solvable_inferred().physics.ray_cast_counter;
		const auto before = num_ray_casts;

		return augs::scope_guard([&num_ray_casts, &measurement, before](){
			const auto surplus = num_ray_casts - before;
			measurement.measure(surplus);
		});
	}

	template <class C>
	void erase_dead(C& container) const {
		erase_if(container, [&](const auto& entry) {
			return operator[](entry).dead();
		});
	}

	template <class C>
	void clear_dead(C& entry) const {
		if (operator[](entry).dead()) {
			entry = {};
		}
	}

	void reinfer_everything();

	void set_fixed_delta(const augs::delta& dt);

	void assign_solvable(const cosmos& b);

	template <class T>
	T calculate_solvable_signi_hash() const;
};