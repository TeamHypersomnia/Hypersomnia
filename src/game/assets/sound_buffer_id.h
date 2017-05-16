#pragma once
#include "game/container_sizes.h"
#include "game/build_settings.h"
#include "augs/audio/sound_effect_modifier.h"

namespace assets {
	enum class sound_buffer_id : unsigned {
		INVALID,
#if BUILD_UNSCRIPTED_TEST_SCENES
		BILMER2000_MUZZLE,
		ASSAULT_RIFLE_MUZZLE,
		SUBMACHINE_MUZZLE,
		PISTOL_MUZZLE,
		KEK9_MUZZLE,
		ROCKET_LAUNCHER_MUZZLE,
		ELECTRIC_PROJECTILE_FLIGHT,
		ELECTRIC_DISCHARGE_EXPLOSION,
		ELECTRIC_ENGINE,
		MISSILE_THRUSTER,

		IMPACT,
		DEATH,
		BULLET_PASSES_THROUGH_HELD_ITEM,

		WIND,
		ENGINE,

		BUTTON_HOVER,
		BUTTON_CLICK,

		LOW_AMMO_CUE,
		FIREARM_ENGINE,

		CAST_SUCCESSFUL,
		CAST_UNSUCCESSFUL,

		CAST_CHARGING,

		EXPLOSION,
		GREAT_EXPLOSION,

		INTERFERENCE_EXPLOSION,
		PED_EXPLOSION,

		ZAP,

		GRENADE_UNPIN,
		GRENADE_THROW,

		ITEM_THROW,

		COLLISION_METAL_WOOD,
		COLLISION_METAL_METAL,
		COLLISION_GRENADE,
#endif
		COUNT = MAX_SOUND_BUFFER_COUNT + 1
	};
}

struct sound_response {
	// GEN INTROSPECTOR struct sound_response
	assets::sound_buffer_id id = assets::sound_buffer_id::COUNT;
	augs::sound_effect_modifier modifier;
	// END GEN INTROSPECTOR
};