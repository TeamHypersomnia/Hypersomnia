#pragma once
#include <unordered_map>
#include "../assets/animation.h"
#include "../globals/animations.h"

namespace resources {
	typedef std::unordered_map<animation_response_type, assets::animation_id> animation_response;
}
