#pragma once
#include "game/entity_handle_declaration.h"

class variable_step;

namespace rendering_scripts {
	void standard_rendering(variable_step& step, const_entity_handle camera);
}