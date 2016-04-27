#pragma once
#include "../globals/entity_name.h"

namespace components {
	struct name {
		entity_name id;

		bool custom_nickname = false;
		std::wstring nickname;
	};
}

augs::entity_id get_first_named_ancestor(augs::entity_id);