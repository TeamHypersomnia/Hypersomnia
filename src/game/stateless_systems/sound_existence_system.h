#pragma once
#include "game/cosmos/entity_handle_declaration.h"
#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_id.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/components/transform_component.h"

class sound_existence_system {
public:
	void play_sounds_from_events(const logic_step) const;
};