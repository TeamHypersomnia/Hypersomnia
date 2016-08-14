#pragma once
#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"

#include "game/enums/entity_name.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

namespace components {
	struct name {
		typedef augs::constant_size_vector<wchar_t, NICKNAME_LENGTH> nickname_type;
		static_assert(nickname_type::array_size % 4 == 0, "Wrong nickname padding");

		entity_name id;

		int custom_nickname = false;
		nickname_type nickname;

		std::wstring get_nickname() const;
		void set_nickname(std::wstring);
		
		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(id),

				CEREAL_NVP(custom_nickname),
				CEREAL_NVP(nickname)
			);
		}
	};
}

entity_id get_first_named_ancestor(const_entity_handle);
