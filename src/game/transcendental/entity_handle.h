#pragma once
#include <iosfwd>
#include "3rdparty/sol2/sol/forward.hpp"

#include "augs/build_settings/platform_defines.h"
#include "augs/templates/get_by_dynamic_id.h"

#include "augs/templates/maybe_const.h"
#include "augs/templates/component_traits.h"
#include "augs/templates/type_matching_and_indexing.h"

#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/cosmos_solvable_access.h"
#include "game/transcendental/entity_id.h"
#include "game/organization/all_components_declaration.h"

#include "game/detail/entity_handle_mixins/all_handle_mixins.h"
#include "game/enums/entity_flag.h"

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
	using owner_reference = maybe_const_ref_t<is_const, cosmos>;
	using entity_ptr = maybe_const_ptr_t<is_const, void>;

	using this_handle_type = basic_entity_handle<is_const>;
	using misc_base = misc_mixin<this_handle_type>;

	entity_ptr ptr;
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
	auto* find_ptr() const {
		return dispatch([](auto& typed_handle) { 
			return typed_handle.template find<T>(); 
		});
	}

public:
	using const_type = basic_entity_handle<!is_const>;
	using misc_base::get_raw_flavour_id;
	friend const_type;

	basic_entity_handle(
		owner_reference owner, 
		const entity_id raw_id
	) : basic_entity_handle(
		owner.get_solvable({}).get_entity_pool().find(raw_id),
		owner, 
		raw_id 
	) {
	}

	auto& get(cosmos_solvable_access) const {
		return agg();
	}

	const auto& get() const {
		return agg();
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

	bool operator==(const entity_id id) const {
		return raw_id == id;
	}

	bool operator!=(const entity_id id) const {
		return !operator==(id);
	}

	template <bool C = !is_const, class = std::enable_if_t<C>>
	operator const_entity_handle() const {
		return const_entity_handle(ptr, owner, raw_id);
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

	operator bool() const {
		return alive();
	}

	template <class F>
	decltype(auto) dispatch(F&& callback) {
		return get_by_dynamic_id(
			all_entity_types(),
			raw_id.type_id,
			[&](auto t) {
				using entity_type = decltype(t);
				using handle_type = basic_typed_entity_handle<is_const, entity_type>;

				auto& specific_ref = 
					*reinterpret_cast<
						maybe_const_ptr_t<is_const>(handle_type::aggregate_type)
					>(ptr)
				;
					
				return callback(handle_type(owner, specific_ref));
			}
		);
	}

	template <class T>
	bool has() const {
		return dispatch([](auto& typed_handle) { 
			return typed_handle.template has<T>(); 
		});
	}

	template<class T>
	decltype(auto) find() const {
		if constexpr(is_synchronized_v<T>) {
			return component_synchronizer<this_handle_type, T>(find_ptr<T>(), *this);
		}
		else {
			return find_ptr<T>();
		}
	}

	template<class T>
	decltype(auto) get() const {
		if constexpr(is_synchronized_v<T>) {
			return component_synchronizer<this_handle_type, T>(find_ptr<T>(), *this);
		}
		else {
			return *find_ptr<T>();
		}
	}

	template <bool C = !is_const, class = std::enable_if_t<C>>
	entity_handle construct_entity(const logic_step step) const;

	template <class F>
	void for_each_component(F&& callback) const {
		ensure(alive());

		agg().for_each_component(
			std::forward<F>(callback),
		   	pool_provider()
		);
	}
};

std::ostream& operator<<(std::ostream& out, const entity_handle&x);
std::ostream& operator<<(std::ostream& out, const const_entity_handle &x);
