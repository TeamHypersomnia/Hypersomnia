#pragma once
#include "augs/misc/enum_associative_array.h"
#include "game/enums/physical_material_type.h"
#include "game/assets/sound_buffer_id.h"

typedef augs::enum_associative_array<
	physical_material_type,
	augs::enum_associative_array<physical_material_type, assets::sound_buffer_id>
> collision_sound_matrix_type;