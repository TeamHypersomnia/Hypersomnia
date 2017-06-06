#pragma once
#include <iosfwd>

#include "augs/templates/maybe_const.h"
#include "augs/templates/component_traits.h"
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/templates/for_each_in_types.h"
#include "augs/templates/list_ops.h"

#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "augs/entity_system/component_setters_mixin.h"
#include "augs/entity_system/component_allocators_mixin.h"
#include "augs/misc/pool_handle.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/types_specification/all_components_declaration.h"

#include "game/detail/entity_handle_mixins/inventory_mixin.h"
#include "game/detail/entity_handle_mixins/physics_mixin.h"
#include "game/detail/entity_handle_mixins/relations_mixin.h"
#include "game/detail/entity_handle_mixins/spatial_properties_mixin.h"

#include "game/enums/entity_flag.h"
#include "augs/build_settings/setting_empty_bases.h"
#include "game/transcendental/step_declaration.h"
#include "game/components/name_component_declaration.h"

template <class T>
constexpr std::size_t component_index_v = index_in_list_v<T, put_all_components_into_t<type_list>>;

template<class F>
void for_each_component_type(F&& callback) {
	for_each_through_std_get(
		put_all_components_into_t<std::tuple>(), 
		std::forward<F>(callback)
	);
}

class cosmos;
class cosmic_delta;

template <bool, class>
class component_synchronizer;

namespace augs {
	template <class, class...>
	class operations_on_all_components_mixin;
}

