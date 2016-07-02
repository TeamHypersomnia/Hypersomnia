#pragma once
#include "math/vec2.h"
#include <Box2D/Box2D.h>

#include "physics_setup_helpers.h"
#include "game/components/physics_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/polygon_component.h"
#include "game/components/render_component.h"
#include "game/components/sprite_component.h"
#include "game/detail/state_for_drawing_camera.h"

#include "game/stateful_systems/physics_system.h"

#include "game/cosmos.h"

#include "3rdparty/polypartition/polypartition.h"

#include "ensure.h"

#include "texture_baker/texture_baker.h"
#include "game/resources/manager.h"

