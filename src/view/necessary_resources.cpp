#include "augs/string/string_templates.h"
#include "augs/templates/container_templates.h"
#include "augs/templates/enum_introspect.h"
#include "augs/filesystem/file.h"
#include "augs/audio/sound_data.h"
#include "augs/gui/button_corners.h"
#include "augs/log.h"

#include "view/game_drawing_settings.h"

#include "view/necessary_resources.h"

#include "view/viewables/regeneration/images_from_commands.h"
#include "view/viewables/regeneration/procedural_image_definition.h"

#include "augs/readwrite/lua_file.h"
#include "augs/graphics/renderer_command.h"
#include "augs/graphics/renderer.h"
#include "augs/graphics/shader.hpp"

all_necessary_fbos::all_necessary_fbos(
	const vec2i screen_size,
	const game_drawing_settings settings
) {
	(void)settings;
	apply(screen_size);
}

void all_necessary_fbos::apply(
	const vec2i screen_size
) {
	if (/* just_minimized */ screen_size.is_zero()) {
		return;
	}

	auto reset = [screen_size](auto& fbo, auto... args) {
		if (!fbo || fbo->get_size() != static_cast<vec2u>(screen_size)) {
			fbo.emplace(screen_size, augs::graphics::fbo_opts { args... });
		}
	};

	reset(illuminating_smoke);
	reset(smoke);
	reset(light, augs::graphics::fbo_opt::WITH_STENCIL);
	reset(flash_afterimage);
}

all_necessary_shaders::all_necessary_shaders(
	augs::renderer& renderer,
	const augs::path_type& canon_directory,
	const augs::path_type& local_directory,
	const game_drawing_settings /* settings */
) try {
	augs::introspect(
		[&](const auto& label, auto& shader) {
			const auto canon_vsh_path = typesafe_sprintf("%x/%x.vsh", canon_directory, label);
			const auto local_vsh_path = typesafe_sprintf("%x/%x.vsh", local_directory, label);

			const auto final_vsh_path = augs::switch_path(
				canon_vsh_path,
				local_vsh_path
			);

			const auto final_fsh_path = augs::path_type(final_vsh_path).replace_extension(".fsh");
			
			try {
				shader.emplace(
					final_vsh_path,
					final_fsh_path
				);
			}
			catch (const augs::graphics::shader_error& err) {
				LOG(
					"There was a problem building %x.\n That shader was not set.",
					label
				);

				LOG(err.what());
				throw;
			}
		},
		*this
	);

	using U = augs::common_uniform_name;

	if (illuminated) {
		illuminated->set_as_current(renderer);
		illuminated->set_uniform(renderer, U::basic_texture, 0);
		illuminated->set_uniform(renderer, U::light_texture, 2);
	}

	if (standard) {
		standard->set_as_current(renderer);
		standard->set_uniform(renderer, U::basic_texture, 0);
	}

	if (pure_color_highlight) {
		pure_color_highlight->set_as_current(renderer);
		pure_color_highlight->set_uniform(renderer, U::basic_texture, 0);
	}

	if (circular_bars) {
		circular_bars->set_as_current(renderer);
		circular_bars->set_uniform(renderer, U::basic_texture, 0);
	}

	if (smoke) {
		smoke->set_as_current(renderer);
		smoke->set_uniform(renderer, U::smoke_texture, 1);
		smoke->set_uniform(renderer, U::light_texture, 2);
	}

	if (illuminating_smoke) {
		illuminating_smoke->set_as_current(renderer);
		illuminating_smoke->set_uniform(renderer, U::smoke_texture, 3);
	}

	if (textured_light) {
		textured_light->set_as_current(renderer);
		textured_light->set_uniform(renderer, U::basic_texture, 0);
	}

	if (flash_afterimage) {
		flash_afterimage->set_as_current(renderer);
		flash_afterimage->set_uniform(renderer, U::afterimage_texture, 0);
	}

	if (neon_occluder) {
		neon_occluder->set_as_current(renderer);
		neon_occluder->set_uniform(renderer, U::basic_texture, 0);
	}
} 
catch (const augs::graphics::shader_compilation_error& err) {
	throw necessary_resource_loading_error("Failed to compile a necessary shader. Details: %x", err.what());
}
catch (const augs::graphics::shader_program_build_error& err) {
	throw necessary_resource_loading_error("Failed to link a necessary shader. Details: %x", err.what());
}
catch (const augs::graphics::shader_error& err) {
	throw necessary_resource_loading_error("Failed to load a necessary shader. Details: %x", err.what());
}