template <bool is_const>
class EMPTY_BASES basic_entity_handle :
	private augs::component_allocators_mixin<is_const, basic_entity_handle<is_const>>,
	public augs::component_setters_mixin<is_const, basic_entity_handle<is_const>>,

	public inventory_mixin<is_const, basic_entity_handle<is_const>>,
	public physics_mixin<is_const, basic_entity_handle<is_const>>,
	public relations_mixin<is_const, basic_entity_handle<is_const>>,
	public spatial_properties_mixin<is_const, basic_entity_handle<is_const>>
{
	template <typename T>
	static void check_component_type() {
		static_assert(is_one_of_list_v<T, put_all_components_into_t<type_list>>, "Unknown component type!");
	}

public:
	static constexpr bool is_const_value = is_const;
private:
	template <class, class...> friend class augs::operations_on_all_components_mixin;

	template <bool, class> friend class relations_mixin;
	template <bool, class> friend class basic_relations_mixin;
	template <bool> friend class basic_entity_handle;

	using owner_reference = maybe_const_ref_t<is_const, cosmos>;
	using aggregate_ptr = maybe_const_ptr_t<is_const, put_all_components_into_t<augs::component_aggregate>>;

	owner_reference owner;
	entity_id raw_id;
	aggregate_ptr ptr;

	typedef augs::component_allocators_mixin<is_const, basic_entity_handle<is_const>> allocator;

	friend class augs::component_allocators_mixin<is_const, basic_entity_handle<is_const>>;
	friend class cosmic_delta;
	friend class cosmos;

	auto& get() const {
		return *ptr;
	}

private:
	basic_entity_handle(
		owner_reference owner,
		const entity_id raw_id,
		const aggregate_ptr ptr
	) :
		raw_id(raw_id),
		owner(owner),
		ptr(ptr)
	{}

public:
	basic_entity_handle(
		owner_reference owner, 
		const entity_id raw_id
	) : basic_entity_handle(
		owner, 
		raw_id, 
		owner.get_aggregate_pool().find(raw_id)
	) {
	}

	friend std::ostream& operator<<(std::ostream& out, const basic_entity_handle &x);

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

	template <class = std::enable_if_t<!is_const>>
	operator const_entity_handle() const {
		return const_entity_handle(owner, raw_id, ptr);
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

	using allocator::has;

	template <class T>
	decltype(auto) get() const {
		check_component_type<T>();

		ensure(alive());
		
		if constexpr(is_component_synchronized_v<T>) {
			return component_synchronizer<is_const, T>(&allocator::template get<T>(), *this);
		}
		else {
			return allocator::template get<T>();
		}
	}

	template <class T, class = std::enable_if_t<!is_const>>
	void add(const T& c) const {
		check_component_type<T>();
		ensure(alive());
		
		if constexpr(is_component_synchronized_v<T>) {
			allocator::template add<T>(c);
			owner.complete_reinference(*this);
		}
		else {
			allocator::template add<T>(c);
		}
	}

	template <class T, class = std::enable_if_t<!is_const>>
	void add(const component_synchronizer<is_const, T>& c) const {
		add(c.get_raw_component());
	}

	template<class T>
	decltype(auto) find() const {
		check_component_type<T>();

		ensure(alive());

		if constexpr(is_component_synchronized_v<T>) {
			return component_synchronizer<is_const, T>(allocator::template find<T>(), *this);
		}
		else {
			return allocator::template find<T>();
		}
	}

	template<class T, class = std::enable_if_t<!is_const>>
	void remove() const {
		check_component_type<T>();

		ensure(alive());

		if constexpr(is_component_synchronized_v<T>) {
			allocator::template remove<T>();
			owner.complete_reinference(*this);
		}
		else {
			allocator::template remove<T>();
		}
	}

	template <class = std::enable_if_t<!is_const>>
	basic_entity_handle<is_const> add_standard_components(const logic_step step, const bool activate_inferred) const;

	template <class = std::enable_if_t<!is_const>>
	basic_entity_handle<is_const> add_standard_components(const logic_step step) const;

	template <class = std::enable_if_t<!is_const>>
	void recalculate_basic_processing_categories() const;

	bool get_flag(const entity_flag f) const {
		ensure(alive());
		components::flags from;

		if (has<components::flags>()) {
			from = get<components::flags>();
		}

		return from.values.test(f);
	}

	template <class = std::enable_if_t<!is_const>>
	void set_flag(const entity_flag f) const {
		ensure(alive());
		if (!has<components::flags>()) {
			add(components::flags());
		}

		get<components::flags>().values.set(f, true);
	}

	template<class = std::enable_if_t<!is_const>>
	void unset_flag(const entity_flag f) const {
		ensure(alive());
		if (!has<components::flags>()) {
			add(components::flags());
		}

		get<components::flags>().values.set(f, false);
	}

	template <class F>
	void for_each_component(F&& callback) const {
		auto& self = get();
		const auto& ids = self.component_ids;
		auto& fundamentals = self.fundamentals;
		auto& cosm = get_cosmos();

		for_each_through_std_get(
			fundamentals,
			std::forward<F>(callback)
		);

		for_each_through_std_get(
			ids,
			[&](const auto& id) {
				using component_type = typename std::decay_t<decltype(id)>::element_type;
				const auto component_handle = cosm.get_component_pool<component_type>().get_handle(id);

				if (component_handle.alive()) {
					callback(component_handle.get());
				}
			}
		);
	}
	
	auto& get_meta_of_name() const {
		return get<components::name>().get_meta();
	}

	const auto& get_name() const {
		return get<components::name>().get_name();
	}

	template <class = std::enable_if_t<!is_const>>
	void set_name(const entity_name_type& new_name) const;

	bool is_inferred_state_activated() const {
		const auto inferred = find<components::all_inferred_state>();

		return inferred != nullptr && inferred.is_activated();
	}
};

template <bool is_const>
std::vector<entity_id> to_id_vector(std::vector<basic_entity_handle<is_const>> vec) {
	return std::vector<entity_id>(vec.begin(), vec.end());
}

template <class id_type>
bool is_valid_cache_id(const id_type id) {
	return id.indirection_index >= 0;
}

size_t make_cache_id(const unversioned_entity_id id);
size_t make_cache_id(const entity_id id);
size_t make_cache_id(const const_entity_handle handle);

inline size_t make_cache_id(const entity_id id) {
	ensure(is_valid_cache_id(id));
	return id.indirection_index;
}

inline size_t make_cache_id(const unversioned_entity_id id) {
	ensure(is_valid_cache_id(id));
	return id.indirection_index;
}

inline size_t make_cache_id(const const_entity_handle handle) {
	return make_cache_id(handle.get_id());
}