#pragma once
#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"

#include "game/enums/entity_name.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "zeroed_pod.h"

namespace components {
	struct name {
		typedef augs::constant_size_wstring<NICKNAME_LENGTH> nickname_type;
		static_assert(nickname_type::array_size % 4 == 0, "Wrong nickname padding");

		// GEN INTROSPECTOR components::name
		entity_name id = entity_name::INVALID;

		int custom_nickname = false;
		nickname_type nickname;
		// END GEN INTROSPECTOR

		std::wstring get_nickname() const;
		void set_nickname(const std::wstring&);
	};
}

entity_id get_first_named_ancestor(const const_entity_handle);