all_necessary_sounds::all_necessary_sounds(
	const augs::path_type& directory
) try :
	button_click(augs::sound_data(typesafe_sprintf("%x/button_click.wav", directory))),
	button_hover(augs::sound_data(typesafe_sprintf("%x/button_hover.wav", directory))),
	round_clock_tick(augs::sound_data(typesafe_sprintf("%x/round_clock_tick.wav", directory))),
	alarm_tick(augs::sound_data(typesafe_sprintf("%x/alarm_tick.wav", directory)))
{}
catch (const augs::sound_decoding_error& err) {
	throw necessary_resource_loading_error(err.what());
}

augs::path_type get_procedural_image_path(const augs::path_type& from_source_path) {
	return augs::path_type(GENERATED_FILES_DIR) / from_source_path;
}

necessary_image_definitions_map::necessary_image_definitions_map(
	sol::state& lua,
	const augs::path_type& directory,
	const bool force_regenerate
) {
	using id_type = assets::necessary_image_id;

	augs::for_each_enum_except_bounds([&](const id_type id) {
		if (found_in(*this, id)) {
			return;
		}

		const auto stem = to_lowercase(augs::enum_to_string(id));

		image_definition definition_template;
		auto& source_image = definition_template.source_image;

		if (
			const auto additional_properties_path = typesafe_sprintf("%x/%x.lua", directory, stem);
			augs::exists(additional_properties_path)
		) {
			try {
				augs::load_from_lua_table(
					lua,
					definition_template,
					additional_properties_path
				);
			}
			catch (const augs::lua_deserialization_error& err) {
				throw necessary_resource_loading_error(
					"Failed to load additional properties for %x (%x).\nNot a valid lua table.\n%x",
					stem, 
					additional_properties_path,
					err.what()
				);
			}
			catch (const augs::file_open_error& err) {
				throw necessary_resource_loading_error(
					"Failed to load additional properties for %x (%x).\nFile might be corrupt.\n%x",
					stem,
					additional_properties_path,
					err.what()
				);
			}
		}

		if (const auto image_path = typesafe_sprintf("%x/%x.png", directory, stem);
			augs::exists(image_path)
		) {
			source_image.path = image_path;
			emplace(id, definition_template);
		}
		else if (
			const auto procedural_definition_path = typesafe_sprintf("%x/procedural/%x.lua", directory, stem);
			augs::exists(procedural_definition_path)
		) {
			procedural_image_definition def;

			try {
				augs::load_from_lua_table(lua, def, procedural_definition_path);
			}
			catch (const augs::lua_deserialization_error& err) {
				throw necessary_resource_loading_error(
					"Failed to load procedural image definition for %x (%x).\nNot a valid lua table.\n%x",
					stem,
					procedural_definition_path,
					err.what()
				);
			}
			catch (const augs::file_open_error& err) {
				throw necessary_resource_loading_error(
					"Failed to load procedural image definition for %x (%x).\nFile might be corrupt.\n%x",
					stem,
					procedural_definition_path,
					err.what()
				);
			}
			
			if (
				const bool exactly_one = def.button_with_corners.has_value() != def.image_from_commands.has_value();
				!exactly_one
			) {
				throw necessary_resource_loading_error(
					"Failed to load procedural image definition for %x (%x):\n%x",
					stem,
					procedural_definition_path,
					"Either none or more than one type of procedural image have been specified."
				);
			}

			if (def.button_with_corners) {
				const auto path_template = get_procedural_image_path(
					typesafe_sprintf("%x/procedural/%x_%x.png", directory, stem)
				);
				
				const auto input = def.button_with_corners.value();
		
				regenerate_button_with_corners(path_template, input, force_regenerate);
		
				const auto first = id;

				augs::for_each_enum_except_bounds([&](const button_corner_type type) {
					source_image.path = typesafe_sprintf(path_template.string(), get_filename_for(type));
					
					emplace(
						static_cast<id_type>(
							static_cast<int>(first) + static_cast<int>(type)
						), 
						definition_template
					);
				});
			}
			else if (def.image_from_commands) {
				const auto generated_image_path = get_procedural_image_path(
					typesafe_sprintf("%x/procedural/%x.png", directory, stem)
				);
		
				regenerate_image_from_commands(
					generated_image_path,
					def.image_from_commands.value(),
					force_regenerate
				);
		
				source_image.path = generated_image_path;
				emplace(id, definition_template);
			}
		}

		if (/* nothing_loaded */ source_image.path.empty()) {
			throw necessary_resource_loading_error(
				"Failed to load necessary image: %x.\n%x",
				stem,
				"No source image exists, nor does a procedural definition."
			);
		}
	});
}
