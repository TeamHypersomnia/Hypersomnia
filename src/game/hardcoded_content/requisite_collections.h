#pragma once
#include <optional>

#include "augs/graphics/fbo.h"
#include "augs/graphics/texture.h"
#include "augs/graphics/shader.h"
#include "augs/math/vec2.h"
#include "augs/audio/sound_buffer.h"

#include "augs/templates/exception_templates.h"

#include "game/assets/game_image.h"
#include "game/assets/game_image_id.h"

/*
	Collections of resources that are necessary for the game to function properly,
	regardless of whether there are active game worlds (cosmoi) or just some GUI.

	Never needed by the logic, only the view.
	
	Depending on game drawing settings, some graphics resources (fbos, shaders) might be destroyed,
	thus they are held via std::optional.


	If somethinig is absolutely necessary for the game to function,
	and it fails to load, it will throw requisite_resource_loading_error.
*/

struct requisite_resource_loading_error : error_with_typesafe_sprintf {
	using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
};

struct game_drawing_settings;

using optional_fbo = std::optional<augs::graphics::fbo>;
using optional_shader = std::optional<augs::graphics::shader_program>;

struct fbo_collection {
	optional_fbo illuminating_smoke;
	optional_fbo smoke;
	optional_fbo light;

	fbo_collection(
		const vec2i screen_size, 
		const game_drawing_settings
	);

	void apply(
		const vec2i screen_size,
		const game_drawing_settings
	);
};

struct shader_collection {
	// GEN INTROSPECTOR struct shader_collection
	optional_shader standard;
	optional_shader illuminated;
	optional_shader specular_highlights;
	optional_shader pure_color_highlight;
	optional_shader circular_bars;
	optional_shader smoke;
	optional_shader illuminating_smoke;
	optional_shader exploding_rings;
	optional_shader light;
	// END GEN INTROSPECTOR

	shader_collection(
		const augs::path_type& canon_directory,
		const augs::path_type& local_directory,
		const game_drawing_settings
	);
};

struct sound_buffer_collection {
	augs::single_sound_buffer button_click;
	augs::single_sound_buffer button_hover;

	sound_buffer_collection(const augs::path_type& directory);
};

struct requisite_image_collection {
	augs::enum_associative_array<assets::requisite_image_id, game_image_definition> all;

	requisite_image_collection(
		const augs::path_type& directory,
		const bool force_regenerate
	);
};

using requisite_images_in_atlas = augs::enum_associative_array<
	assets::requisite_image_id,
	/* requisite images have only diffuse maps, thus no need for neon/desaturation entries */
	augs::texture_atlas_entry
>;