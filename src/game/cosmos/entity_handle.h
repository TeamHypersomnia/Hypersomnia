#pragma once
#include <ostream>
#include "augs/build_settings/platform_defines.h"
#include "augs/templates/get_by_dynamic_id.h"
#include "augs/build_settings/platform_defines.h"
#include "game/cosmos/on_entity_meta.h"

#include "game/detail/inventory/inventory_slot_handle.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/traits/component_traits.h"
#include "augs/templates/traits/is_nullopt.h"
#include "augs/templates/folded_finders.h"

#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/cosmos_solvable_access.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_solvable.h"
#include "game/cosmos/specific_entity_handle.h"

#include "game/organization/all_components_declaration.h"

#include "game/detail/entity_handle_mixins/all_handle_mixins.h"

#include "game/cosmos/step_declaration.h"
#include "game/components/flags_component.h"

#include "game/components/sprite_sync.h"
#include "game/components/motor_joint_sync.h"

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

	template <class T>
	auto* find_invariant_ptr() const {
		return conditional_dispatch_ret<entity_types_having_all_of<T>>(
			[&](const auto& t) -> const T* { 
				if constexpr(is_nullopt_v<decltype(t)>) {
					return nullptr;
				}
				else {
					return t.get_flavour().template find<T>(); 
				}
			}
		);
	}
	
	template <class T, class H>
	static auto* find_or_nullptr(const H& t) {
		return t.template find_component_ptr<T>();
	}

	template <class T>
	static std::nullptr_t find_or_nullptr(const std::nullopt_t&) {
		return nullptr;
	}

	template <class T>
	auto* find_component_ptr() const {
		return conditional_dispatch_ret<entity_types_having_all_of<T>>(
			[&](const auto& t) -> maybe_const_ptr_t<is_const, T> {
				return find_or_nullptr<T>(t);
			}
		);
	}

public:
	static constexpr bool is_specific = false;

	using const_type = basic_entity_handle<!is_const>;
	using misc_base::get_raw_flavour_id;
	friend const_type;

	static this_handle_type dead_handle(owner_reference owner) {
		return { nullptr, owner, {} };
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

	const auto& get_meta() const {
		ensure(alive());
		return *reinterpret_cast<const entity_solvable_meta*>(ptr);
	}

	auto& get(cosmos_solvable_access) const {
		ensure(alive());
		return *ptr;
	}

	const auto& get() const {
		ensure(alive());
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
		return raw_id == h.raw_id;
	}

	bool operator==(const basic_entity_handle<!is_const>& h) const {
		return raw_id == h.raw_id;
	}

	bool operator==(const entity_id id) const {
		return raw_id == id;
	}

	bool operator!=(const entity_id id) const {
		return !operator==(id);
	}

	bool operator==(const entity_guid id) const {
		return this->get_guid() == id;
	}

	operator entity_guid() const {
		return this->get_guid();
	}

	operator entity_id() const {
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
		using handle_type = basic_ref_typed_entity_handle<is_const, E>;
		using specific_ptr_type = maybe_const_ptr_t<is_const, entity_solvable<E>>;

		const auto specific_ptr = reinterpret_cast<specific_ptr_type>(ptr);
		const auto stored_id = ref_stored_id_provider<handle_type>( *specific_ptr, typed_entity_id<E>(raw_id.raw) );
		return handle_type(owner, stored_id);
	}

	template <class List, class F>
	FORCE_INLINE decltype(auto) conditional_dispatch_ret(F&& callback) const {
		ensure(alive());

		return conditional_find_by_dynamic_id<List>(
			all_entity_types(), 
			raw_id.type_id,
			[&](auto t) -> decltype(auto) { 
				using E = remove_cref<decltype(t)>;

				if constexpr(is_nullopt_v<E>) {
					return callback(std::nullopt);
				}
				else {
					return callback(this->get_specific<E>());
				}
			}
		);
	}

	template <class List, class F>
	FORCE_INLINE void conditional_dispatch(F&& callback) const {
		this->conditional_dispatch_ret<List>(
			[&](const auto& typed_handle) {
				if constexpr(!is_nullopt_v<decltype(typed_handle)>) {
					callback(typed_handle);
				}
			}
		);
	}

	template <class... List, class F>
	decltype(auto) dispatch_on_having_all_ret(F&& callback) const {
		return this->conditional_dispatch_ret<entity_types_having_all_of<List...>>(
			[&](const auto& typed_handle) -> decltype(auto) {
				return callback(typed_handle);
			}
		);
	}

	template <class... List, class F>
	void dispatch_on_having_all(F&& callback) const {
		this->conditional_dispatch_ret<entity_types_having_all_of<List...>>(
			[&](const auto& typed_handle) {
				if constexpr(!is_nullopt_v<decltype(typed_handle)>) {
					callback(typed_handle);
				}
			}
		);
	}

	template <class... List, class F>
	void dispatch_on_having_any(F&& callback) const {
		this->conditional_dispatch_ret<entity_types_having_any_of<List...>>(
			[&](const auto& typed_handle) {
				if constexpr(!is_nullopt_v<decltype(typed_handle)>) {
					callback(typed_handle);
				}
			}
		);
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
		return this->conditional_dispatch_ret<entity_types_having_any_of<T>>(
			[&](const auto& typed_handle) {
				return !is_nullopt_v<decltype(typed_handle)>;
			}
		);
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
			return *find_invariant_ptr<T>();
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
	if (x.dead()) {
		return out << "(dead handle)";
	}

	return out << typesafe_sprintf("%x-%x", x.get_name(), x.get_id());
}

template <class C>
auto subscript_handle_getter(C& cosm, const entity_id id) 
	-> basic_entity_handle<std::is_const_v<C>>
{
	const auto ptr = cosm.get_solvable({}).on_entity_meta(id, [&](auto* const agg) {
		return reinterpret_cast<maybe_const_ptr_t<std::is_const_v<C>, void>>(agg);	
	});

	return { ptr, cosm, id };
}

template <class C>
auto subscript_handle_getter(C& cosm, const child_entity_id id)
	-> basic_entity_handle<std::is_const_v<C>>
{
	return subscript_handle_getter(cosm, entity_id(id));
}

template <class C>
auto subscript_handle_getter(C& cosm, const unversioned_entity_id id)
	-> basic_entity_handle<std::is_const_v<C>>
{
	return subscript_handle_getter(cosm, cosm.find_versioned(id));
}

template <class C>
auto subscript_handle_getter(C& cosm, const entity_guid guid)
	-> basic_entity_handle<std::is_const_v<C>>
{
	return subscript_handle_getter(cosm, cosm.get_solvable().get_entity_id_by(guid));
}
