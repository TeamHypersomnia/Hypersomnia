#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"

template <class Character>
void move_character_out_of_every_frame(const Character& character) {
	const auto far_away_offset = vec2(100000, 100000);
	const auto character_transform = character.get_logic_transform();
	character.set_logic_transform(transformr(character_transform.pos + far_away_offset, character_transform.rotation));
}
