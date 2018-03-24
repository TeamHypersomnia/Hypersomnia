#include "rigid_body_component.h"

#include <Box2D/Box2D.h>

#include "fixtures_component.h"

#include "augs/math/vec2.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "augs/ensure.h"
#include "game/debug_drawing_settings.h"

components::rigid_body::rigid_body(
	const si_scaling si,
	const components::transform t
) {
	set_transform(si, t);
}

void components::rigid_body::set_transform(
	const si_scaling si,
	const components::transform& t
) {
	t.to_si_space(si).to_physics_engine_transforms(physics_transforms);
}
