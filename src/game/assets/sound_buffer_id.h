#pragma once
#include "game/container_sizes.h"
#include "augs/audio/sound_effect_modifier.h"

namespace assets {
	enum class sound_buffer_id {
		// GEN INTROSPECTOR enum class assets::sound_buffer_id
		INVALID,

#if BUILD_TEST_SCENES
		BILMER2000_MUZZLE,
		ASSAULT_RIFLE_MUZZLE,
		SUBMACHINE_MUZZLE,
		SN69_MUZZLE,
		KEK9_MUZZLE,
		ROCKET_LAUNCHER_MUZZLE,
		ELECTRIC_PROJECTILE_FLIGHT,
		ELECTRIC_DISCHARGE_EXPLOSION,
		MISSILE_THRUSTER,

		IMPACT,
		DEATH,
		BULLET_PASSES_THROUGH_HELD_ITEM,

		WIND,
		ENGINE,

		LOW_AMMO_CUE,
		FIREARM_ENGINE,

		CAST_SUCCESSFUL,
		CAST_UNSUCCESSFUL,

		CAST_CHARGING,

		EXPLOSION,
		GREAT_EXPLOSION,

		INTERFERENCE_EXPLOSION,
		PED_EXPLOSION,

		GRENADE_UNPIN,
		GRENADE_THROW,

		ITEM_THROW,

		COLLISION_METAL_WOOD,
		COLLISION_METAL_METAL,
		COLLISION_GRENADE,
#endif
		COUNT = MAX_SOUND_BUFFER_COUNT + 1
		// END GEN INTROSPECTOR
	};
}

struct sound_effect_input {
	// GEN INTROSPECTOR struct sound_effect_input
	assets::sound_buffer_id id = assets::sound_buffer_id::INVALID;
	augs::sound_effect_modifier modifier;
	// END GEN INTROSPECTOR
};