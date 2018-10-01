#pragma once

enum class entity_creation_error_type {
	// GEN INTROSPECTOR enum class entity_creation_error_type
	POOL_FULL,
	DEAD_FLAVOUR
	// END GEN INTROSPECTOR
};

struct entity_creation_error {
	entity_creation_error_type type;
};
