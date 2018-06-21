#pragma once
#include "augs/math/camera_cone.h"
#include "augs/math/matrix.h"

#include "augs/drawing/drawing.h"

#include "augs/graphics/renderer.h"
#include "augs/graphics/shader.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "view/frame_profiler.h"
#include "view/rendering_scripts/draw_entity.h"

#include "game/components/item_slot_transfers_component.h"
#include "game/components/render_component.h"
#include "game/debug_drawing_settings.h"
#include "game/detail/visible_entities.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"

#include "view/viewer_eye.h"
#include "view/rendering_scripts/rendering_scripts.h"
#include "view/rendering_scripts/illuminated_rendering.h"

#include "view/necessary_resources.h"
#include "view/game_drawing_settings.h"
#include "view/viewables/all_viewables_declarations.h"
#include "view/viewables/image_in_atlas.h"
#include "view/viewables/images_in_atlas_map.h"

#include "view/audiovisual_state/audiovisual_state.h"

#include "view/viewables/atlas_distributions.h"

namespace augs {
	class renderer;
}

struct all_necessary_shaders;
struct all_necessary_fbos;

/* Require all */

using illuminated_rendering_fbos = all_necessary_fbos;
using illuminated_rendering_shaders = all_necessary_shaders;

struct illuminated_rendering_input {
	const viewer_eye eye;
	const audiovisual_state& audiovisuals;
	const game_drawing_settings drawing;
	const necessary_images_in_atlas_map& necessary_images;
	const augs::baked_font& gui_font;
	const images_in_atlas_map& game_images;
	const double interpolation_ratio = 0.0;
	augs::renderer& renderer;
	frame_profiler& frame_performance;
	const std::optional<augs::graphics::texture>& general_atlas;
	const illuminated_rendering_fbos& fbos;
	const illuminated_rendering_shaders& shaders;
	const visible_entities& all_visible;
};

