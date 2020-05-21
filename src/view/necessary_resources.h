#pragma once
#include <optional>

#include "augs/graphics/fbo.h"
#include "augs/graphics/texture.h"
#include "augs/graphics/shader.h"
#include "augs/math/vec2.h"
#include "augs/misc/enum/enum_array.h"
#include "augs/audio/sound_buffer.h"

#include "augs/misc/enum/enum_map.h"
#include "augs/templates/exception_templates.h"

#include "view/viewables/image_definition.h"
#include "view/necessary_image_id.h"

/*
	Collections of resources that are necessary for the game to function properly,
	regardless of whether there are active game worlds (cosmoi) or just some GUI.

	Never needed by the logic, only the view.
	
	Depending on game drawing settings, some graphics resources (fbos, shaders) might be destroyed,
	thus they are held via std::optional.


	If somethinig is absolutely necessary for the game to function,
	and it fails to load, it will throw necessary_resource_loading_error.
*/

struct necessary_resource_loading_error : error_with_typesafe_sprintf {
	using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
};

struct game_drawing_settings;

using optional_fbo = std::optional<augs::graphics::fbo>;
using optional_shader = std::optional<augs::graphics::shader_program>;
 
namespace augs {
	class renderer;
};

struct all_necessary_fbos {
	optional_fbo illuminating_smoke;
	optional_fbo smoke;
	optional_fbo light;
	optional_fbo flash_afterimage;

	all_necessary_fbos(
		const vec2i screen_size, 
		const game_drawing_settings
	);

	void apply(
		const vec2i screen_size
	);
};

struct all_necessary_shaders {
	// GEN INTROSPECTOR struct all_necessary_shaders
	optional_shader standard;
	optional_shader illuminated;
	optional_shader pure_color_highlight;
	optional_shader fog_of_war;
	optional_shader circular_bars;
	optional_shader smoke;
	optional_shader illuminating_smoke;
	optional_shader exploding_rings;
	optional_shader light;
	optional_shader textured_light;
	optional_shader flash_afterimage;
	optional_shader neon_occluder;
	// END GEN INTROSPECTOR

	all_necessary_shaders(
		augs::renderer& in,
		const augs::path_type& canon_directory,
		const augs::path_type& local_directory,
		const game_drawing_settings
	);
};

struct all_necessary_sounds {
	augs::single_sound_buffer button_click;
	augs::single_sound_buffer button_hover;
	augs::single_sound_buffer round_clock_tick;
	augs::single_sound_buffer alarm_tick;

	all_necessary_sounds(const augs::path_type& directory);
};

namespace sol {
	class state;
}

struct necessary_image_definitions_map :
	public augs::enum_map<assets::necessary_image_id, image_definition>
{
	necessary_image_definitions_map(
		sol::state& lua,
		const augs::path_type& directory,
		const bool force_regenerate
	);
};

using necessary_images_in_atlas_map = augs::enum_array<
	/* necessary images have only diffuse maps, thus no need for neon/desaturation entries */
	augs::atlas_entry,
	assets::necessary_image_id
>;