#include "game/build_settings.h"
#if BUILD_TEST_SCENES
#include "all.h"
#include "game/assets/assets_manager.h"
#include "augs/graphics/shader.h"

#include "application/config_lua_table.h"

#include "application/content_generation/texture_atlases.h"
#include "application/content_generation/neon_maps.h"
#include "application/content_generation/desaturations.h"
#include "application/content_generation/buttons_with_corners.h"
#include "application/content_generation/scripted_images.h"
#include "application/content_generation/polygonizations_of_images.h"

#include "augs/graphics/OpenGL_includes.h"

using namespace augs::graphics;
using namespace assets;

void load_standard_everything(const config_lua_table& cfg) {
	auto& manager = get_assets_manager();

	const auto images = load_standard_images();
	const auto fonts = load_standard_fonts();

	atlases_regeneration_input in;

	for (const auto& i : images) {
		for (const auto& t : i.second.texture_maps) {
			if (t.path.size() > 0) {
				ensure(t.target_atlas != gl_texture_id::INVALID);

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
		cfg.save_regenerated_atlases_as_binary,
		cfg.packer_detail_max_atlas_size
	);

	LOG("Content regenerated successfully.\n--------------------------------------------\n");

	manager.load_baked_metadata(
		images,
		fonts,
		regenerated
	);

	set_standard_spell_properties(manager);
	set_standard_animations(manager);
	set_standard_particle_effects(manager);
	set_standard_tile_layers(manager);
	set_standard_sound_buffers(manager);
	set_standard_physical_materials(manager);
}

void create_standard_opengl_resources(const config_lua_table& cfg) {
	auto& manager = get_assets_manager();

	manager.create(
		gl_texture_id::GAME_WORLD_ATLAS,
		cfg.save_regenerated_atlases_as_binary
	);

	manager[shader_id::DEFAULT_VERTEX].create_from_file(shader::type::VERTEX, "resources/shaders/default.vsh");
	manager[shader_id::DEFAULT_FRAGMENT].create_from_file(shader::type::FRAGMENT, "resources/shaders/default.fsh");
	manager.create(program_id::DEFAULT, shader_id::DEFAULT_VERTEX, shader_id::DEFAULT_FRAGMENT);

	manager[shader_id::DEFAULT_ILLUMINATED_VERTEX].create_from_file(shader::type::VERTEX, "resources/shaders/default_illuminated.vsh");
	manager[shader_id::DEFAULT_ILLUMINATED_FRAGMENT].create_from_file(shader::type::FRAGMENT, "resources/shaders/default_illuminated.fsh");
	manager.create(program_id::DEFAULT_ILLUMINATED, shader_id::DEFAULT_ILLUMINATED_VERTEX, shader_id::DEFAULT_ILLUMINATED_FRAGMENT);

	manager[shader_id::PURE_COLOR_HIGHLIGHT_VERTEX].create_from_file(shader::type::VERTEX, "resources/shaders/pure_color_highlight.vsh");
	manager[shader_id::PURE_COLOR_HIGHLIGHT_FRAGMENT].create_from_file(shader::type::FRAGMENT, "resources/shaders/pure_color_highlight.fsh");
	manager.create(program_id::PURE_COLOR_HIGHLIGHT, shader_id::PURE_COLOR_HIGHLIGHT_VERTEX, shader_id::PURE_COLOR_HIGHLIGHT_FRAGMENT);

	manager[shader_id::CIRCULAR_BARS_VERTEX].create_from_file(shader::type::VERTEX, "resources/shaders/circular_bars.vsh");
	manager[shader_id::CIRCULAR_BARS_FRAGMENT].create_from_file(shader::type::FRAGMENT, "resources/shaders/circular_bars.fsh");
	manager.create(program_id::CIRCULAR_BARS, shader_id::CIRCULAR_BARS_VERTEX, shader_id::CIRCULAR_BARS_FRAGMENT);

	manager[shader_id::EXPLODING_RING_VERTEX].create_from_file(shader::type::VERTEX, "resources/shaders/exploding_ring.vsh");
	manager[shader_id::EXPLODING_RING_FRAGMENT].create_from_file(shader::type::FRAGMENT, "resources/shaders/exploding_ring.fsh");
	manager.create(program_id::EXPLODING_RING, shader_id::EXPLODING_RING_VERTEX, shader_id::EXPLODING_RING_FRAGMENT);

	manager[shader_id::LIGHT_VERTEX].create_from_file(shader::type::VERTEX, "resources/shaders/light.vsh");
	manager[shader_id::LIGHT_FRAGMENT].create_from_file(shader::type::FRAGMENT, "resources/shaders/light.fsh");
	manager.create(program_id::LIGHT, shader_id::LIGHT_VERTEX, shader_id::LIGHT_FRAGMENT);

	manager[shader_id::SMOKE_VERTEX].create_from_file(shader::type::VERTEX, "resources/shaders/fullscreen.vsh");
	manager[shader_id::SMOKE_FRAGMENT].create_from_file(shader::type::FRAGMENT, "resources/shaders/smoke.fsh");
	manager.create(program_id::SMOKE, shader_id::SMOKE_VERTEX, shader_id::SMOKE_FRAGMENT);

	manager[shader_id::ILLUMINATING_SMOKE_VERTEX].create_from_file(shader::type::VERTEX, "resources/shaders/fullscreen.vsh");
	manager[shader_id::ILLUMINATING_SMOKE_FRAGMENT].create_from_file(shader::type::FRAGMENT, "resources/shaders/illuminating_smoke.fsh");
	manager.create(program_id::ILLUMINATING_SMOKE, shader_id::ILLUMINATING_SMOKE_VERTEX, shader_id::ILLUMINATING_SMOKE_FRAGMENT);

	manager[shader_id::SPECULAR_HIGHLIGHTS_VERTEX].create_from_file(shader::type::VERTEX, "resources/shaders/default.vsh");
	manager[shader_id::SPECULAR_HIGHLIGHTS_FRAGMENT].create_from_file(shader::type::FRAGMENT, "resources/shaders/specular_highlights.fsh");
	manager.create(program_id::SPECULAR_HIGHLIGHTS, shader_id::SPECULAR_HIGHLIGHTS_VERTEX, shader_id::SPECULAR_HIGHLIGHTS_FRAGMENT);

	{
		auto& illuminated_shader = manager[program_id::DEFAULT_ILLUMINATED];
		illuminated_shader.use();

		const auto basic_texture_uniform = glGetUniformLocation(illuminated_shader.id, "basic_texture");
		const auto light_texture_uniform = glGetUniformLocation(illuminated_shader.id, "light_texture");

		glUniform1i(basic_texture_uniform, 0);
		glUniform1i(light_texture_uniform, 2);
	}

	{
		auto& default_shader = manager[program_id::DEFAULT];
		default_shader.use();

		const auto basic_texture_uniform = glGetUniformLocation(default_shader.id, "basic_texture");
		glUniform1i(basic_texture_uniform, 0);
	}

	{
		auto& pure_color_highlight_shader = manager[program_id::PURE_COLOR_HIGHLIGHT];
		pure_color_highlight_shader.use();

		const auto basic_texture_uniform = glGetUniformLocation(pure_color_highlight_shader.id, "basic_texture");
		glUniform1i(basic_texture_uniform, 0);
	}

	{
		auto& circular_bars_shader = manager[program_id::CIRCULAR_BARS];
		circular_bars_shader.use();

		const auto basic_texture_uniform = glGetUniformLocation(circular_bars_shader.id, "basic_texture");
		glUniform1i(basic_texture_uniform, 0);
	}

	{
		auto& smoke_shader = manager[program_id::SMOKE];
		smoke_shader.use();

		const auto light_texture_uniform = glGetUniformLocation(smoke_shader.id, "light_texture");
		const auto smoke_texture_uniform = glGetUniformLocation(smoke_shader.id, "smoke_texture");

		glUniform1i(smoke_texture_uniform, 1);
		glUniform1i(light_texture_uniform, 2);
	}

	{
		auto& illuminating_smoke_shader = manager[program_id::ILLUMINATING_SMOKE];
		illuminating_smoke_shader.use();

		const auto illuminating_smoke_texture_uniform = glGetUniformLocation(illuminating_smoke_shader.id, "smoke_texture");

		glUniform1i(illuminating_smoke_texture_uniform, 3);
	}

	{
		auto& specular_highlights_shader = manager[program_id::SPECULAR_HIGHLIGHTS];
		specular_highlights_shader.use();

		const auto basic_texture_uniform = glGetUniformLocation(specular_highlights_shader.id, "basic_texture");
		const auto light_texture_uniform = glGetUniformLocation(specular_highlights_shader.id, "light_texture");

		glUniform1i(basic_texture_uniform, 0);
		glUniform1i(light_texture_uniform, 2);
	}
}
#endif