#pragma once
#include "game/globals/entity_name.h"

namespace components {
	struct name {
		entity_name id;

		bool custom_nickname = false;
		std::wstring nickname;
	};
}

entity_id get_first_named_ancestor(entity_id);
