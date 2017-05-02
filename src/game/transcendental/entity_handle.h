#pragma once
#include <iosfwd>

#include "augs/templates/maybe_const.h"
#include "augs/templates/is_component_synchronized.h"
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/templates/for_each_in_types.h"
#include "augs/templates/list_ops.h"

#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/entity_system/component_aggregate.h"
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
#include "augs/build_settings/setting_entity_handle_has_debug_name_reference.h"
#include "game/transcendental/step_declaration.h"

template<class F>
void for_each_component_type(F callback) {
	for_each_through_std_get(
		put_all_components_into_t<std::tuple>(), 
		callback
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
		static_assert(is_one_of_list_v<T, concat_lists_t<disabled_components, put_all_components_into_t<type_list>>>, "Unknown component type!");
	}

public:
	static constexpr bool is_const_value = is_const;
private:
	template <class, class...> friend class augs::operations_on_all_components_mixin;

	template <bool, class> friend class relations_mixin;
	template <bool, class> friend class basic_relations_mixin;

	typedef maybe_const_ref_t<is_const, cosmos> owner_reference;

	owner_reference owner;
	entity_id raw_id;
#if ENTITY_HANDLE_HAS_DEBUG_NAME_REFERENCE
	const std::string& debug_name;
#endif

	entity_id get_id_for_handle_operations() const {
		return raw_id;
	}

	decltype(auto) get_pool_for_handle_operations() const {
		return owner.get_aggregate_pool();
	}

	typedef augs::component_allocators_mixin<is_const, basic_entity_handle<is_const>> allocator;

	friend class augs::component_allocators_mixin<is_const, basic_entity_handle<is_const>>;
	friend class cosmic_delta;
	friend class cosmos;

	template <class T, typename = void>
	struct component_or_synchronizer_or_disabled {
		typedef maybe_const_ref_t<is_const, T> return_type;
		typedef maybe_const_ptr_t<is_const, T> return_ptr;

		basic_entity_handle<is_const> h;

		return_ptr find() const {
			return h.allocator::template find<T>();
		}

		bool has() const {
			return h.allocator::template has<T>();
		}

		return_type get() const {
			return h.allocator::template get<T>();
		}

		void add(const T& t) const {
			h.allocator::add(t);
		}
	};

	template <class T>
	struct component_or_synchronizer_or_disabled<T, std::enable_if_t<is_component_synchronized<T> && !is_one_of_list_v<T, disabled_components>>> {
		typedef component_synchronizer<is_const, T> return_type;

		basic_entity_handle<is_const> h;

		bool has() const {
			return h.allocator::template has<T>();
		}

		return_type find() const {
			return { h.allocator::template find<T>(), h };
		}

		return_type get() const {
			return { &h.allocator::template get<T>(), h };
		}

		void add(const T& t) const {
			h.allocator::add(t);
			h.get_cosmos().complete_reinference(h);
		}
	};

	template <class T>
	struct component_or_synchronizer_or_disabled<T, std::enable_if_t<is_one_of_list_v<T, disabled_components>>> {
		typedef maybe_const_ref_t<is_const, T> return_type;
		typedef maybe_const_ptr_t<is_const, T> return_ptr;

		basic_entity_handle<is_const> h;

		return_ptr find() const {
			return nullptr;
		}

		bool has() const {
			return false;
		}

		return_type get() const {
			thread_local T t;
			t = T();
			return t;
		}

		void add(const T& t) const {

		}
	};

	template<class T>
	using component_or_synchronizer_t = typename component_or_synchronizer_or_disabled<T>::return_type;

	decltype(auto) get_aggregate_pool() const {
		return owner.get_aggregate_pool();
	}

	decltype(auto) get() const {
		return get_aggregate_pool().get(raw_id);
	}

public:
	basic_entity_handle(
		owner_reference owner, 
		const entity_id raw_id
	) : 
		raw_id(raw_id), 
		owner(owner) 
#if ENTITY_HANDLE_HAS_DEBUG_NAME_REFERENCE
		, debug_name(owner.get_debug_name(raw_id))
#endif
	{

	}

	friend std::ostream& operator<<(std::ostream& out, const basic_entity_handle &x);

	entity_id get_id() const {
		return raw_id;
	}

	void set_id(const entity_id id) {
		raw_id = id;
	}

	bool alive() const {
		return get_aggregate_pool().alive(get_id());
	}

	bool dead() const {
		return !alive();
	}

	std::string get_debug_name() const {
		return get_cosmos().get_debug_name(raw_id);
	}

	typename owner_reference get_cosmos() const {
		return this->owner;
	}

	bool operator==(entity_id id) const {
		return raw_id == id;
	}

	bool operator!=(entity_id id) const {
		return raw_id != id;
	}

	template <bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
	operator const_entity_handle() const {
		return const_entity_handle(this->owner, this->raw_id);
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

	template <class component>
	bool has() const {
		check_component_type<component>();
		ensure(alive());
		return component_or_synchronizer_or_disabled<component>({ *this }).has();
	}

	template<class component>
	decltype(auto) get() const {
		check_component_type<component>();
		ensure(alive());
		return component_or_synchronizer_or_disabled<component>({ *this }).get();
	}

	template<class component, bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
	decltype(auto) add(const component& c) const {
		check_component_type<component>();
		ensure(alive());
		component_or_synchronizer_or_disabled<component>({ *this }).add(c);
		return get<component>();
	}

	template<class component, bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
	decltype(auto) add(const component_synchronizer<is_const, component>& c) const {
		check_component_type<component>();
		ensure(alive());
		component_or_synchronizer_or_disabled<component>({ *this }).add(c.get_raw_component());
		return get<component>();
	}

	template<class component>
	decltype(auto) find() const {
		check_component_type<component>();
		ensure(alive());
		return component_or_synchronizer_or_disabled<component>({ *this }).find();
	}

	template<bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
	basic_entity_handle<is_const> add_standard_components(const logic_step step) const;

	template<bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
	void recalculate_basic_processing_categories() const;

	bool get_flag(const entity_flag f) const {
		ensure(alive());
		components::flags from;

		if (has<components::flags>()) {
			from = get<components::flags>();
		}

		return from.bit_flags.test(f);
	}

	template<bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
	void set_flag(const entity_flag f) const {
		ensure(alive());
		if (!has<components::flags>()) {
			add(components::flags());
		}

		get<components::flags>().bit_flags.set(f, true);
	}

	template<bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
	void unset_flag(const entity_flag f) const {
		ensure(alive());
		if (!has<components::flags>()) {
			add(components::flags());
		}

		get<components::flags>().bit_flags.set(f, false);
	}

	template <class F>
	void for_each_component(F callback) const {
		const auto& ids = get().component_ids;
		auto& cosm = get_cosmos();

		for_each_through_std_get(
			ids,
			[&](const auto& id) {
				typedef typename std::decay_t<decltype(id)>::element_type component_type;
				const auto component_handle = cosm.get_component_pool<component_type>().get_handle(id);

				if (component_handle.alive()) {
					callback(component_handle.get());
				}
			}
		);
	}

	bool is_inferred_state_activated() const {
		const auto inferred = find<components::all_inferred_state>();

		return inferred != nullptr && inferred.is_activated();
	}
};

template <bool is_const>
std::vector<entity_id> to_id_vector(std::vector<basic_entity_handle<is_const>> vec) {
	return std::vector<entity_id>(vec.begin(), vec.end());
}

size_t make_cache_id(const unversioned_entity_id id);
size_t make_cache_id(const entity_id id);
size_t make_cache_id(const const_entity_handle handle);

inline size_t make_cache_id(const entity_id id) {
	ensure(id.indirection_index >= 0);
	return id.indirection_index;
}

inline size_t make_cache_id(const unversioned_entity_id id) {
	ensure(id.indirection_index >= 0);
	return id.indirection_index;
}

inline size_t make_cache_id(const const_entity_handle handle) {
	return make_cache_id(handle.get_id());
}