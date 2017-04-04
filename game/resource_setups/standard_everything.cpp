#include "all.h"
#include "game/resources/manager.h"
#include "augs/graphics/shader.h"

#include "application/config_lua_table.h"

#include "application/content_generation/texture_atlases.h"
#include "application/content_generation/neon_maps.h"
#include "application/content_generation/desaturations.h"
#include "application/content_generation/buttons_with_corners.h"
#include "application/content_generation/scripted_images.h"
#include "application/content_generation/polygonizations_of_images.h"

#include "3rdparty/GL/OpenGL.h"

void load_standard_everything(const config_lua_table& cfg) {
	auto& manager = get_resource_manager();

	const auto images = load_standard_images();
	const auto fonts = load_standard_fonts();

	atlases_regeneration_input in;

	for (const auto& i : images) {
		for (const auto& t : i.second.texture_maps) {
			if (t.path.size() > 0) {
				ensure(t.target_atlas != assets::physical_texture_id::INVALID);

				in.images.push_back({ t.path, t.target_atlas });
			}
		}
	}

	for (const auto& f : fonts) {
		in.fonts.push_back({ f.second.loading_input, f.second.target_atlas });
	}

	LOG("\n--------------------------------------------\nChecking content integrity...");

	regenerate_scripted_images(cfg.debug_regenerate_content_every_launch);
	regenerate_buttons_with_corners(cfg.debug_regenerate_content_every_launch);
	regenerate_neon_maps(cfg.debug_regenerate_content_every_launch);
	regenerate_desaturations(cfg.debug_regenerate_content_every_launch);
	regenerate_polygonizations_of_images(cfg.debug_regenerate_content_every_launch);

	const auto regenerated = regenerate_atlases(
		in,
		cfg.debug_regenerate_content_every_launch,
		cfg.check_content_integrity_every_launch,
		cfg.save_regenerated_atlases_as_binary
	);

	LOG("Content regenerated successfully.\n--------------------------------------------\n");

	manager.load_baked_metadata(
		images,
		fonts,
		regenerated
	);

	manager.create(
		assets::physical_texture_id::GAME_WORLD_ATLAS,
		cfg.save_regenerated_atlases_as_binary
	);

	load_standard_behaviour_trees();
	load_standard_tile_layers();
	load_standard_sound_buffers();

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
