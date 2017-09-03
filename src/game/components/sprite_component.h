#pragma once
#include "augs/pad_bytes.h"

#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "augs/graphics/rgba.h"
#include "augs/graphics/vertex.h"

#include "augs/drawing/drawing.h"
#include "augs/drawing/sprite.h"

#include "game/assets/game_image_id.h"

namespace components {
	using sprite = augs::sprite<assets::game_image_id>;
}