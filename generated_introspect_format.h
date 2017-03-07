#include "game/transcendental/types_specification/all_components_declaration.h"
#include "game/components/behaviour_tree_component.h"
#include "game/components/car_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/light_component.h"
#include "game/components/movement_component.h"
#include "game/components/sound_existence_component.h"
#include "game/components/particles_existence_component.h"
#include "game/components/special_physics_component.h"
#include "augs/audio/sound_effect_modifier.h"
#include "augs/image/image.h"

/*
	This file was generated with use of Introspector-generator available at:
	https://github.com/TeamHypersomnia/Introspector-generator
	Please use that tool whenever you add or remove new fields to objects that need to be introspected.
*/

class config_lua_table;
class recoil_player;

#define NVP(x) x, #x

namespace resources {
	struct particle_effect_modifier;
	struct emission;
}

namespace augs {
%x}