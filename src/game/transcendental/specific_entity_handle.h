#pragma once
#include "augs/templates/folded_finders.h"
#include "augs/templates/for_each_std_get.h"

#include "game/transcendental/component_synchronizer.h"
#include "game/transcendental/entity_pools.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/specific_entity_handle_declaration.h"
#include "game/transcendental/entity_solvable.h"
#include "game/transcendental/cosmos_solvable_access.h"

#include "game/detail/entity_handle_mixins/all_handle_mixins.h"
#include "game/common_state/entity_flavours.h"

template <class E>
struct entity_solvable;

template <class E>
struct empty_id_provider {};

template <class derived_handle_type>
struct iterated_id_provider {
private:
	using E = entity_type_of<derived_handle_type>;
	static constexpr auto is_const = is_handle_const_v<derived_handle_type>;

	using subject_reference = maybe_const_ref_t<is_const, entity_solvable<E>>;

	subject_reference subject;
	const unsigned iteration_index;
	
protected:
	auto& get_subject() const {
		return subject;
	}

	auto* find_subject() const {
		return &subject;
	}

public:
	iterated_id_provider(
		subject_reference subject,
		const unsigned iteration_index = static_cast<unsigned>(-1)
	) : 
		subject(subject),
		iteration_index(iteration_index) 
	{}

	template <class T>
	iterated_id_provider(
		const iterated_id_provider<T>& b
	) : 
		subject(b.subject),
		iteration_index(b.iteration_index)
	{
	}

	auto get_id() const {
		const auto h = *static_cast<const derived_handle_type*>(this);
		return typed_entity_id<E>(h.get_pool().to_id(iteration_index));
	}

	constexpr bool alive() const {
		return true;
	}

	constexpr bool dead() const {
		return false;
	}

	constexpr explicit operator bool() const {
		return true;
	}

	auto& get(cosmos_solvable_access) const {
		return subject;
	}

	const auto& get() const {
		return subject;
	}

	void ensure_alive() const {}
};

template <class derived_handle_type>
struct stored_id_provider {
private:
	using E = entity_type_of<derived_handle_type>;
	static constexpr auto is_const = is_handle_const_v<derived_handle_type>;

	using id_type = typed_entity_id<E>;
	using subject_pointer = maybe_const_ptr_t<is_const, entity_solvable<E>>;

protected:
	const subject_pointer subject;
	const id_type stored_id;

	auto& get_subject() const {
		return *subject;
	}

	auto* find_subject() const {
		return subject;
	}

public:
	stored_id_provider(
		const subject_pointer subject,
		const id_type stored_id
	) : 
		subject(subject),
		stored_id(stored_id)
	{}

	template <class T>
	stored_id_provider(
		const stored_id_provider<T>& b
	) : 
		subject(b.subject),
		stored_id(b.stored_id)
	{
	}

	auto get_id() const {
		return stored_id;
	}

	bool alive() const {
		return subject != nullptr;
	}

	bool dead() const {
		return subject == nullptr;
	}

	explicit operator bool() const {
		return subject != nullptr;
	}

	auto& get(cosmos_solvable_access) const {
		return *subject;
	}

	const auto& get() const {
		return *subject;
	}

	void ensure_alive() const {
		ensure(subject != nullptr);
	}
};

template <class derived_handle_type>
struct ref_stored_id_provider {
private:
	using E = entity_type_of<derived_handle_type>;
	static constexpr auto is_const = is_handle_const_v<derived_handle_type>;

	using id_type = typed_entity_id<E>;

	using subject_reference = maybe_const_ref_t<is_const, entity_solvable<E>>;
	const subject_reference subject;
	
	const id_type stored_id;

protected:
	auto& get_subject() const {
		return subject;
	}

	auto* find_subject() const {
		return &subject;
	}

public:
	template <class T>
	ref_stored_id_provider(
		const ref_stored_id_provider<T>& b
	) : 
		subject(b.subject),
		stored_id(b.stored_id)
	{
	}

	ref_stored_id_provider(
		const subject_reference subject,
		const id_type stored_id
	) : 
		subject(subject),
		stored_id(stored_id)
	{}

	auto get_id() const {
		return stored_id;
	}

	constexpr bool alive() const {
		return true;
	}

	constexpr bool dead() const {
		return false;
	}

	constexpr explicit operator bool() const {
		return true;
	}

	auto& get(cosmos_solvable_access) const {
		return subject;
	}

	const auto& get() const {
		return subject;
	}

	void ensure_alive() const {}
};

