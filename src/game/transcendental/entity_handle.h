#pragma once
#include <ostream>
#include "3rdparty/sol2/sol/forward.hpp"

#include "augs/build_settings/platform_defines.h"
#include "augs/templates/get_by_dynamic_id.h"

#include "augs/templates/maybe_const.h"
#include "augs/templates/traits/component_traits.h"
#include "augs/templates/type_matching_and_indexing.h"

#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/cosmos_solvable_access.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_solvable.h"
#include "game/transcendental/specific_entity_handle.h"

#include "game/organization/all_components_declaration.h"

#include "game/detail/entity_handle_mixins/all_handle_mixins.h"

#include "game/transcendental/step_declaration.h"
#include "game/components/flags_component.h"

class cosmos;

template <class, class>
class component_synchronizer;

template <bool is_const>
class basic_entity_handle :
	public misc_mixin<basic_entity_handle<is_const>>,
	public inventory_mixin<basic_entity_handle<is_const>>,
	public physics_mixin<basic_entity_handle<is_const>>,
	public relations_mixin<basic_entity_handle<is_const>>,
	public spatial_properties_mixin<basic_entity_handle<is_const>>
{
	template <bool, class, template <class> class>
	friend class specific_entity_handle;

	using owner_reference = maybe_const_ref_t<is_const, cosmos>;
	using entity_ptr = maybe_const_ptr_t<is_const, void>;

	using this_handle_type = basic_entity_handle<is_const>;
	using misc_base = misc_mixin<this_handle_type>;

	const entity_ptr ptr;
	owner_reference owner;
	entity_id raw_id;

	template <typename T>
	static void check_component_type() {
		static_assert(is_one_of_list_v<T, component_list_t<type_list>>, "Unknown component type!");
	}

	template <typename T>
	static void check_invariant_type() {
		static_assert(is_one_of_list_v<T, invariant_list_t<type_list>>, "Unknown invariant type!");
	}

	auto& pool_provider() const {
		return owner.get_solvable({});
	}

	basic_entity_handle(
		const entity_ptr ptr,
		owner_reference owner,
		const entity_id raw_id
	) :
		ptr(ptr),
		owner(owner),
		raw_id(raw_id)
	{}

	template <class T>
	auto* find_invariant_ptr() const {
		return dispatch([](const auto typed_handle) { 
			return typed_handle.get_flavour().template find<T>(); 
		});
	}

	template <class T>
	auto* find_component_ptr() const {
		return dispatch([](const auto typed_handle) { 
			return typed_handle.template find_component_ptr<T>(); 
		});
	}

	static auto dereference(
		owner_reference owner, 
		const entity_id raw_id
	) {
		return owner.get_solvable({}).on_entity_meta(raw_id, [&](auto* const agg) {
			return reinterpret_cast<entity_ptr>(agg);	
		});
	}

public:
	using const_type = basic_entity_handle<!is_const>;
	using misc_base::get_raw_flavour_id;
	friend const_type;

	static this_handle_type dead_handle(owner_reference owner) {
		return { nullptr, owner, {} };
	}

	basic_entity_handle(
		owner_reference owner, 
		const entity_id raw_id
	) : basic_entity_handle(
		dereference(owner, raw_id),
		owner, 
		raw_id 
	) {
	}

	const auto& get_meta() const {
		return *reinterpret_cast<const entity_solvable_meta*>(ptr);
	};

	auto& get(cosmos_solvable_access) const {
		return *ptr;
	}

	const auto& get() const {
		return *ptr;
	}

	auto get_id() const {
		return raw_id;
	}

	auto get_type_id() const {
		return raw_id.type_id;
	}

	bool alive() const {
		return ptr != nullptr;
	}

	bool dead() const {
		return !alive();
	}

	auto& get_cosmos() const {
		return owner;
	}

	bool operator==(const this_handle_type& h) const {
		return this->get_id() == h.get_id();
	}

	bool operator==(const basic_entity_handle<!is_const>& h) const {
		return this->get_id() == h.get_id();
	}

	bool operator==(const entity_id id) const {
		return raw_id == id;
	}

	bool operator==(const entity_guid id) const {
		return this->get_guid() == id;
	}

	bool operator!=(const entity_id id) const {
		return !operator==(id);
	}

	operator entity_guid() const {
		return this->get_guid();
	}

	operator entity_id() const {
		return raw_id;
	}

	operator child_entity_id() const {
		return raw_id;
	}

	operator unversioned_entity_id() const {
		return raw_id;
	}

	/* Enable non-const to const handle conversion */
	template <bool C = !is_const, class = std::enable_if_t<C>>
	operator basic_entity_handle<!is_const>() const {
		return basic_entity_handle<!is_const>(ptr, owner, raw_id);
	}

	explicit operator bool() const {
		return alive();
	}

	template <class E>
	decltype(auto) get_specific() const {
		using handle_type = basic_typed_entity_handle<is_const, E>;
		using specific_ptr_type = maybe_const_ptr_t<is_const, entity_solvable<E>>;

		auto& specific_ref = *reinterpret_cast<specific_ptr_type>(ptr);
		return handle_type(specific_ref, owner, get_id());
	}

	template <class List, class F>
	void conditional_dispatch(F&& callback) const {
		ensure(alive());

		for_each_through_std_get(List(), [&](auto t) { 
			using E = decltype(t);

			if (raw_id.type_id == entity_type_id::of<E>()) {
				callback(this->get_specific<E>());
			}
		});
	}

	template <class... List, class F>
	void dispatch_on_having(F&& callback) const {
		conditional_dispatch<entity_types_having_all_of<List...>>(std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) dispatch(F&& callback) const {
		ensure(alive());

		return get_by_dynamic_id(
			all_entity_types(),
			raw_id.type_id,
			[&](auto t) -> decltype(auto) {
				using E = decltype(t);
				return callback(this->get_specific<E>());
			}
		);
	}

	template <class T>
	bool has() const {
		return dispatch([](const auto typed_handle) { 
			return typed_handle.template has<T>(); 
		});
	}

	template<class T>
	decltype(auto) find() const {
		if constexpr(is_invariant_v<T>) {
			return find_invariant_ptr<T>();
		}
		else {
			if constexpr(is_synchronized_v<T>) {
				return component_synchronizer<this_handle_type, T>(find_component_ptr<T>(), *this);
			}
			else {
				return find_component_ptr<T>();
			}
		}
	}

	template<class T>
	decltype(auto) get() const {
		if constexpr(is_invariant_v<T>) {
			const auto p = find_invariant_ptr<T>();
			ensure(p != nullptr);
			return *p;
		}
		else {
			if constexpr(is_synchronized_v<T>) {
				return component_synchronizer<this_handle_type, T>(find_component_ptr<T>(), *this);
			}
			else {
				return *find_component_ptr<T>();
			}
		}
	}

	template <class F>
	void for_each_component(F&& callback) const {
		dispatch([&](const auto typed_handle) {
			typed_handle.for_each_component(std::forward<F>(callback));
		});
	}
};

template <bool is_const>
std::ostream& operator<<(std::ostream& out, const basic_entity_handle<is_const> &x) {
	return out << typesafe_sprintf("%x-%x", x.get_name(), x.get_id());
}
