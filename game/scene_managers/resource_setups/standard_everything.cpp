#include "all.h"
#include "game/resources/manager.h"
#include "augs/graphics/shader.h"

#include "3rdparty/GL/OpenGL.h"

namespace resource_setups {
	void load_standard_everything() {
		resource_setups::load_standard_atlas();

		resource_manager.create(assets::atlas_id::GAME_WORLD_ATLAS,
			resources::manager::atlas_creation_mode::FROM_ALL_TEXTURES
			| resources::manager::atlas_creation_mode::FROM_ALL_FONTS);

		resource_setups::load_standard_particle_effects();
		resource_setups::load_standard_behaviour_trees();
		resource_setups::load_standard_tile_layers();
		resource_setups::load_standard_sound_buffers();

		resource_manager.create(assets::shader_id::DEFAULT_VERTEX, "hypersomnia/shaders/default.vsh", augs::graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::DEFAULT_FRAGMENT, "hypersomnia/shaders/default.fsh", augs::graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::DEFAULT, assets::shader_id::DEFAULT_VERTEX, assets::shader_id::DEFAULT_FRAGMENT);

		resource_manager.create(assets::shader_id::DEFAULT_ILLUMINATED_VERTEX, "hypersomnia/shaders/default_illuminated.vsh", augs::graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::DEFAULT_ILLUMINATED_FRAGMENT, "hypersomnia/shaders/default_illuminated.fsh", augs::graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::DEFAULT_ILLUMINATED, assets::shader_id::DEFAULT_ILLUMINATED_VERTEX, assets::shader_id::DEFAULT_ILLUMINATED_FRAGMENT);

		resource_manager.create(assets::shader_id::PURE_COLOR_HIGHLIGHT_VERTEX, "hypersomnia/shaders/pure_color_highlight.vsh", augs::graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::PURE_COLOR_HIGHLIGHT_FRAGMENT, "hypersomnia/shaders/pure_color_highlight.fsh", augs::graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::PURE_COLOR_HIGHLIGHT, assets::shader_id::PURE_COLOR_HIGHLIGHT_VERTEX, assets::shader_id::PURE_COLOR_HIGHLIGHT_FRAGMENT);

		resource_manager.create(assets::shader_id::CIRCULAR_BARS_VERTEX, "hypersomnia/shaders/circular_bars.vsh", augs::graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::CIRCULAR_BARS_FRAGMENT, "hypersomnia/shaders/circular_bars.fsh", augs::graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::CIRCULAR_BARS, assets::shader_id::CIRCULAR_BARS_VERTEX, assets::shader_id::CIRCULAR_BARS_FRAGMENT);

		resource_manager.create(assets::shader_id::LIGHT_VERTEX, "hypersomnia/shaders/light.vsh", augs::graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::LIGHT_FRAGMENT, "hypersomnia/shaders/light.fsh", augs::graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::LIGHT, assets::shader_id::LIGHT_VERTEX, assets::shader_id::LIGHT_FRAGMENT);

		resource_manager.create(assets::shader_id::SMOKE_VERTEX, "hypersomnia/shaders/fullscreen.vsh", augs::graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::SMOKE_FRAGMENT, "hypersomnia/shaders/smoke.fsh", augs::graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::SMOKE, assets::shader_id::SMOKE_VERTEX, assets::shader_id::SMOKE_FRAGMENT);

		resource_manager.create(assets::shader_id::SPECULAR_HIGHLIGHTS_VERTEX, "hypersomnia/shaders/default.vsh", augs::graphics::shader::type::VERTEX);
		resource_manager.create(assets::shader_id::SPECULAR_HIGHLIGHTS_FRAGMENT, "hypersomnia/shaders/specular_highlights.fsh", augs::graphics::shader::type::FRAGMENT);
		resource_manager.create(assets::program_id::SPECULAR_HIGHLIGHTS, assets::shader_id::SPECULAR_HIGHLIGHTS_VERTEX, assets::shader_id::SPECULAR_HIGHLIGHTS_FRAGMENT);

		{
			auto& illuminated_shader = *resource_manager.find(assets::program_id::DEFAULT_ILLUMINATED);
			illuminated_shader.use();

			const auto basic_texture_uniform = glGetUniformLocation(illuminated_shader.id, "basic_texture");
			const auto light_texture_uniform = glGetUniformLocation(illuminated_shader.id, "light_texture");

			glUniform1i(basic_texture_uniform, 0);
			glUniform1i(light_texture_uniform, 2);
		}

		{
			auto& default_shader = *resource_manager.find(assets::program_id::DEFAULT);
			default_shader.use();

			const auto basic_texture_uniform = glGetUniformLocation(default_shader.id, "basic_texture");
			glUniform1i(basic_texture_uniform, 0);
		}

		{
			auto& pure_color_highlight_shader = *resource_manager.find(assets::program_id::PURE_COLOR_HIGHLIGHT);
			pure_color_highlight_shader.use();

			const auto basic_texture_uniform = glGetUniformLocation(pure_color_highlight_shader.id, "basic_texture");
			glUniform1i(basic_texture_uniform, 0);
		}

		{
			auto& circular_bars_shader = *resource_manager.find(assets::program_id::CIRCULAR_BARS);
			circular_bars_shader.use();

			const auto basic_texture_uniform = glGetUniformLocation(circular_bars_shader.id, "basic_texture");
			glUniform1i(basic_texture_uniform, 0);
		}

		{
			auto& smoke_shader = *resource_manager.find(assets::program_id::SMOKE);
			smoke_shader.use();

			const auto light_texture_uniform = glGetUniformLocation(smoke_shader.id, "light_texture");
			const auto smoke_texture_uniform = glGetUniformLocation(smoke_shader.id, "smoke_texture");

			glUniform1i(smoke_texture_uniform, 1);
			glUniform1i(light_texture_uniform, 2);
		}

		{
			auto& specular_highlights_shader = *resource_manager.find(assets::program_id::SPECULAR_HIGHLIGHTS);
			specular_highlights_shader.use();

			const auto basic_texture_uniform = glGetUniformLocation(specular_highlights_shader.id, "basic_texture");
			const auto light_texture_uniform = glGetUniformLocation(specular_highlights_shader.id, "light_texture");

			glUniform1i(basic_texture_uniform, 0);
			glUniform1i(light_texture_uniform, 2);
		}
	}
}