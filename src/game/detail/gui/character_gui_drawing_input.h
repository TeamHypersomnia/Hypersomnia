#pragma once
#include "augs/misc/basic_input_context.h"

#include "augs/math/camera_cone.h"
#include "augs/drawing/drawing.h"

#include "game/enums/input_context_enums.h"
#include "game/detail/gui/hotbar_settings.h"

class interpolation_system;
struct aabb_highlighter;

struct viewing_game_gui_context_dependencies {
	const interpolation_system& interpolation;
	const aabb_highlighter& world_hover_highlighter;
	const hotbar_settings hotbar;
	const double interpolation_ratio = 0.0;
	const input_context input_information;
	const camera_cone camera;
	const augs::drawer_with_default output;
};