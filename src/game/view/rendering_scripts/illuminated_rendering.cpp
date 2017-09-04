#include "augs/graphics/renderer.h"

#include "augs/math/matrix.h"

#include "game/assets/all_assets.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/view/necessary_resources.h"

#include "game/systems_stateless/render_system.h"

#include "game/view/rendering_scripts/rendering_scripts.h"
#include "game/view/rendering_scripts/illuminated_rendering.h"

#include "game/view/game_gui/elements/character_gui.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/render_component.h"
#include "game/view/audiovisual_state/audiovisual_state.h"
#include "game/debug_drawing_settings.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "game/view/audiovisual_state/systems/interpolation_system.h"

#include "augs/drawing/drawing.h"
#include "augs/graphics/shader.h"

void illuminated_rendering(const illuminated_rendering_input in) {
	auto& renderer = in.renderer;
	
	const auto& cosmos = in.cosm;
	const auto camera = in.camera;
	const auto viewed_character = cosmos[in.viewed_character];
	const auto viewed_crosshair = viewed_character.alive() ? viewed_character[child_entity_name::CHARACTER_CROSSHAIR] : cosmos[entity_id()];
	const auto& interp = in.audiovisuals.get<interpolation_system>();
	const auto& particles = in.audiovisuals.get<particles_simulation_system>();
	const auto& wandering_pixels = in.audiovisuals.get<wandering_pixels_system>();
	const auto& exploding_rings = in.audiovisuals.get<exploding_ring_system>();
	const auto& flying_numbers = in.audiovisuals.get<flying_number_indicator_system>();
	const auto& highlights = in.audiovisuals.get<pure_color_highlight_system>();
	const auto& thunders = in.audiovisuals.get<thunder_system>();
	const auto global_time_seconds = cosmos.get_total_time_passed_in_seconds(in.interpolation_ratio);
	const auto settings = in.drawing;
	const auto matrix = augs::orthographic_projection(camera.visible_world_area);
	const auto& visible_per_layer = in.visible.per_layer;
	const auto& shaders = in.shaders;
	const auto& fbos = in.fbos;
	const auto& necessarys = in.necessary_images;
	const auto& game_images = in.game_images;
	const auto blank = necessarys.at(assets::necessary_image_id::BLANK);
	const auto& gui_font = in.gui_font;

	const auto output = augs::drawer_with_default{ renderer.get_triangle_buffer(), blank };
	const auto line_output = augs::line_drawer_with_default{ renderer.get_line_buffer(), blank };

	in.game_world_atlas.bind();

	shaders.standard->set_as_current();
	shaders.standard->set_projection(matrix);

	fbos.smoke->set_as_current();

	renderer.clear_current_fbo();
	renderer.set_additive_blending();
	
	auto basic_sprite_input = components::sprite::drawing_input(output);
	basic_sprite_input.camera = camera;

	auto draw_particles = [&](const render_layer layer) {
		particles.draw_particles_as_sprites(
			game_images,
			basic_sprite_input,
			layer
		);
	};

	draw_particles(render_layer::DIM_SMOKES);
	draw_particles(render_layer::ILLUMINATING_SMOKES);

	renderer.call_and_clear_triangles();

	fbos.illuminating_smoke->set_as_current();
	renderer.clear_current_fbo();

	draw_particles(render_layer::ILLUMINATING_SMOKES);

	renderer.call_and_clear_triangles();
	
	renderer.set_standard_blending();

	augs::graphics::fbo::set_current_to_none();

	const auto& light = in.audiovisuals.get<light_system>();
	
	const auto laser_glow = necessarys.at(assets::necessary_image_id::LASER_GLOW);
	const auto laser_glow_edge = necessarys.at(assets::necessary_image_id::LASER_GLOW_EDGE);

	const auto cast_highlight = necessarys.at(assets::necessary_image_id::CAST_HIGHLIGHT);

	light.render_all_lights({
		renderer,
		cosmos, 
		matrix,
		fbos.light.value(),
		*shaders.light, 
		*shaders.standard, 
		[&]() {
			if (viewed_character.alive()) {
				draw_crosshair_lasers({
					[&](const vec2 from, const vec2 to, const rgba col) {
						if (!settings.draw_weapon_laser) {
							return;
						}

						const vec2 edge_size = static_cast<vec2>(laser_glow_edge.get_size());

						output.line(laser_glow, camera[from], camera[to], edge_size.y / 3.f, col);

						const auto edge_offset = (to - from).set_length(edge_size.x);

						output.line(laser_glow_edge, camera[to], camera[to + edge_offset], edge_size.y / 3.f, col);
						output.line(laser_glow_edge, camera[from - edge_offset], camera[from], edge_size.y / 3.f, col, { flip::HORIZONTALLY });
					},
					[](const vec2, const vec2) {},
					interp,
					viewed_crosshair,
					viewed_character
				});
			}

			draw_cast_spells_highlights({
				output,
				interp,
				camera,
				cosmos,
				global_time_seconds
			});

			renderer.set_active_texture(3);
			fbos.illuminating_smoke->get_texture().bind();
			renderer.set_active_texture(0);

			shaders.illuminating_smoke->set_as_current();

			renderer.fullscreen_quad();

			shaders.standard->set_as_current();

			exploding_rings.draw_highlights_of_rings(
				output,
				cast_highlight,
				camera,
				cosmos
			);
		},
		camera,
		interp,
		particles,
		in.visible.per_layer,
		game_images
	});

	shaders.illuminated->set_as_current();
	shaders.illuminated->set_projection(matrix);

	auto draw_layer = [&](
		const render_layer r, 
		const renderable_drawing_type type = renderable_drawing_type::NORMAL
	) {
		render_system().draw_entities(visible_per_layer[r], cosmos, output, game_images, camera, global_time_seconds, interp, type);
	};

	draw_layer(render_layer::UNDER_GROUND);
	draw_layer(render_layer::GROUND);
	draw_layer(render_layer::ON_GROUND);

	renderer.call_and_clear_triangles();

	shaders.specular_highlights->set_as_current();
	shaders.specular_highlights->set_projection(matrix);

	renderer.call_and_clear_triangles();

	shaders.illuminated->set_as_current();

	draw_layer(render_layer::CAR_INTERIOR);
	draw_layer(render_layer::CAR_WHEEL);

	renderer.call_and_clear_triangles();

	shaders.pure_color_highlight->set_as_current();
	shaders.pure_color_highlight->set_projection(matrix);
	
	draw_layer(render_layer::SMALL_DYNAMIC_BODY, renderable_drawing_type::BORDER_HIGHLIGHTS);
	
	renderer.call_and_clear_triangles();
	
	shaders.illuminated->set_as_current();
	
	draw_layer(render_layer::DYNAMIC_BODY);
	draw_layer(render_layer::SMALL_DYNAMIC_BODY);
	
	renderer.call_and_clear_triangles();

	renderer.set_active_texture(1);
	fbos.smoke->get_texture().bind();
	renderer.set_active_texture(0);

	shaders.smoke->set_as_current();

	renderer.fullscreen_quad();

	shaders.standard->set_as_current();
	
	draw_layer(render_layer::FLYING_BULLETS);
	draw_layer(render_layer::NEON_CAPTIONS);
	
	if (settings.draw_crosshairs) {
		draw_layer(render_layer::CROSSHAIR);
	}
	
	draw_layer(render_layer::OVER_CROSSHAIR);
	
	if (settings.draw_weapon_laser && viewed_character.alive()) {
		const auto laser = necessarys.at(assets::necessary_image_id::LASER);
		
		draw_crosshair_lasers({
			[&](const vec2 from, const vec2 to, const rgba col) {
				line_output.line(
					laser,
					camera[from],
					camera[to], 
					col
				);
			},

			[&](const vec2 from, const vec2 to) {
				line_output.dashed_line(
					laser,
					camera[from],
					camera[to],
					white,
					10.f,
					40.f, 
					global_time_seconds
				);
			},

			interp, 
			viewed_crosshair, 
			viewed_character
		});

		renderer.call_lines();
		renderer.clear_lines();
	}

	draw_particles(render_layer::ILLUMINATING_PARTICLES);

	for (const auto e : visible_per_layer[render_layer::WANDERING_PIXELS_EFFECTS]) {
		wandering_pixels.draw_wandering_pixels_as_sprites(cosmos[e], game_images, basic_sprite_input);
	}

	renderer.call_and_clear_triangles();

	shaders.circular_bars->set_as_current();
	shaders.circular_bars->set_projection(matrix);

	const auto set_center_uniform = [&](const auto& tex) {
		const auto upper = tex.get_atlas_space_uv({ 0.0f, 0.0f });
		const auto lower = tex.get_atlas_space_uv({ 1.f, 1.f });
		const auto center = (upper + lower) / 2;

		shaders.circular_bars->set_uniform("texture_center", center);
	};

	augs::vertex_triangle_buffer textual_infos;

	{
		const auto tex = necessarys.at(assets::necessary_image_id::CIRCULAR_BAR_MEDIUM);

		set_center_uniform(tex);

		textual_infos = draw_circular_bars_and_get_textual_info({
			in.visible.all,
			output,
			renderer.get_special_buffer(),
			cosmos,
			viewed_character,
			camera,
			interp,
			global_time_seconds,
			gui_font,
			tex
		});

		renderer.call_and_clear_triangles();
	}
	
	{
		const auto tex = necessarys.at(assets::necessary_image_id::CIRCULAR_BAR_SMALL);

		set_center_uniform(tex);

		draw_hud_for_released_explosives({
			output,
			renderer.get_special_buffer(),
			interp,
			camera,
			cosmos,
			global_time_seconds
		});

		renderer.call_and_clear_triangles();
	}

	shaders.standard->set_as_current();

	renderer.call_triangles(textual_infos);

	shaders.exploding_rings->set_as_current();
	shaders.exploding_rings->set_projection(matrix);

	exploding_rings.draw_rings(
		output,
		renderer.specials,
		camera,
		cosmos
	);

	renderer.call_and_clear_triangles();

	shaders.pure_color_highlight->set_as_current();

	highlights.draw_highlights(
		output,
		camera,
		cosmos,
		interp,
		game_images
	);

	thunders.draw_thunders(
		line_output,
		camera
	);

	renderer.call_and_clear_triangles();
	renderer.call_and_clear_lines();

	shaders.standard->set_as_current();

	flying_numbers.draw_numbers(
		gui_font,
		output, 
		camera
	);

	in.game_world_atlas.bind();
}
