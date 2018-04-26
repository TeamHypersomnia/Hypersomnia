#pragma once
#include <string>
#include "augs/templates/hash_templates.h"
#include "augs/string/string_templates_declaration.h"
#include "augs/string/get_current_type_name.h"

#include "game/transcendental/entity_type_traits.h"
#include "game/organization/all_entity_types_declaration.h"

struct raw_flavour_id_key {};

using raw_entity_flavour_id = augs::pooled_object_id<unsigned short, raw_flavour_id_key>;

template <class...>
struct constrained_entity_flavour_id; 

using entity_flavour_id = constrained_entity_flavour_id<>;

template <class... C>
struct constrained_entity_flavour_id {
	using matching_types = entity_types_having_all_of<C...>;

	// GEN INTROSPECTOR struct constrained_entity_flavour_id class... C
	raw_entity_flavour_id raw;
	entity_type_id type_id;
	// END GEN INTROSPECTOR

	explicit operator bool() const {
		return raw.is_set();
	}

	bool operator==(const constrained_entity_flavour_id<C...> b) const {
		return raw == b.raw && type_id == b.type_id; 
	}

	bool operator!=(const constrained_entity_flavour_id<C...> b) const {
		return !operator==(b);
	}

	operator entity_flavour_id() const {
		return { raw, type_id };
	}
};

template <class... C>
std::ostream& operator<<(std::ostream& out, const constrained_entity_flavour_id<C...> x) {
	return out << "(" << get_current_type_name(x.type_id) << ": " << x.raw << ")";
}

template <class E>
struct typed_entity_flavour_id {
	raw_entity_flavour_id raw;

	typed_entity_flavour_id() = default;
	explicit typed_entity_flavour_id(const raw_entity_flavour_id raw) : raw(raw) {};

	explicit operator bool() const {
		return raw.is_set();
	}

	bool operator==(const typed_entity_flavour_id<E> b) const {
		return raw == b.raw;
	}

	bool operator!=(const typed_entity_flavour_id<E> b) const {
		return !operator==(b);
	}

	template <
		class... C,
		class V = std::enable_if_t<has_all_of_v<E, C...>>
	>
	operator constrained_entity_flavour_id<C...>() const {
		return { raw, entity_type_id::of<E>() };
	}
};

template <class T>
std::ostream& operator<<(std::ostream& out, const typed_entity_flavour_id<T> x) {
	return out << "(" << get_type_name<T>() << ": " << x.raw << ")";
}

namespace std {
	template <>
	struct hash<entity_flavour_id> {
		std::size_t operator()(const entity_flavour_id v) const {
			return augs::simple_two_hash(v.raw, v.type_id);
		}
	};

	template <class T>
	struct hash<typed_entity_flavour_id<T>> {
		std::size_t operator()(const typed_entity_flavour_id<T> v) const {
			return augs::simple_two_hash(typeid(T).hash_code(), v.raw);
		}
	};
}
