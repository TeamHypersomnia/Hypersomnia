#pragma once
#include <string>
#include "augs/templates/hash_templates.h"
#include "augs/string/string_templates_declaration.h"
#include "augs/string/get_current_type_name.h"

#include "game/cosmos/entity_type_traits.h"
#include "game/organization/all_entity_types_declaration.h"

struct raw_flavour_id_key {};

using raw_entity_flavour_id = augs::pooled_object_id<unsigned short, raw_flavour_id_key>;

template <class...>
struct constrained_entity_flavour_id; 

using entity_flavour_id = constrained_entity_flavour_id<>;

template <class...>
struct all_or_some;

template <class A, class... Rest>
struct all_or_some<A, Rest...> {
	using type = entity_types_having_all_of<A, Rest...>;
};

template <>
struct all_or_some<> {
	using type = all_entity_types;
};

template <class... C>
struct constrained_entity_flavour_id {
	using matching_types = typename all_or_some<C...>::type;

	// GEN INTROSPECTOR struct constrained_entity_flavour_id class... C
	raw_entity_flavour_id raw;
	entity_type_id type_id;
	// END GEN INTROSPECTOR

	bool is_set() const {
		return raw.is_set() && type_id.is_set();
	}

	bool operator==(const constrained_entity_flavour_id<C...>& b) const noexcept = default;

	operator entity_flavour_id() const {
		return { raw, type_id };
	}

	template <class F>
	decltype(auto) dispatch(F) const;

	auto hash() const {
		return augs::hash_multiple(raw, type_id);
	}
};

template <class... C>
std::ostream& operator<<(std::ostream& out, const constrained_entity_flavour_id<C...> x) {
	if (!x.is_set()) {
		return out << "(unset)";
	}

	return out << "(" << get_current_type_name(x.type_id) << ": " << x.raw << ")";
}

template <class E>
struct typed_entity_flavour_id {
	using used_entity_type = E;

	// GEN INTROSPECTOR struct typed_entity_flavour_id class E
	raw_entity_flavour_id raw;
	// END GEN INTROSPECTOR

	typed_entity_flavour_id() = default;
	explicit typed_entity_flavour_id(const raw_entity_flavour_id raw) : raw(raw) {};

	bool is_set() const {
		return raw.is_set();
	}

	bool operator==(const typed_entity_flavour_id<E> b) const {
		return raw == b.raw;
	}

	bool operator!=(const typed_entity_flavour_id<E> b) const {
		return !operator==(b);
	}

	template <
		class C,
		class... CR,
		class V = std::enable_if_t<has_all_of_v<E, C, CR...>>
	>
	operator constrained_entity_flavour_id<C, CR...>() const {
		return { raw, entity_type_id::of<E>() };
	}

	operator entity_flavour_id() const {
		return { raw, entity_type_id::of<E>() };
	}

	auto hash() const {
		return augs::hash_multiple(entity_type_id::of<E>(), raw);
	}

};

template <class... C>
template <class F>
decltype(auto) constrained_entity_flavour_id<C...>::dispatch(F callback) const {
	using M = typename constrained_entity_flavour_id<C...>::matching_types;

	return type_id.constrained_dispatch<M>([&](auto e) {
		return callback(typed_entity_flavour_id<decltype(e)>(this->raw));
	});
}

template <class T>
struct is_constrained_flavour_id : std::false_type {};

template <class... Args>
struct is_constrained_flavour_id<constrained_entity_flavour_id<Args...>> : std::true_type {};

template <class T>
struct is_typed_flavour_id : std::false_type {};

template <class T>
struct is_typed_flavour_id<typed_entity_flavour_id<T>> : std::true_type {};

template <class T>
constexpr bool is_typed_flavour_id_v = is_typed_flavour_id<T>::value;

template <class T>
constexpr bool is_constrained_flavour_id_v = is_constrained_flavour_id<T>::value;

template <class T>
struct is_flavour_id {
	static constexpr bool value = is_constrained_flavour_id_v<T> || is_typed_flavour_id_v<T>;
};
	
template <class T>
constexpr bool is_flavour_id_v = is_flavour_id<T>::value;

template <class T>
std::ostream& operator<<(std::ostream& out, const typed_entity_flavour_id<T> x) {
	return out << "(" << get_type_name<T>() << ": " << x.raw << ")";
}

namespace std {
	template <class... C>
	struct hash<constrained_entity_flavour_id<C...>> {
		std::size_t operator()(const constrained_entity_flavour_id<C...>& v) const {
			return v.hash();
		}
	};

	template <class T>
	struct hash<typed_entity_flavour_id<T>> {
		std::size_t operator()(const typed_entity_flavour_id<T>& v) const {
			return v.hash(); 
		}
	};
}
