#include "augs/templates/introspect.h"
#include "augs/filesystem/file.h"
#include "augs/audio/sound_data.h"
#include "augs/misc/lua_readwrite.h"
#include "augs/gui/button_corners.h"

#include "game/view/game_drawing_settings.h"

#include "game/hardcoded_content/requisite_collections.h"

#include "application/content_regeneration/procedural_image.h"

#include "generated/introspectors.h"

fbo_collection::fbo_collection(
	const vec2i screen_size,
	const game_drawing_settings settings
) {
	apply(screen_size, settings);
}

void fbo_collection::apply(
	const vec2i screen_size,
	const game_drawing_settings settings
) {
	auto reset = [screen_size](auto& fbo) {
		if (!fbo || fbo->get_size() != screen_size) {
			fbo.emplace(screen_size);
		}
	};

	reset(illuminating_smoke);
	reset(smoke);
	reset(light);
}

shader_collection::shader_collection(
	const augs::path_type& canon_directory,
	const augs::path_type& local_directory,
	const game_drawing_settings settings
) {
	augs::introspect(
		[&](const std::string label, auto& shader) {
			const auto canon_vsh_path = typesafe_sprintf("%x%x.vsh", canon_directory, label);
			const auto local_vsh_path = typesafe_sprintf("%x%x.vsh", local_directory, label);

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
			catch (augs::graphics::shader_error err) {
				LOG(
					"There was a problem building %x.\n That shader was not set.",
					label
				);

				LOG(err.what());
			}
		},
		*this
	);

	if (illuminated) {
		illuminated->set_as_current();
		illuminated->set_uniform("basic_texture", 0);
		illuminated->set_uniform("light_texture", 2);
	}

	if (standard) {
		standard->set_as_current();
		standard->set_uniform("basic_texture", 0);
	}

	if (pure_color_highlight) {
		pure_color_highlight->set_as_current();
		pure_color_highlight->set_uniform("basic_texture", 0);
	}

	if (circular_bars) {
		circular_bars->set_as_current();
		circular_bars->set_uniform("basic_texture", 0);
	}

	if (smoke) {
		smoke->set_as_current();
		smoke->set_uniform("smoke_texture", 1);
		smoke->set_uniform("light_texture", 2);
	}

	if (illuminating_smoke) {
		illuminating_smoke->set_as_current();
		illuminating_smoke->set_uniform("smoke_texture", 3);
	}

	if (specular_highlights) {
		specular_highlights->set_as_current();
		specular_highlights->set_uniform("basic_texture", 0);
		specular_highlights->set_uniform("light_texture", 2);
	}
}

sound_buffer_collection::sound_buffer_collection(
	const augs::path_type& directory
) try :
	button_click(augs::sound_data(typesafe_sprintf("%xbutton_click.wav", directory))),
	button_hover(augs::sound_data(typesafe_sprintf("%xbutton_hover.wav", directory)))
{}
catch (const augs::sound_decoding_error err) {
	throw requisite_resource_loading_error(err.what());
}

augs::path_type get_procedural_image_path(const augs::path_type& from_source_path) {
	return typesafe_sprintf("generated/%x", from_source_path);
}

requisite_image_collection::requisite_image_collection(
	const augs::path_type& directory,
	const bool force_regenerate
) {
	using id_type = assets::requisite_image_id;

	/* 
		This additional reference is only to mitigate MSVC bug 
		whereby there is some(?) problem capturing "this" contents in lambdas.
	*/

	auto& requisites = all;

	augs::for_each_enum_except_bounds<id_type>([&](const id_type id) {
		if (found_in(requisites, id)) {
			return;
		}

		const auto stem = to_lowercase(augs::enum_to_string(id));

		game_image_definition definition_template;

		if (
			const auto additional_properties_path = typesafe_sprintf("%x%x.lua", directory, stem);
			augs::file_exists(additional_properties_path)
		) {
			try {
				augs::load_from_lua_table(
					definition_template,
					additional_properties_path
				);
			}
			catch (augs::lua_deserialization_error err) {
				throw requisite_resource_loading_error(
					"Error while loading additional properties for %x (%x):", 
					stem, 
					additional_properties_path,
					err.what()
				);
			}
		}

		if (
			const auto source_image_path = typesafe_sprintf("%x%x.png", directory, stem);
			augs::file_exists(source_image_path)
		) {
			definition_template.source_image_path = source_image_path;
			requisites.emplace(id, definition_template);
		}
		else if (
			const auto procedural_definition_path = typesafe_sprintf("%xprocedural/%x.lua", directory, stem);
			augs::file_exists(procedural_definition_path)
		) {
			procedural_image_definition def;

			try {
				augs::load_from_lua_table(def, procedural_definition_path);
			}
			catch (augs::lua_deserialization_error err) {
				throw requisite_resource_loading_error(
					"Error while loading procedural image definition for %x (%x):",
					stem,
					procedural_definition_path,
					err.what()
				);
			}

			
			if (
				const bool exactly_one = def.button_with_corners.has_value() != def.scripted_image.has_value();
				!exactly_one
			) {
				throw requisite_resource_loading_error(
					"Error while loading procedural image definition for %x (%x):\n%x",
					stem,
					procedural_definition_path,
					"Either none or more than one type of procedural image have been specified."
				);
			}

			if (def.button_with_corners) {
				const auto path_template = get_procedural_image_path(
					typesafe_sprintf("%xprocedural/%x_%x.png", directory, stem)
				);
				
				const auto input = def.button_with_corners.value();
		
				regenerate_button_with_corners(path_template, input, force_regenerate);
		
				const auto first = id;

				augs::for_each_enum_except_bounds<button_corner_type>([&](const button_corner_type type) {
					definition_template.source_image_path = typesafe_sprintf(path_template.string(), get_filename_for(type));
					
					requisites.emplace(
						static_cast<id_type>(
							static_cast<int>(first) + static_cast<int>(type)
						), 
						definition_template
					);
				});
			}
			else if (def.scripted_image) {
				const auto generated_image_path = get_procedural_image_path(
					typesafe_sprintf("%xprocedural/%x.png", directory, stem)
				);
		
				regenerate_scripted_image(
					generated_image_path,
					def.scripted_image.value(),
					force_regenerate
				);
		
				definition_template.source_image_path = generated_image_path;
				requisites.emplace(id, definition_template);
			}
		}

		if (const bool nothing_loaded = definition_template.source_image_path.empty()) {
			throw requisite_resource_loading_error(
				"Error while loading requisite image: %x.\n%x",
				stem,
				"No source image exists, nor does a procedural definition."
			);
		}
	});
}