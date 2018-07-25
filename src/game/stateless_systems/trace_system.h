#pragma once

#include "game/cosmos/step_declaration.h"

class trace_system {
public:
	void lengthen_sprites_of_traces(const logic_step) const;
	void destroy_outdated_traces(const logic_step) const;
	void spawn_finishing_traces_for_deleted_entities(const logic_step) const;
};