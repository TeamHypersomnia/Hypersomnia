#pragma once
#include <unordered_map>
#include "../assets/animation.h"
#include "../messages/animation_response_message.h"

namespace resources {
	typedef std::unordered_map<messages::animation_response_message::response, assets::animation_id> animation_response;
}
