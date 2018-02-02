#pragma once
#include <tuple>
#include "augs/misc/trivially_copyable_tuple.h"

#include "game/organization/all_entity_types.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/templates/type_mod_templates.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/detail/entity_handle_mixins/all_handle_mixins.h"

static constexpr bool statically_allocate_entities = STATICALLY_ALLOCATE_ENTITIES;

template <class T>
using make_entity_flavour = 
	replace_list_type_t<
		T::invariants, 
		std::conditional_v<
			match_exists_in_list_v<apply_negation<std::is_trivially_copyable>, T::invariants>,
			std::tuple,
			augs::trivially_copyable_tuple
		>
	>
;

template <class T>
using make_aggregate = 
	replace_list_type_t<
		T::components, 
		std::conditional_v<
			match_exists_in_list_v<apply_negation<std::is_trivially_copyable>, T::components>,
			std::tuple,
			augs::trivially_copyable_tuple
		>
	>
;

template <class T>
using make_aggregate_pool = std::conditional_v<
	statically_allocate_entities,
	augs::pool<make_aggregate<T>, of_size<T::statically_allocated_num>::make_constant_vector, cosmic_pool_size_type>,
	augs::pool<make_aggregate<T>, make_vector, cosmic_pool_size_type>
>;

using all_aggregate_pools = 
	replace_list_type_t<
		transform_types_in_list_v<
			all_entity_types,
			make_aggregate_pool
		>,
		std::tuple
	>
;

template <bool is_const, class entity_type>
class basic_iterated_entity_handle :
	public misc_mixin<basic_entity_handle<is_const>>,
	public inventory_mixin<basic_entity_handle<is_const>>,
	public physics_mixin<basic_entity_handle<is_const>>,
	public relations_mixin<basic_entity_handle<is_const>>,
	public spatial_properties_mixin<basic_entity_handle<is_const>>
{
	using this_handle_type = basic_iterated_entity_handle<is_const, entity_type>;
	using misc_base = misc_mixin<this_handle_type>;

	using const_handle_type = basic_iterated_entity_handle<true, entity_type>;

	using aggregate_type = make_aggregate<entity_type>;
	using flavour_type = make_entity_flavour<entity_type>;

	using aggregate_pool_type = make_aggregate_pool<entity_type>;

	using aggregate_reference = maybe_const_ref_t<is_const, aggregate_type>;
	using void_aggregate_ptr = maybe_const_ptr_t<is_const, void>;

	using owner_reference = maybe_const_ref_t<is_const, cosmos>;

	friend class cosmos;
	friend basic_iterated_entity_handle<!is_const, entity_type>;

	aggregate_reference subject;
	owner_reference owner;
	unsigned iteration_index = 0u;

	template <class T>
	maybe_const_ptr_t<is_const, T> find_component_ptr() const {
		if constexpr(has<T>()) {
			return std::addressof(std::get<T>(subject));
		}

		return nullptr;
	}

	static auto& get_pool(owner_reference ref) {
		return 
			std::get<aggregate_pool_type>(owner.get_solvable({}).significant.aggregate_pools)
		;
	}

private:
	basic_iterated_entity_handle(
		aggregate_reference subject,
		owner_reference owner,
		const entity_id iteration_index
	) :
		subject(subject),
		owner(owner),
		iteration_index(iteration_index)
	{}

public:
	using misc_base::get_flavour_id;

	basic_iterated_entity_handle(
		owner_reference owner,
		const unsigned iteration_index	
	) : 
		owner(owner),
		iteration_index(iteration_index),
		subject(get_pool(owner).data()[iteration_index])
	{
	}


	const entity_flavour& get_flavour() const{
		return handle.get_cosmos().get_flavour(get_flavour_id());
	}

	const flavour_type& get_flavour() const {
		const auto id = get_flavour_id(); 
		return owner.get_flavour();
		const auto self = *static_cast<const E*>(this);
		return self.template get<components::flavour>().get_flavour();
	}

	template <class component>
 	constexpr bool has() const {
		return is_one_of_list_v<component, aggregate_type>;
	}

	template <class T>
	decltype(auto) get() const {
		if constexpr(is_invariant_v<T>) {
			return get_flavour().template get<T>();
		}
		else {
			static_assert(has<component>());

			if constexpr(is_synchronized_v<T>) {
				return component_synchronizer<this_handle_type, T>(find_component_ptr<T>(), *this);
			}
			else {
				return *find_component_ptr<T>();
			}
		}
	}

	template<class T>
	decltype(auto) find() const {
		if constexpr(is_invariant_v<T>) {
			return get_flavour().template find<T>();
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

	auto& get(cosmos_solvable_access) const {
		return subject;
	}

	const auto& get() const {
		return subject;
	}

	entity_id get_id() const {
		entity_type_id type_id;
		type_id.set<entity_type>();

		return {
			get_pool(owner).to_id(iteration_index),
			type_id
		};
	}

	auto& get_cosmos() const {
		return owner;
	}

	bool operator==(const entity_id id) const {
		return get_id() == id;
	}

	bool operator!=(const entity_id id) const {
		return !operator==(id);
	}

	template <bool C = !is_const, class = std::enable_if_t<C>>
	operator const_handle_type() const {
		return const_handle_type(subject, owner, iteration_index);
	}

	operator entity_id() const {
		return get_id();
	}

	operator child_entity_id() const {
		return get_id();
	}

	operator unversioned_entity_id() const {
		return get_id();
	}

	operator basic_entity_handle<is_const>() const {
		return { static_cast<void_entity_ptr>(ptr), owner, get_id() };
	}
};

template <class entity_type>
using iterated_entity_handle = iterated_entity_handle<false, entity_type>;

template <class entity_type>
using const_iterated_entity_handle = iterated_entity_handle<true, entity_type>;
