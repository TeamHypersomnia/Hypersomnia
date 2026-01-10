#pragma once
#if BUILD_TEST_SCENES
#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"

#include "game/assets/ids/asset_ids.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_handle.h"

#include "game/components/render_component.h"
#include "game/components/sprite_component.h"

#include "test_scenes/test_scene_images.h"
#include "test_scenes/test_scene_sounds.h"
#include "test_scenes/test_scene_flavours.h"
#include "test_scenes/test_scene_physical_materials.h"
#include "test_scenes/test_scene_particle_effects.h"
#include "test_scenes/test_scene_recoil_players.h"

#include "test_scenes/ingredients/box_physics.h"
#include "test_scenes/ingredients/add_sprite.h"

#include "view/viewables/image_cache.h"

namespace ingredients {

}

namespace prefabs {
	entity_handle create_car(const logic_step, const transformr&);
}
#endif