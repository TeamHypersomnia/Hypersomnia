#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_id.h"
#include "game/components/sound_existence_component.h"
#include "game/components/transform_component.h"

class sound_existence_system {
public:
	void destroy_dead_sounds(const logic_step) const;
	void create_sounds_from_game_events(const logic_step) const;
};