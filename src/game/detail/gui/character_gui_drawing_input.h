#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/detail/gui/hotbar_settings.h"
#include "game/enums/input_context_enums.h"
#include "game/detail/camera_cone.h"
#include "augs/graphics/vertex.h"
#include "augs/misc/basic_input_context.h"

class gui_element_system;
class interpolation_system;
struct aabb_highlighter;

struct character_gui_drawing_input {
	const gui_element_system& gui;
	const interpolation_system& interpolation;
	const const_entity_handle viewed_character;
	const aabb_highlighter& world_hover_highlighter;
	const hotbar_settings hotbar;
	const double interpolation_ratio = 0.0;
	const input_context input_information;
	const camera_cone camera;
	
	augs::vertex_triangle_buffer& output;
};