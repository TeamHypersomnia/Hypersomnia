#pragma once
#include "augs/misc/basic_input_context.h"
#include "augs/math/camera_cone.h"

#include "game/enums/input_context_enums.h"

#include "game/detail/visible_entities.h"
#include "game/detail/gui/hotbar_settings.h"

#include "game/transcendental/entity_id.h"
#include "game/view/game_drawing_settings.h"

class cosmos;
struct audiovisual_state;

namespace augs {
	class renderer;
}

struct necessary_shaders;
struct necessary_fbos;

/* Require all */

using illuminated_rendering_fbos = necessary_fbos;
using illuminated_rendering_shaders = necessary_shaders;

struct illuminated_rendering_input {
	const cosmos& cosm;
	const audiovisual_state& audiovisuals;
	const game_drawing_settings drawing;
	const necessary_images_in_atlas& necessary_images;
	const augs::baked_font& gui_font;
	const game_images_in_atlas& game_images;
	const vec2i screen_size;
	const hotbar_settings hotbar;
	const input_context& input_information;
	const double interpolation_ratio = 0.0;
	augs::renderer& renderer;
	const augs::graphics::texture& game_world_atlas;
	const illuminated_rendering_fbos& fbos;
	const illuminated_rendering_shaders& shaders;
	const camera_cone camera;
	const entity_id viewed_character;
	const visible_entities& visible;
};

void illuminated_rendering(const illuminated_rendering_input);
