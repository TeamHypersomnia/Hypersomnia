#pragma once
#include "augs/templates/type_matching_and_indexing.h"
#include "game/transcendental/entity_type.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/typed_entity_handle_declaration.h"

#include "game/detail/entity_handle_mixins/all_handle_mixins.h"

struct empty_id_provider {};

template <class derived_handle_type>
struct iterated_id_provider {
	const unsigned iteration_index;
	
	iterated_id_provider(const unsigned iteration_index) 
		: iteration_index(iteration_index) 
	{}
	
	static auto& get_pool(owner_reference owner) {
		return 
			std::get<typename handle_type::subject_pool_type>(owner.get_solvable({}).significant.entity_pools)
		;
	}

	entity_id get_id() const {
		const auto h = *static_cast<derived_handle_type*>(this);

		return {
			get_pool(h.get_cosmos()).to_id(iteration_index),
			h.get_type_id()
		};
	}
};

template <class derived_handle_type>
struct stored_id_provider {
	const entity_id_base stored_id;

	stored_id_provider(const entity_id_base stored_id) 
		: stored_id(stored_id) 
	{}

	entity_id get_id() const {
		const auto h = *static_cast<derived_handle_type*>(this);
		return { stored_id, h.get_type_id() };
	}
};

template <bool is_const, class entity_type, class identifier_provider>
class specific_entity_handle :
	public misc_mixin<specific_entity_handle<is_const, entity_type, identifier_provider>>,
	public inventory_mixin<specific_entity_handle<is_const, entity_type, identifier_provider>>,
	public physics_mixin<specific_entity_handle<is_const, entity_type, identifier_provider>>,
	public relations_mixin<specific_entity_handle<is_const, entity_type, identifier_provider>>,
	public spatial_properties_mixin<specific_entity_handle<is_const, entity_type, identifier_provider>>,
	public identifier_provider
{
	using this_handle_type = specific_entity_handle<is_const, entity_type, identifier_provider>;
	using misc_base = misc_mixin<this_handle_type>;

	using const_handle_type = specific_entity_handle<true, entity_type, identifier_provider>;

	using components_type = make_components<entity_type>;
	using flavour_type = make_entity_flavour<entity_type>;

	using subject_pool_type = make_entity_pool<entity_type>;

	using subject_reference = maybe_const_ref_t<is_const, entity_solvable<entity_type>>;

	using owner_reference = maybe_const_ref_t<is_const, cosmos>;

	friend class cosmos;
	friend specific_entity_handle<!is_const, entity_type, identifier_provider>;

	using identifier_provider::get_id;

	subject_reference subject;
	owner_reference owner;

	template <class T>
	maybe_const_ptr_t<is_const, T> find_component_ptr() const {
		if constexpr(subject.template has<T>()) {
			return std::addressof(subject.template get<T>());
		}

		return nullptr;
	}

	template <bool is_const>
	friend class basic_entity_handle;

public:
	specific_entity_handle(
		subject_reference subject,
		owner_reference owner,
		const identifier_provider identifier
	) :
		subject(subject),
		owner(owner),
		identifier_provider(identifier)
	{}

	using misc_base::get_flavour;
	using used_entity_type = entity_type;

	template <class T>
	constexpr bool has() const {
		return 
			subject.template has<components_type> 
			|| get_flavour().template has<T>
		;
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

	template <class T>
	decltype(auto) get() const {
		if constexpr(is_invariant_v<T>) {
			return get_flavour().template get<T>();
		}
		else {
			static_assert(has<component>());

			if constexpr(is_synchronized_v<T>) {
				return find<T>();
			}
			else {
				return *find<T>();
			}
		}
	}

	const auto& get_meta() const {
		return static_cast<const entity_solvable_meta&>(subject);
	};

	auto& get(cosmos_solvable_access) const {
		return subject;
	}

	const auto& get() const {
		return subject;
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
		return const_handle_type(subject, owner, identifier);
	}

	auto get_type_id() const {
		return entity_type_id::of<entity_type>;
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

	template <class F>
	void for_each_component(F&& callback) const {
		const auto& immutable_subject = subject;

		for_each_std_through_get(
			immutable_subject.components, 
			std::forward<F>(callback)
		);
	}

	operator basic_entity_handle<is_const>() const {
		return { static_cast<void_entity_ptr>(ptr), owner, get_id() };
	}
};

/* Shortcut */

template <class T>
using entity_type_of = typename std::decay_t<T>::used_entity_type;