template <class B, class H>
void illuminated_rendering(
	const illuminated_rendering_input in,
	const bool has_additional_borders,
	const B& get_additional_border,
	const H& for_each_additional_highlight
) {
	auto& profiler = in.frame_performance;
	auto scope = measure_scope(profiler.rendering_script);

	auto& renderer = in.renderer;
	
	const auto viewed_character = in.eye.viewed_character;
	const auto screen_size = in.eye.screen_size;

	const auto camera = [&in](){ 
		auto result = in.eye.cone;
		result.transform.pos.discard_fract();
		return result;
	}();

	const auto& cosmos = viewed_character.get_cosmos();
	
	const auto& anims = cosmos.get_logical_assets().plain_animations;

	const auto& interp = in.audiovisuals.get<interpolation_system>();
	const auto& particles = in.audiovisuals.get<particles_simulation_system>();
	const auto& wandering_pixels = in.audiovisuals.get<wandering_pixels_system>();
	const auto& exploding_rings = in.audiovisuals.get<exploding_ring_system>();
	const auto& flying_numbers = in.audiovisuals.get<flying_number_indicator_system>();
	const auto& highlights = in.audiovisuals.get<pure_color_highlight_system>();
	const auto& thunders = in.audiovisuals.get<thunder_system>();
	const auto global_time_seconds = cosmos.get_total_seconds_passed(in.interpolation_ratio);
	const auto settings = in.drawing;
	const auto matrix = camera.get_projection_matrix(screen_size);
	const auto& visible = in.all_visible;
	const auto& shaders = in.shaders;
	const auto& fbos = in.fbos;
	const auto& necessarys = in.necessary_images;
	const auto& game_images = in.game_images;
	const auto blank = necessarys.at(assets::necessary_image_id::BLANK);
	const auto& gui_font = in.gui_font;

	const auto output = augs::drawer_with_default{ renderer.get_triangle_buffer(), blank };
	const auto line_output = augs::line_drawer_with_default{ renderer.get_line_buffer(), blank };

	if (in.general_atlas) {
		in.general_atlas->bind();
	}

	auto set_shader_with_matrix = [&](auto& shader) {
		shader->set_as_current();
		shader->set_projection(matrix);
	};

	set_shader_with_matrix(shaders.standard);

	fbos.smoke->set_as_current();

	renderer.clear_current_fbo();
	renderer.set_additive_blending();
	
	const auto draw_particles_in = draw_particles_input{ output, false };

	auto draw_particles = [&](const render_layer layer) {
		particles.draw_particles(
			game_images,
			anims,
			draw_particles_in,
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
	const auto glow_edge_tex = necessarys.at(assets::necessary_image_id::LASER_GLOW_EDGE);

	const auto cast_highlight = necessarys.at(assets::necessary_image_id::CAST_HIGHLIGHT);

	light.render_all_lights({
		renderer,
		profiler,
		cosmos, 
		matrix,
		fbos.light.value(),
		*shaders.light, 
		*shaders.standard, 
		[&]() {
			if (viewed_character) {
				draw_crosshair_lasers({
					[&](const vec2 from, const vec2 to, const rgba col) {
						if (!settings.draw_weapon_laser) {
							return;
						}

						const vec2 edge_size = static_cast<vec2>(glow_edge_tex.get_original_size());

						output.line(laser_glow, from, to, edge_size.y / 3.f, col);

						const auto edge_dir = (to - from).normalize();
						const auto edge_offset = edge_dir * edge_size.x;

						output.line(glow_edge_tex, to, to + edge_offset, edge_size.y / 3.f, col);
						output.line(glow_edge_tex, from - edge_offset + edge_dir, from + edge_dir, edge_size.y / 3.f, col, flip_flags::make_horizontally());
					},
					[](const vec2, const vec2) {},
					interp,
					viewed_character
				});
			}

			draw_cast_spells_highlights({
				output,
				interp,
				cosmos,
				global_time_seconds,
				cast_highlight
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
				cosmos,
				camera,
				screen_size
			);
		},
		camera,
		screen_size,
		interp,
		particles,
		anims,
		visible.per_layer,
		game_images,
		global_time_seconds
	});

	set_shader_with_matrix(shaders.illuminated);

	auto draw_layer = [&](const render_layer r) {
		for (const auto e : visible.per_layer[r]) {
			draw_entity(cosmos[e], { output, game_images, global_time_seconds, flip_flags() }, interp);
		}
	};

	auto draw_borders = [&](const render_layer r, auto provider) {
		for (const auto e : visible.per_layer[r]) {
			draw_border(cosmos[e], { output, game_images, global_time_seconds, flip_flags() }, interp, provider);
		}
	};

	draw_layer(render_layer::UNDER_GROUND);
	draw_layer(render_layer::GROUND);
	draw_layer(render_layer::FLOOR_AND_ROAD);
	draw_layer(render_layer::ON_FLOOR);
	draw_layer(render_layer::ON_ON_FLOOR);

	draw_layer(render_layer::AQUARIUM_FLOWERS);
	draw_layer(render_layer::BOTTOM_FISH);
	draw_layer(render_layer::UPPER_FISH);

	renderer.call_and_clear_triangles();

	set_shader_with_matrix(shaders.specular_highlights);

	renderer.call_and_clear_triangles();

	shaders.illuminated->set_as_current();

	draw_layer(render_layer::CAR_INTERIOR);
	draw_layer(render_layer::CAR_WHEEL);

	renderer.call_and_clear_triangles();

	set_shader_with_matrix(shaders.pure_color_highlight);

	const auto timestamp_ms = static_cast<unsigned>(global_time_seconds * 1000);
	
	auto standard_border_provider = [timestamp_ms](const const_entity_handle sentience) {
		std::optional<rgba> result;

		sentience.dispatch_on_having<components::sentience>(
			[&](const auto typed_handle) {
				result = typed_handle.template get<components::sentience>().find_low_health_border(timestamp_ms);
			}
		);

		return result;
	};

	if (has_additional_borders) {
		draw_borders(
			render_layer::SMALL_DYNAMIC_BODY,
			[&](const const_entity_handle sentience) -> std::optional<rgba> {
				if (const auto maybe_border = get_additional_border(sentience)) {
					return maybe_border;
				}

				return standard_border_provider(sentience);
			}
		);
	}
	else {
		draw_borders(render_layer::SENTIENCES, standard_border_provider);
	}

	renderer.call_and_clear_triangles();
	
	shaders.illuminated->set_as_current();
	
	draw_layer(render_layer::DYNAMIC_BODY);
	draw_layer(render_layer::SMALL_DYNAMIC_BODY);
	draw_layer(render_layer::SENTIENCES);
	
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
		cosmos.template for_each_having<components::sentience>(
			[&](const auto it) {
				if (const auto s = it.find_crosshair_def()) {
					invariants::sprite::drawing_input in = output;
					in.global_time_seconds = global_time_seconds;
					in.renderable_transform = it.get_world_crosshair_transform(interp);

					s->appearance.draw(game_images, in);
				}
			}
		);
	}
	
	if (settings.draw_weapon_laser && viewed_character.alive()) {
		const auto laser = necessarys.at(assets::necessary_image_id::LASER);
		
		draw_crosshair_lasers({
			[&](const vec2 from, const vec2 to, const rgba col) {
				line_output.line(
					laser,
					from,
					to, 
					col
				);
			},

			[&](const vec2 from, const vec2 to) {
				line_output.dashed_line(
					laser,
					from,
					to,
					white,
					10.f,
					40.f, 
					global_time_seconds
				);
			},

			interp, 
			viewed_character
		});

		renderer.call_lines();
		renderer.clear_lines();
	}

	draw_particles(render_layer::ILLUMINATING_PARTICLES);

	for (const auto e : visible.per_layer[render_layer::WANDERING_PIXELS_EFFECTS]) {
		wandering_pixels.draw_wandering_pixels_as_sprites(cosmos[e], game_images, invariants::sprite::drawing_input(output));
	}

	renderer.call_and_clear_triangles();

	set_shader_with_matrix(shaders.circular_bars);

	const auto set_center_uniform = [&](const augs::atlas_entry& tex) {
		shaders.circular_bars->set_uniform("texture_center", tex.get_center());
	};

	augs::vertex_triangle_buffer textual_infos;

	if (viewed_character) {
		const auto tex = necessarys.at(assets::necessary_image_id::CIRCULAR_BAR_MEDIUM);

		set_center_uniform(tex);

		textual_infos = draw_circular_bars_and_get_textual_info({
			visible.all,
			output,
			renderer.get_special_buffer(),
			cosmos,
			viewed_character,
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

		draw_hud_for_unpinned_explosives({
			output,
			renderer.get_special_buffer(),
			interp,
			cosmos,
			global_time_seconds,
			tex
		});

		renderer.call_and_clear_triangles();
	}

	shaders.standard->set_as_current();

	renderer.call_triangles(textual_infos);

	set_shader_with_matrix(shaders.exploding_rings);

	exploding_rings.draw_rings(
		output,
		renderer.specials,
		cosmos,
		camera,
		screen_size
	);

	renderer.call_and_clear_triangles();

	shaders.pure_color_highlight->set_as_current();

	highlights.draw_highlights(
		output,
		cosmos,
		interp,
		game_images
	);

	for_each_additional_highlight([&](
		const entity_id subject, 
		const rgba color
	){
		draw_color_highlight(
			cosmos[subject],
			color,
			{
				output,
				game_images,
				global_time_seconds,
				flip_flags()
			},
			interp
		);
	});

	thunders.draw_thunders(
		line_output
	);

	renderer.call_and_clear_triangles();
	renderer.call_and_clear_lines();

	shaders.standard->set_as_current();

	flying_numbers.draw_numbers(
		gui_font,
		output, 
		camera,
		screen_size
	);

	renderer.call_and_clear_triangles();
	renderer.call_and_clear_lines();

	if (in.general_atlas) {
		in.general_atlas->bind();
	}
}
