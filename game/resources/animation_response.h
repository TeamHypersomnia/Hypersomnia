#pragma once
#include <unordered_map>
#include "../assets/animation_id.h"
#include "../globals/animations.h"

namespace resources {
	typedef std::unordered_map<animation_response_type, assets::animation_id> animation_response;
}
