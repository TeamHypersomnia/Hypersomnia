#include "augs/math/camera_cone.h"
#include "augs/math/matrix.h"

#include "augs/drawing/drawing.h"

#include "augs/graphics/renderer.h"
#include "augs/graphics/shader.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"

#include "view/frame_profiler.h"
#include "view/rendering_scripts/draw_entity.h"

#include "game/components/item_slot_transfers_component.h"
#include "game/debug_drawing_settings.h"
#include "game/detail/visible_entities.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"

#include "view/rendering_scripts/rendering_scripts.h"
#include "view/rendering_scripts/illuminated_rendering.h"
#include "view/rendering_scripts/draw_wandering_pixels_as_sprites.h"
#include "view/rendering_scripts/helper_drawer.h"
#include "view/rendering_scripts/draw_marker_borders.h"

#include "view/viewables/all_viewables_declaration.h"
#include "view/viewables/image_in_atlas.h"
#include "view/viewables/images_in_atlas_map.h"

#include "view/audiovisual_state/audiovisual_state.h"

#include "view/viewables/atlas_distributions.h"

#include "view/rendering_scripts/illuminated_rendering.h"

void illuminated_rendering(
	const illuminated_rendering_input in,
	const std::vector<additional_highlight>& additional_highlights
) {
	auto& profiler = in.frame_performance;

	auto& renderer = in.renderer;
	
	const auto viewed_character = in.camera.viewed_character;

	const auto cone = [&in](){ 
		auto result = in.camera.cone;
		result.eye.transform.pos.discard_fract();
		return result;
	}();

	const auto& cosmos = viewed_character.get_cosmos();
	
	const auto& anims = cosmos.get_logical_assets().plain_animations;

	const auto& av = in.audiovisuals;
	const auto& interp = av.get<interpolation_system>();
	const auto& particles = av.get<particles_simulation_system>();
	const auto& wandering_pixels = av.get<wandering_pixels_system>();
	const auto& exploding_rings = av.get<exploding_ring_system>();
	const auto& flying_numbers = av.get<flying_number_indicator_system>();
	const auto& highlights = av.get<pure_color_highlight_system>();
	const auto& thunders = av.get<thunder_system>();
	const auto global_time_seconds = cosmos.get_total_seconds_passed(in.interpolation_ratio);
	const auto settings = in.drawing;
	const auto matrix = cone.get_projection_matrix();
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

	auto total_particles_scope = measure_scope_additive(profiler.particles_rendering);
	auto total_layer_scope = measure_scope_additive(profiler.drawing_layers);

	auto draw_particles = [&](const particle_layer layer) {
		auto scope = measure_scope(total_particles_scope);

		particles.draw_particles(
			game_images,
			anims,
			draw_particles_in,
			layer
		);
	};

	draw_particles(particle_layer::DIM_SMOKES);
	draw_particles(particle_layer::ILLUMINATING_SMOKES);

	renderer.call_and_clear_triangles();

	fbos.illuminating_smoke->set_as_current();
	renderer.clear_current_fbo();

	draw_particles(particle_layer::ILLUMINATING_SMOKES);

	renderer.call_and_clear_triangles();
	
	renderer.set_standard_blending();

	augs::graphics::fbo::set_current_to_none();

	const auto& light = av.get<light_system>();
	
	const auto laser_glow = necessarys.at(assets::necessary_image_id::LASER_GLOW);
	const auto glow_edge_tex = necessarys.at(assets::necessary_image_id::LASER_GLOW_EDGE);

	const auto cast_highlight = necessarys.at(assets::necessary_image_id::CAST_HIGHLIGHT);

	const auto queried_cone = [&]() {
		auto c = cone;
		c.eye.zoom /= in.camera_query_mult;
		return c;
	}();

	const auto drawing_input = draw_renderable_input { 
		{
			output, 
			game_images, 
			global_time_seconds,
			flip_flags(),
			av.randomizing,
			queried_cone
		},
		interp
	};

	light.render_all_lights({
		renderer,
		profiler,
		total_layer_scope,
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
					[](const vec2, const vec2, const rgba) {},
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

			draw_beep_lights({
				output,
				interp,
				cosmos,
				cast_highlight
			})();

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
				cone
			);
		},
		cone,
		in.camera_query_mult,
		particles,
		anims,
		visible,
		drawing_input
	});

	set_shader_with_matrix(shaders.illuminated);

	const auto helper = helper_drawer {
		visible,
		cosmos,
		drawing_input,
		total_layer_scope
	};

	helper.draw<
		render_layer::UNDER_GROUND,
		render_layer::GROUND,
		render_layer::FLOOR_AND_ROAD,
		render_layer::ON_FLOOR,
		render_layer::ON_ON_FLOOR,

		render_layer::PLANTED_BOMBS,

		render_layer::AQUARIUM_FLOWERS,
		render_layer::AQUARIUM_DUNES,
		render_layer::BOTTOM_FISH,
		render_layer::UPPER_FISH,
		render_layer::AQUARIUM_BUBBLES
	>();

	visible.for_each<render_layer::DIM_WANDERING_PIXELS>(cosmos, [&](const auto e) {
		draw_wandering_pixels_as_sprites(wandering_pixels, e, game_images, drawing_input.make_input_for<invariants::sprite>());
	});

	renderer.call_and_clear_triangles();

	set_shader_with_matrix(shaders.specular_highlights);

	renderer.call_and_clear_triangles();

	shaders.illuminated->set_as_current();

	helper.draw<
		render_layer::WATER_COLOR_OVERLAYS,
		render_layer::WATER_SURFACES,
		render_layer::CAR_INTERIOR,
		render_layer::CAR_WHEEL
	>();

	renderer.call_and_clear_triangles();

	set_shader_with_matrix(shaders.pure_color_highlight);

	const auto timestamp_ms = static_cast<unsigned>(global_time_seconds * 1000);
	
	auto standard_border_provider = [timestamp_ms](const const_entity_handle sentience) {
		std::optional<rgba> result;

		sentience.dispatch_on_having_all<components::sentience>(
			[&](const auto typed_handle) {
				result = typed_handle.template get<components::sentience>().find_low_health_border(timestamp_ms);
			}
		);

		return result;
	};

	helper.draw_border<render_layer::SENTIENCES>(standard_border_provider);

	renderer.call_and_clear_triangles();

	if (settings.draw_area_markers) {
		visible.for_each<render_layer::AREA_MARKERS>(cosmos, [&](const auto e) {
			e.template dispatch_on_having_all<invariants::box_marker>([&](const auto typed_handle){ 
				const auto where = typed_handle.get_logic_transform();
				::draw_marker_borders(typed_handle, line_output, where, cone.eye.zoom);
			});
		});
	}

	if (viewed_character) {
		if (const auto sentience = viewed_character.find<components::sentience>()) {
			if (const auto sentience_def = viewed_character.find<invariants::sentience>()) {
				if (sentience->use_button == use_button_state::QUERYING) {
					if (const auto tr = viewed_character.find_viewing_transform(interp)) {
						const auto& a = sentience_def->use_button_angle;
						const auto& r = sentience_def->use_button_radius;

						if (r > 0.f) {
							const auto& p = tr->pos;
							const auto dash_len = 5.f;

							line_output.dashed_circular_sector(
								p,
								r,
								gray,
								tr->rotation,
								a,
								dash_len
							);
						}
					}
				}
			}
		}
	}

	renderer.call_and_clear_lines();

	shaders.illuminated->set_as_current();

	helper.draw<
		render_layer::DYNAMIC_BODY,
		render_layer::OVER_DYNAMIC_BODY,
		render_layer::SMALL_DYNAMIC_BODY,
		render_layer::GLASS_BODY,
		render_layer::SENTIENCES
	>();
	
	renderer.call_and_clear_triangles();

	renderer.set_active_texture(1);
	fbos.smoke->get_texture().bind();
	renderer.set_active_texture(0);

	shaders.smoke->set_as_current();

	renderer.fullscreen_quad();

	shaders.standard->set_as_current();
	
	helper.draw<
		render_layer::FLYING_BULLETS,
		render_layer::NEON_CAPTIONS
	>();
	
	if (settings.draw_crosshairs) {
		cosmos.template for_each_having<components::sentience>(
			[&](const auto it) {
				if (const auto s = it.find_crosshair_def()) {
					auto in = drawing_input.make_input_for<invariants::sprite>();
					
					in.global_time_seconds = global_time_seconds;
					in.renderable_transform = it.get_world_crosshair_transform(interp);

					augs::draw(s->appearance, game_images, in);
				}
			}
		);
	}
	
	if (settings.draw_weapon_laser && viewed_character.alive()) {
		const auto laser = necessarys.at(assets::necessary_image_id::LASER);
		
		draw_crosshair_lasers({
			line_output_wrapper { line_output, laser },
			dashed_line_output_wrapper  { line_output, laser, 10.f, 40.f, global_time_seconds },
			interp, 
			viewed_character
		});

		renderer.call_and_clear_lines();
	}

	draw_particles(particle_layer::ILLUMINATING_PARTICLES);

	visible.for_each<render_layer::ILLUMINATING_WANDERING_PIXELS>(cosmos, [&](const auto e) {
		draw_wandering_pixels_as_sprites(wandering_pixels, e, game_images, drawing_input.make_input_for<invariants::sprite>());
	});

	renderer.call_and_clear_triangles();

	set_shader_with_matrix(shaders.circular_bars);

	const auto set_center_uniform = [&](const augs::atlas_entry& tex) {
		shaders.circular_bars->set_uniform("texture_center", tex.get_center());
	};

	augs::vertex_triangle_buffer textual_infos;

	if (viewed_character) {
		const auto tex = necessarys.at(assets::necessary_image_id::CIRCULAR_BAR_LARGE);

		set_center_uniform(tex);

		textual_infos = draw_sentiences_hud({
			visible,
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
		auto draw_explosives_hud = [&](const auto tex_id, const auto type) {
			const auto tex = necessarys.at(tex_id);

			set_center_uniform(tex);

			draw_hud_for_explosives({
				output,
				renderer.get_special_buffer(),
				interp,
				cosmos,
				global_time_seconds,
				tex,
				type
			});

			renderer.call_and_clear_triangles();
		};

		draw_explosives_hud(
			assets::necessary_image_id::CIRCULAR_BAR_SMALL,
			circular_bar_type::SMALL
		);

		draw_explosives_hud(
			assets::necessary_image_id::CIRCULAR_BAR_MEDIUM,
			circular_bar_type::MEDIUM
		);

		draw_explosives_hud(
			assets::necessary_image_id::CIRCULAR_BAR_OVER_MEDIUM,
			circular_bar_type::OVER_MEDIUM
		);
	}

	shaders.standard->set_as_current();

	renderer.call_triangles(textual_infos);

	set_shader_with_matrix(shaders.exploding_rings);

	exploding_rings.draw_rings(
		output,
		renderer.specials,
		cosmos,
		cone
	);

	renderer.call_and_clear_triangles();

	shaders.pure_color_highlight->set_as_current();

	highlights.draw_highlights(cosmos, drawing_input);

	for (const auto& h : additional_highlights) {
		draw_color_highlight(cosmos[h.id], h.col, drawing_input);
	}

	thunders.draw_thunders(
		line_output
	);

	renderer.call_and_clear_triangles();
	renderer.call_and_clear_lines();

	shaders.standard->set_as_current();

	flying_numbers.draw_numbers(
		gui_font,
		output, 
		cone
	);

	renderer.call_and_clear_triangles();
	renderer.call_and_clear_lines();

	if (in.general_atlas) {
		in.general_atlas->bind();
	}
}
