#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/assets/assets_manager.h"
#include "game/assets/shader_program_id.h"
#include "augs/graphics/shader.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/templates/string_to_enum.h"

#include "generated/introspectors.h"

using namespace assets;
using namespace augs::graphics;

void load_requisite_shaders(assets_manager& manager) {
	augs::for_each_file_in_dir_recursive(
		"content/requisite/shaders",
		[&](const std::string& path) {
			if (augs::get_extension(path) == ".vsh") {
				const auto program_id = augs::string_to_enum<shader_program_id>(to_uppercase(augs::get_stem(path)));
				const auto& vsh_path = path;
				const auto& fsh_path = augs::replace_extension(vsh_path, ".fsh");

				manager.get_store_by<shader_program_id>().try_emplace(
					program_id,
					vsh_path,
					fsh_path
				);
			}
		}
	);

	{
		auto& illuminated_shader = manager.at(shader_program_id::DEFAULT_ILLUMINATED);
		illuminated_shader.set_as_current();

		illuminated_shader.set_uniform("basic_texture", 0);
		illuminated_shader.set_uniform("light_texture", 2);
	}

	{
		auto& default_shader = manager.at(shader_program_id::DEFAULT);
		default_shader.set_as_current();

		default_shader.set_uniform("basic_texture", 0);
	}

	{
		auto& pure_color_highlight_shader = manager.at(shader_program_id::PURE_COLOR_HIGHLIGHT);
		pure_color_highlight_shader.set_as_current();
		pure_color_highlight_shader.set_uniform("basic_texture", 0);
	}

	{
		auto& circular_bars_shader = manager.at(shader_program_id::CIRCULAR_BARS);
		circular_bars_shader.set_as_current();
		circular_bars_shader.set_uniform("basic_texture", 0);
	}

	{
		auto& smoke_shader = manager.at(shader_program_id::SMOKE);
		smoke_shader.set_as_current();

		smoke_shader.set_uniform("smoke_texture", 1);
		smoke_shader.set_uniform("light_texture", 2);
	}

	{
		auto& illuminating_smoke_shader = manager.at(shader_program_id::ILLUMINATING_SMOKE);
		illuminating_smoke_shader.set_as_current();
		illuminating_smoke_shader.set_uniform("smoke_texture", 3);
	}

	{
		auto& specular_highlights_shader = manager.at(shader_program_id::SPECULAR_HIGHLIGHTS);
		specular_highlights_shader.set_as_current();
		specular_highlights_shader.set_uniform("basic_texture", 0);
		specular_highlights_shader.set_uniform("light_texture", 2);
	}
}