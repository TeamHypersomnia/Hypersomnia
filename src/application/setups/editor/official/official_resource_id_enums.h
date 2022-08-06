#pragma once

// todo renames after we sort out the old maps:
// metropolis -> welcome_to_metropolis
// AIR_DUCT_COLLISION -> COLLISION_AIR_DUCT

enum class official_sprites {
	// GEN INTROSPECTOR enum class official_sprites
	ROAD,
	FLOOR,

	AWAKENING,
	WELCOME_TO_METROPOLIS,

	AQUARIUM_GLASS,

	COUNT
	// END GEN INTROSPECTOR
};

enum class official_sounds {
	// GEN INTROSPECTOR enum class official_sounds
	LOUDY_FAN,

	AQUARIUM_AMBIENCE_LEFT,
	AQUARIUM_AMBIENCE_RIGHT,

	GLASS_DAMAGE,
	COLLISION_GLASS,

	COUNT
	// END GEN INTROSPECTOR
};

enum class official_lights {
	// GEN INTROSPECTOR enum class official_lights
	STRONG_LAMP,
	AQUARIUM_LAMP,

	COUNT
	// END GEN INTROSPECTOR
};

enum class official_materials {
	// GEN INTROSPECTOR enum class official_materials
	GLASS,

	COUNT
	// END GEN INTROSPECTOR
};

enum class official_particles {
	// GEN INTROSPECTOR enum class official_particles
	GLASS_DAMAGE,

	COUNT
	// END GEN INTROSPECTOR
};
