#include "all.h"
#include "game/resources/manager.h"
#include "augs/graphics/shader.h"

#include "3rdparty/GL/OpenGL.h"

namespace resource_setups {
	void load_standard_everything() {
		auto& manager = get_resource_manager();
		
		const auto images = load_standard_images();
		const auto fonts = load_standard_fonts();

		atlas_regeneration_input in;
		manager.regenerate_atlases_and_load_baked_metadata(in);

		manager.create(assets::atlas_id::GAME_WORLD_ATLAS, "generated/atlases/game_world_atlas.png");

		manager.create_inverse_with_flip(
			assets::animation_id::TORSO_MOVE,
			assets::texture_id::TORSO_MOVING_FIRST,
			assets::texture_id::TORSO_MOVING_LAST,
			20.0f
		);

		manager.create_inverse_with_flip(
			assets::animation_id::BLUE_TORSO_MOVE,
			assets::texture_id::BLUE_TORSO_MOVING_FIRST,
			assets::texture_id::BLUE_TORSO_MOVING_LAST,
			20.0f
		);

		manager.create_inverse_with_flip(
			assets::animation_id::VIOLET_TORSO_MOVE,
			assets::texture_id::VIOLET_TORSO_MOVING_FIRST,
			assets::texture_id::VIOLET_TORSO_MOVING_LAST,
			20.0f
		);

		manager.create(
			assets::animation_id::BLINK_ANIMATION,
			assets::texture_id::BLINK_FIRST,
			assets::texture_id::BLINK_LAST,
			50.0f, resources::animation::loop_type::NONE
		);

		manager.create(
			assets::animation_id::CAST_BLINK_ANIMATION,
			assets::texture_id::CAST_BLINK_FIRST,
			assets::texture_id::CAST_BLINK_LAST,
			50.0f, resources::animation::loop_type::NONE
		);

		{
			auto& player_response = manager.create(assets::animation_response_id::TORSO_SET);
			player_response[animation_response_type::MOVE] = assets::animation_id::TORSO_MOVE;
		}

		{
			auto& player_response = manager.create(assets::animation_response_id::BLUE_TORSO_SET);
			player_response[animation_response_type::MOVE] = assets::animation_id::BLUE_TORSO_MOVE;
		}

		{
			auto& player_response = manager.create(assets::animation_response_id::VIOLET_TORSO_SET);
			player_response[animation_response_type::MOVE] = assets::animation_id::VIOLET_TORSO_MOVE;
		}

		resource_setups::load_standard_particle_effects();
		resource_setups::load_standard_behaviour_trees();
		resource_setups::load_standard_tile_layers();
		resource_setups::load_standard_sound_buffers();

		manager.create(assets::shader_id::DEFAULT_VERTEX, "hypersomnia/shaders/default.vsh", augs::graphics::shader::type::VERTEX);
		manager.create(assets::shader_id::DEFAULT_FRAGMENT, "hypersomnia/shaders/default.fsh", augs::graphics::shader::type::FRAGMENT);
		manager.create(assets::program_id::DEFAULT, assets::shader_id::DEFAULT_VERTEX, assets::shader_id::DEFAULT_FRAGMENT);

		manager.create(assets::shader_id::DEFAULT_ILLUMINATED_VERTEX, "hypersomnia/shaders/default_illuminated.vsh", augs::graphics::shader::type::VERTEX);
		manager.create(assets::shader_id::DEFAULT_ILLUMINATED_FRAGMENT, "hypersomnia/shaders/default_illuminated.fsh", augs::graphics::shader::type::FRAGMENT);
		manager.create(assets::program_id::DEFAULT_ILLUMINATED, assets::shader_id::DEFAULT_ILLUMINATED_VERTEX, assets::shader_id::DEFAULT_ILLUMINATED_FRAGMENT);

		manager.create(assets::shader_id::PURE_COLOR_HIGHLIGHT_VERTEX, "hypersomnia/shaders/pure_color_highlight.vsh", augs::graphics::shader::type::VERTEX);
		manager.create(assets::shader_id::PURE_COLOR_HIGHLIGHT_FRAGMENT, "hypersomnia/shaders/pure_color_highlight.fsh", augs::graphics::shader::type::FRAGMENT);
		manager.create(assets::program_id::PURE_COLOR_HIGHLIGHT, assets::shader_id::PURE_COLOR_HIGHLIGHT_VERTEX, assets::shader_id::PURE_COLOR_HIGHLIGHT_FRAGMENT);

		manager.create(assets::shader_id::CIRCULAR_BARS_VERTEX, "hypersomnia/shaders/circular_bars.vsh", augs::graphics::shader::type::VERTEX);
		manager.create(assets::shader_id::CIRCULAR_BARS_FRAGMENT, "hypersomnia/shaders/circular_bars.fsh", augs::graphics::shader::type::FRAGMENT);
		manager.create(assets::program_id::CIRCULAR_BARS, assets::shader_id::CIRCULAR_BARS_VERTEX, assets::shader_id::CIRCULAR_BARS_FRAGMENT);

		manager.create(assets::shader_id::EXPLODING_RING_VERTEX, "hypersomnia/shaders/exploding_ring.vsh", augs::graphics::shader::type::VERTEX);
		manager.create(assets::shader_id::EXPLODING_RING_FRAGMENT, "hypersomnia/shaders/exploding_ring.fsh", augs::graphics::shader::type::FRAGMENT);
		manager.create(assets::program_id::EXPLODING_RING, assets::shader_id::EXPLODING_RING_VERTEX, assets::shader_id::EXPLODING_RING_FRAGMENT);

		manager.create(assets::shader_id::LIGHT_VERTEX, "hypersomnia/shaders/light.vsh", augs::graphics::shader::type::VERTEX);
		manager.create(assets::shader_id::LIGHT_FRAGMENT, "hypersomnia/shaders/light.fsh", augs::graphics::shader::type::FRAGMENT);
		manager.create(assets::program_id::LIGHT, assets::shader_id::LIGHT_VERTEX, assets::shader_id::LIGHT_FRAGMENT);

		manager.create(assets::shader_id::SMOKE_VERTEX, "hypersomnia/shaders/fullscreen.vsh", augs::graphics::shader::type::VERTEX);
		manager.create(assets::shader_id::SMOKE_FRAGMENT, "hypersomnia/shaders/smoke.fsh", augs::graphics::shader::type::FRAGMENT);
		manager.create(assets::program_id::SMOKE, assets::shader_id::SMOKE_VERTEX, assets::shader_id::SMOKE_FRAGMENT);

		manager.create(assets::shader_id::ILLUMINATING_SMOKE_VERTEX, "hypersomnia/shaders/fullscreen.vsh", augs::graphics::shader::type::VERTEX);
		manager.create(assets::shader_id::ILLUMINATING_SMOKE_FRAGMENT, "hypersomnia/shaders/illuminating_smoke.fsh", augs::graphics::shader::type::FRAGMENT);
		manager.create(assets::program_id::ILLUMINATING_SMOKE, assets::shader_id::ILLUMINATING_SMOKE_VERTEX, assets::shader_id::ILLUMINATING_SMOKE_FRAGMENT);

		manager.create(assets::shader_id::SPECULAR_HIGHLIGHTS_VERTEX, "hypersomnia/shaders/default.vsh", augs::graphics::shader::type::VERTEX);
		manager.create(assets::shader_id::SPECULAR_HIGHLIGHTS_FRAGMENT, "hypersomnia/shaders/specular_highlights.fsh", augs::graphics::shader::type::FRAGMENT);
		manager.create(assets::program_id::SPECULAR_HIGHLIGHTS, assets::shader_id::SPECULAR_HIGHLIGHTS_VERTEX, assets::shader_id::SPECULAR_HIGHLIGHTS_FRAGMENT);

		{
			auto& illuminated_shader = *manager.find(assets::program_id::DEFAULT_ILLUMINATED);
			illuminated_shader.use();

			const auto basic_texture_uniform = glGetUniformLocation(illuminated_shader.id, "basic_texture");
			const auto light_texture_uniform = glGetUniformLocation(illuminated_shader.id, "light_texture");

			glUniform1i(basic_texture_uniform, 0);
			glUniform1i(light_texture_uniform, 2);
		}

		{
			auto& default_shader = *manager.find(assets::program_id::DEFAULT);
			default_shader.use();

			const auto basic_texture_uniform = glGetUniformLocation(default_shader.id, "basic_texture");
			glUniform1i(basic_texture_uniform, 0);
		}

		{
			auto& pure_color_highlight_shader = *manager.find(assets::program_id::PURE_COLOR_HIGHLIGHT);
			pure_color_highlight_shader.use();

			const auto basic_texture_uniform = glGetUniformLocation(pure_color_highlight_shader.id, "basic_texture");
			glUniform1i(basic_texture_uniform, 0);
		}

		{
			auto& circular_bars_shader = *manager.find(assets::program_id::CIRCULAR_BARS);
			circular_bars_shader.use();

			const auto basic_texture_uniform = glGetUniformLocation(circular_bars_shader.id, "basic_texture");
			glUniform1i(basic_texture_uniform, 0);
		}

		{
			auto& smoke_shader = *manager.find(assets::program_id::SMOKE);
			smoke_shader.use();

			const auto light_texture_uniform = glGetUniformLocation(smoke_shader.id, "light_texture");
			const auto smoke_texture_uniform = glGetUniformLocation(smoke_shader.id, "smoke_texture");

			glUniform1i(smoke_texture_uniform, 1);
			glUniform1i(light_texture_uniform, 2);
		}

		{
			auto& illuminating_smoke_shader = *manager.find(assets::program_id::ILLUMINATING_SMOKE);
			illuminating_smoke_shader.use();

			const auto illuminating_smoke_texture_uniform = glGetUniformLocation(illuminating_smoke_shader.id, "smoke_texture");

			glUniform1i(illuminating_smoke_texture_uniform, 3);
		}

		{
			auto& specular_highlights_shader = *manager.find(assets::program_id::SPECULAR_HIGHLIGHTS);
			specular_highlights_shader.use();

			const auto basic_texture_uniform = glGetUniformLocation(specular_highlights_shader.id, "basic_texture");
			const auto light_texture_uniform = glGetUniformLocation(specular_highlights_shader.id, "light_texture");

			glUniform1i(basic_texture_uniform, 0);
			glUniform1i(light_texture_uniform, 2);
		}
	}
}