#pragma once
#include <iosfwd>
#include "3rdparty/sol2/sol/forward.hpp"

#include "augs/build_settings/platform_defines.h"

#include "augs/templates/maybe_const.h"
#include "augs/templates/component_traits.h"
#include "augs/templates/type_matching_and_indexing.h"

#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/cosmos_solvable_access.h"
#include "game/transcendental/entity_id.h"
#include "game/organization/all_components_declaration.h"

#include "game/detail/entity_handle_mixins/inventory_mixin.h"
#include "game/detail/entity_handle_mixins/physics_mixin.h"
#include "game/detail/entity_handle_mixins/relations_mixin.h"
#include "game/detail/entity_handle_mixins/spatial_properties_mixin.h"

#include "game/enums/entity_flag.h"
#include "game/transcendental/step_declaration.h"
#include "game/components/flags_component.h"

class cosmos;

template <class, class>
class component_synchronizer;

template <bool is_const>
class basic_entity_handle :
	public inventory_mixin<basic_entity_handle<is_const>>,
	public physics_mixin<basic_entity_handle<is_const>>,
	public relations_mixin<basic_entity_handle<is_const>>,
	public spatial_properties_mixin<basic_entity_handle<is_const>>
{
	using owner_reference = maybe_const_ref_t<is_const, cosmos>;
	using entity_ptr = maybe_const_ptr_t<is_const, cosmic_entity>;

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

	auto& agg() const {
		return *ptr;
	}

private:
	basic_entity_handle(
		const entity_ptr ptr,
		owner_reference owner,
		const entity_id raw_id
	) :
		ptr(ptr),
		owner(owner),
		raw_id(raw_id)
	{}

public:
	static constexpr bool is_const_value = is_const;
	using this_handle_type = basic_entity_handle<is_const>;
	using const_type = basic_entity_handle<!is_const>;
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

	entity_id get_id() const {
		return raw_id;
	}

	void set_id(const entity_id id) {
		raw_id = id;
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
		return raw_id != id;
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

	template <class component>
	bool has() const {
		return agg().template has<component>(pool_provider());
	}

	template <class T>
	decltype(auto) get() const {
		if constexpr(is_invariant_v<T>) {
			return get_flavour().template get<T>();
		}
		else {
			check_component_type<T>();

			ensure(alive());

			if constexpr(is_synchronized_v<T>) {
				return component_synchronizer<this_handle_type, T>(&agg().template get<T>(pool_provider()), *this);
			}
			else {
				return agg().template get<T>(pool_provider());
			}
		}
	}

	template <class T, bool C = !is_const, class = std::enable_if_t<C>>
	void add(const T& c) const {
		check_component_type<T>();
		ensure(alive());
		
		if constexpr(is_synchronized_v<T>) {
			agg().template add<T>(c, pool_provider());
		}
		else {
			agg().template add<T>(c, pool_provider());
		}
	}

	template <class T, bool C = !is_const, class = std::enable_if_t<C>>
	void add(const component_synchronizer<this_handle_type, T>& c) const {
		add(c.get_raw_component());
	}

	template<class T>
	decltype(auto) find() const {
		if constexpr(is_invariant_v<T>) {
			return get_flavour().template find<T>();
		}
		else {
			check_component_type<T>();

			ensure(alive());

			if constexpr(is_synchronized_v<T>) {
				return component_synchronizer<this_handle_type, T>(agg().template find<T>(pool_provider()), *this);
			}
			else {
				return agg().template find<T>(pool_provider());
			}
		}
	}

	template <class component, bool C = !is_const, class = std::enable_if_t<C>>
	decltype(auto) operator+=(const component& c) const {
		add(c);
		return get<component>();
	}

	template <bool C = !is_const, class = std::enable_if_t<C>>
	entity_handle add_standard_components(const logic_step step) const;

	template <bool C = !is_const, class = std::enable_if_t<C>>
	void recalculate_basic_processing_categories() const;

	bool get_flag(const entity_flag f) const {
		ensure(alive());
		return get<invariants::flags>().values.test(f);
	}

	template <class F>
	void for_each_component(F&& callback) const {
		ensure(alive());

		agg().for_each_component(
			std::forward<F>(callback),
		   	pool_provider()
		);
	}
	
	entity_guid get_guid() const {
		return get_cosmos().get_solvable().get_guid(raw_id);
	}

	const auto& get_flavour() const {
		return get<components::flavour>().get_flavour();
	}

	auto get_flavour_id() const {
		return get<components::flavour>().get_flavour_id();
	}

	const auto& get_name() const {
		return get<components::flavour>().get_name();
	}

	bool sentient_and_unconscious() const {
		if (const auto sentience = find<components::sentience>()) {
			return !sentience->is_conscious();
		}

		return false;
	}
};

std::ostream& operator<<(std::ostream& out, const entity_handle&x);
std::ostream& operator<<(std::ostream& out, const const_entity_handle &x);

inline auto linear_cache_key(const const_entity_handle handle) {
	return linear_cache_key(handle.get_id());
}
