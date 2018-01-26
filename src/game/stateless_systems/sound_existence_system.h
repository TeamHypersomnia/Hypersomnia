#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_id.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/components/transform_component.h"

class sound_existence_system {
public:
	void create_sounds_from_game_events(const logic_step) const;
};