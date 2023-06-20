#pragma once
#include <stdexcept>

enum class entity_creation_error_type {
	// GEN INTROSPECTOR enum class entity_creation_error_type
	POOL_FULL,
	DEAD_FLAVOUR,
	// END GEN INTROSPECTOR
	COUNT
};

struct entity_creation_error : std::runtime_error {
	static auto desc(entity_creation_error_type type) {
		if (type == entity_creation_error_type::POOL_FULL) {
			return "Too many entities!";
		}

		return "Invalid flavour id!";
	}

	entity_creation_error_type type;
	entity_creation_error(const entity_creation_error_type type) : std::runtime_error(desc(type)), type(type) {}
};