template <bool is_const, class entity_type, template <class> class identifier_provider>
class specific_entity_handle :
	public misc_mixin<specific_entity_handle<is_const, entity_type, identifier_provider>>,
	public inventory_mixin<specific_entity_handle<is_const, entity_type, identifier_provider>>,
	public physics_mixin<specific_entity_handle<is_const, entity_type, identifier_provider>>,
	public relations_mixin<specific_entity_handle<is_const, entity_type, identifier_provider>>,
	public spatial_properties_mixin<specific_entity_handle<is_const, entity_type, identifier_provider>>,
	public identifier_provider<specific_entity_handle<is_const, entity_type, identifier_provider>>
{
	using this_handle_type = specific_entity_handle<is_const, entity_type, identifier_provider>;
	using used_identifier_provider = identifier_provider<this_handle_type>;

	using misc_base = misc_mixin<this_handle_type>;

	using const_handle_type = specific_entity_handle<true, entity_type, identifier_provider>;

	using subject_type = entity_solvable<entity_type>;

	using owner_reference = maybe_const_ref_t<is_const, cosmos>;

	// cosmos shall only be able to construct directly
	friend class cosmos;
	friend specific_entity_handle<!is_const, entity_type, identifier_provider>;
	friend used_identifier_provider;

	using used_identifier_provider::get_subject;
	using used_identifier_provider::find_subject;

public:
	using const_type = specific_entity_handle<true, entity_type, identifier_provider>;
	using misc_base::get_flavour;
	using used_identifier_provider::get_id;
	using used_identifier_provider::get;
	using used_identifier_provider::alive;
	using used_identifier_provider::dead;
	using used_identifier_provider::ensure_alive;
	using used_identifier_provider::operator bool;
	using used_entity_type = entity_type;


private:
	owner_reference owner;

	template <class T>
	maybe_const_ptr_t<is_const, T> find_component_ptr() const {
		if constexpr(subject_type::template has<T>()) {
			ensure_alive();

			return std::addressof(get_subject().template get<T>());
		}

		return nullptr;
	}

	template <bool>
	friend class basic_entity_handle;

public:
	specific_entity_handle(
		owner_reference owner,
		const used_identifier_provider identifier
	) :
		used_identifier_provider(identifier),
		owner(owner)
	{}

	const auto& get_meta() const {
		ensure_alive();
		return static_cast<const entity_solvable_meta&>(get_subject());
	};

	const auto& get_pool() const {
		return get_cosmos().get_solvable().significant.template get_pool<entity_type>();
	}

	template <class T>
	static constexpr bool has() {
		return 
			subject_type::template has<T>() 
			|| entity_flavour<entity_type>::template has<T>()
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
			static_assert(has<T>());

			if constexpr(is_synchronized_v<T>) {
				return find<T>();
			}
			else {
				return *find<T>();
			}
		}
	}

	auto& get_cosmos() const {
		return owner;
	}

	bool operator==(const this_handle_type& h) const {
		return this->get_id() == h.get_id();
	}

	bool operator==(const specific_entity_handle<!is_const, entity_type, identifier_provider>& h) const {
		return this->get_id() == h.get_id();
	}

	bool operator==(const entity_id id) const {
		return get_id() == id;
	}

	bool operator!=(const entity_id id) const {
		return !operator==(id);
	}

	/* Return a handle with a reference instead of a pointer */
	template <class T = used_identifier_provider, class = std::enable_if_t<std::is_same_v<T, stored_id_provider<this_handle_type>>>>
	specific_entity_handle<is_const, entity_type, ref_stored_id_provider> operator*() const {
		ensure_alive();
		return { owner, { *this->subject, this->stored_id } };
	}

	auto get_type_id() const {
		return entity_type_id::of<entity_type>();
	}

	operator entity_id() const {
		return get_id();
	}

	operator child_entity_id() const {
		return get_id();
	}

	operator unversioned_entity_id() const {
		return entity_id(get_id()).operator unversioned_entity_id();
	}

	template <class F>
	void for_each_component(F&& callback) const {
		ensure_alive();
		const auto& immutable_subject = get_subject();

		for_each_through_std_get(
			immutable_subject.components, 
			std::forward<F>(callback)
		);
	}

	/* For compatibility with the general handle */
	template <class List, class F>
	void conditional_dispatch(F callback) const {
		if constexpr(num_types_in_list_v<List> > 0) {
			if constexpr(is_one_of_list_v<entity_type, List>) {
				callback(*this);
			}
		}

		(void)callback;
	}

	template <class... List, class F>
	void dispatch_on_having(F&& callback) const {
		conditional_dispatch<entity_types_having_all_of<List...>>(std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) dispatch(F callback) const {
		return callback(*this);
	}

	template <bool C>
	operator basic_entity_handle<C>() const {
		if constexpr(is_const && !C) {
			static_assert(always_false_v<entity_type>, "Can't convert to a non-const generic handle");
		}

		using void_entity_ptr = maybe_const_ptr_t<is_const, void>;
		return { static_cast<void_entity_ptr>(find_subject()), owner, get_id() };
	}
};

template <bool is_const, class entity_type, template <class> class identifier_provider>
std::ostream& operator<<(
	std::ostream& out,
   	const specific_entity_handle<is_const, entity_type, identifier_provider> x
) {
	return out << typesafe_sprintf("%x-%x", x.get_name(), x.get_id());
}
