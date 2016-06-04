#pragma once
#include <unordered_map>
#include "game/assets/animation_id.h"
#include "game/globals/animation_response_type.h"

namespace resources {
	typedef std::unordered_map<animation_response_type, assets::animation_id> animation_response;
}
