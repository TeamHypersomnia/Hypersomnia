#include "augs/math/camera_cone.h"
#include "augs/math/matrix.h"

#include "augs/drawing/drawing.h"

#include "augs/graphics/renderer.h"
#include "augs/graphics/shader.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/enums/filters.h"

#include "view/frame_profiler.h"
#include "view/rendering_scripts/draw_entity.h"

#include "game/components/item_slot_transfers_component.h"
#include "game/debug_drawing_settings.h"
#include "game/detail/visible_entities.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"

#include "game/detail/inventory/inventory_slot_handle.h"

#include "view/rendering_scripts/rendering_scripts.h"
#include "view/rendering_scripts/illuminated_rendering.h"
#include "view/rendering_scripts/draw_wandering_pixels_as_sprites.h"
#include "view/rendering_scripts/helper_drawer.h"
#include "view/rendering_scripts/draw_area_indicator.h"

#include "view/viewables/all_viewables_declaration.h"
#include "view/viewables/image_in_atlas.h"
#include "view/viewables/images_in_atlas_map.h"
#include "game/stateless_systems/visibility_system.h"

#include "view/audiovisual_state/audiovisual_state.h"

#include "view/viewables/atlas_distributions.h"

#include "view/rendering_scripts/illuminated_rendering.h"
#include "game/detail/crosshair_math.hpp"

#include "application/performance_settings.h"
#include "augs/gui/text/printer.h"
#include "augs/math/simple_calculations.h"
#include "game/detail/sentience/callout_logic.h"
#include "view/rendering_scripts/draw_offscreen_indicator.h"
#include "game/detail/sentience/callout_logic.h"

#include "augs/graphics/shader.hpp"

void illuminated_rendering(const illuminated_rendering_input in) {
	const auto& additional_highlights = in.additional_highlights;
	const auto& special_indicators = in.special_indicators;
	augs::graphics::fbo::mark_current(in.renderer);

	auto& profiler = in.frame_performance;

	auto& renderer = in.renderer;

	using U = augs::common_uniform_name;

	auto set_uniform = [&](auto&& sh, auto&&... args) {
		sh->set_uniform(renderer, std::forward<decltype(args)>(args)...);
	};
	
	const auto viewed_character = in.camera.viewed_character;

	const auto cone = [&in](){ 
		auto result = in.camera.cone;
		result.eye.transform.pos.discard_fract();
		return result;
	}();

	const auto screen_size = cone.screen_size;

	const auto& cosm = viewed_character.get_cosmos();
	
	const auto& av = in.audiovisuals;
	const auto& interp = av.get<interpolation_system>();
	const auto& wandering_pixels = av.get<wandering_pixels_system>();
	const auto& exploding_rings = av.get<exploding_ring_system>();
	const auto& flying_numbers = av.get<flying_number_indicator_system>();
	const auto& highlights = av.get<pure_color_highlight_system>();
	const auto& thunders = av.get<thunder_system>();
	const auto global_time_seconds = cosm.get_total_seconds_passed(in.interpolation_ratio);
	const auto settings = in.drawing;
	const auto matrix = cone.get_projection_matrix();

	const bool is_zoomed_out = cone.eye.zoom < 1.f;

	auto non_zoomed_cone = cone;
	non_zoomed_cone.eye.zoom = 1.f;
	non_zoomed_cone.eye.transform.pos = screen_size / 2;

	const auto non_zoomed_matrix = non_zoomed_cone.get_projection_matrix();

	const auto& visible = in.all_visible;
	const auto& shaders = in.shaders;
	auto& fbos = in.fbos;
	const auto& necessarys = in.necessary_images;
	const auto& game_images = in.game_images;
	const auto blank = necessarys.at(assets::necessary_image_id::BLANK);
	const auto& gui_font = in.gui_font;

	auto get_drawer = [&]() {
		return augs::drawer_with_default { renderer.get_triangle_buffer(), blank };
	};

	auto get_line_drawer = [&]() {
		return augs::line_drawer_with_default{ renderer.get_line_buffer(), blank };
	};

	const auto viewed_character_transform = viewed_character ? viewed_character.find_viewing_transform(interp) : std::optional<transformr>();

	const auto filtering = in.renderer_settings.default_filtering;

	auto bind_and_set_filter = [&](auto& tex) {
		tex.set_as_current(renderer);
		tex.set_filtering(renderer, filtering);
	};

	if (in.general_atlas) {
		bind_and_set_filter(*in.general_atlas);
	}

	auto set_shader_with_matrix = [&](auto& shader) {
		shader->set_as_current(renderer);
		shader->set_projection(renderer, matrix);
	};

	auto set_shader_with_non_zoomed_matrix = [&](auto& shader) {
		shader->set_as_current(renderer);
		shader->set_projection(renderer, non_zoomed_matrix);
	};

	set_shader_with_matrix(shaders.standard);

	fbos.smoke->set_as_current(renderer);

	renderer.clear_current_fbo();
	renderer.set_additive_blending();
	
	auto total_particles_scope = measure_scope_additive(profiler.particles_rendering);
	auto total_layer_scope = measure_scope_additive(profiler.drawing_layers);

	auto draw_particles = [&](const particle_layer layer) {
		renderer.call_triangles(in.drawn_particles.diffuse[layer]);
	};

	auto draw_particles_neons = [&]() {
		renderer.call_triangles(in.drawn_particles.neons);
	};

	draw_particles(particle_layer::DIM_SMOKES);
	draw_particles(particle_layer::ILLUMINATING_SMOKES);

	renderer.call_and_clear_triangles();

	fbos.illuminating_smoke->set_as_current(renderer);
	renderer.clear_current_fbo();

	draw_particles(particle_layer::ILLUMINATING_SMOKES);

	renderer.call_and_clear_triangles();
	
	renderer.set_standard_blending();

	augs::graphics::fbo::set_current_to_marked(in.renderer);

	const auto& light = av.get<light_system>();
	
	const auto laser_glow = necessarys.at(assets::necessary_image_id::LASER_GLOW);
	const auto glow_edge_tex = necessarys.at(assets::necessary_image_id::LASER_GLOW_EDGE);

	const auto cast_highlight = necessarys.at(assets::necessary_image_id::CAST_HIGHLIGHT);

	const auto queried_cone = [&]() {
		auto c = cone;
		c.eye.zoom /= in.camera_query_mult;
		return c;
	}();

	auto make_drawing_input = [&]() {
		return draw_renderable_input { 
			{
				get_drawer(), 
				game_images, 
				global_time_seconds,
				flip_flags(),
				av.randomizing,
				queried_cone
			},
			interp
		};
	};

	const bool fog_of_war_effective = 
		viewed_character_transform != std::nullopt 
		&& settings.fog_of_war.is_enabled()
	;

	const messages::visibility_information_response* fow_response = nullptr;

#if BUILD_STENCIL_BUFFER
	auto fill_stencil = [&]() {
		if (fow_response != nullptr) {
			renderer.set_stencil(true);

			renderer.start_writing_stencil();
			renderer.set_clear_color(rgba(0, 0, 0, 0));
			renderer.clear_stencil();

			const auto eye_pos = viewed_character_transform->pos;
			const auto& r = *fow_response;

			for (std::size_t t = 0; t < r.get_num_triangles(); ++t) {
				const auto world_light_tri = r.get_world_triangle(t, eye_pos);
				augs::vertex_triangle renderable_light_tri;

				renderable_light_tri.vertices[0].pos = world_light_tri[0];
				renderable_light_tri.vertices[1].pos = world_light_tri[1];
				renderable_light_tri.vertices[2].pos = world_light_tri[2];

				renderable_light_tri.vertices[0].color = white;
				renderable_light_tri.vertices[1].color = white;
				renderable_light_tri.vertices[2].color = white;

				renderer.push_triangle(renderable_light_tri);
			}

			const auto& angle = settings.fog_of_war.angle;

			if (angle >= 360.f) {
				set_shader_with_matrix(shaders.pure_color_highlight);
			}
			else {
				set_shader_with_matrix(shaders.fog_of_war);

				auto dir = calc_crosshair_displacement(viewed_character) + in.pre_step_crosshair_displacement;

				if (dir.is_zero()) {
					dir.set(1, 0);
				}

				const auto left_dir = vec2(dir).rotate(-angle / 2).neg_y();
				const auto right_dir = vec2(dir).rotate(angle / 2).neg_y();

				const auto eye_frag_pos = [&]() {
					auto screen_space = cone.to_screen_space(eye_pos);	
					screen_space.y = screen_size.y - screen_space.y;
					return screen_space;
				}();

				set_uniform(shaders.fog_of_war, U::startingAngleVec, left_dir);
				set_uniform(shaders.fog_of_war, U::endingAngleVec, right_dir);
				set_uniform(shaders.fog_of_war, U::eye_frag_pos, eye_frag_pos);
			}

			renderer.call_and_clear_triangles();
			renderer.start_testing_stencil();
		}
	};
#else
	(void)fow_response;
	auto fill_stencil = [&]() {

	};
#endif

	const auto light_input = light_system_input {
		renderer,
		profiler,
		total_layer_scope,
		cosm, 
		matrix,
		fbos.light.value(),
		*shaders.light, 
		*shaders.textured_light, 
		*shaders.standard, 
		[&]() {
			draw_particles_neons();

			if (viewed_character) {
				draw_crosshair_lasers({
					[&](const vec2 from, const vec2 to, const rgba col) {
						if (!settings.draw_weapon_laser) {
							return;
						}

						const vec2 edge_size = static_cast<vec2>(glow_edge_tex.get_original_size());

						get_drawer().line(laser_glow, from, to, edge_size.y / 3.f, col);

						const auto edge_dir = (to - from).normalize();
						const auto edge_offset = edge_dir * edge_size.x;

						get_drawer().line(glow_edge_tex, to, to + edge_offset, edge_size.y / 3.f, col);
						get_drawer().line(glow_edge_tex, from - edge_offset + edge_dir, from + edge_dir, edge_size.y / 3.f, col, flip_flags::make_horizontally());
					},
					[](const vec2, const vec2, const rgba) {},
					interp,
					viewed_character,
					in.pre_step_crosshair_displacement
				});
			}

			draw_explosion_body_highlights({
				get_drawer(),
				queried_cone,
				interp,
				cosm,
				global_time_seconds,
				cast_highlight
			});

			draw_beep_lights({
				get_drawer(),
				interp,
				cosm,
				cast_highlight
			})();

			renderer.set_active_texture(3);
			bind_and_set_filter(fbos.illuminating_smoke->get_texture());
			renderer.set_active_texture(0);

			shaders.illuminating_smoke->set_as_current(renderer);

			renderer.fullscreen_quad();

			shaders.standard->set_as_current(renderer);

			exploding_rings.draw_highlights_of_rings(
				get_drawer(),
				cast_highlight,
				queried_cone
			);
		},
		fill_stencil,
		cone,
		fog_of_war_effective ? viewed_character : std::optional<entity_id>(),
		in.camera_query_mult,
		visible,
		cast_highlight,
		make_drawing_input,
		in.perf_settings
	};

	auto& vis_requests = fresh_thread_local_visibility_requests();
	auto& vis_responses = thread_local_visibility_responses();

	light.gather_vis_requests(light_input);

	auto invoke_visibility_calculations = [&]() {
		auto scope = measure_scope(profiler.light_visibility);
		auto light_raycasts_scope = cosm.measure_raycasts(profiler.visibility_raycasts);

		visibility_system(DEBUG_FRAME_LINES).calc_visibility(cosm, vis_requests, vis_responses, in.perf_settings);
	};

#if BUILD_STENCIL_BUFFER
	if (fog_of_war_effective) {
		const auto fow_size = settings.fog_of_war.get_real_size();

		messages::visibility_information_request request;
		request.eye_transform = *viewed_character_transform;
		request.filter = predefined_queries::line_of_sight();
		request.queried_rect = fow_size;
		request.subject = viewed_character;

		vis_requests.push_back(request);
	}

	invoke_visibility_calculations();

	if (fog_of_war_effective) {
		const auto fow_response_index = vis_requests.size() - 1;

		if (vis_responses.size() > fow_response_index) {
			const auto& fow_resp = vis_responses[fow_response_index];

		   	if (!fow_resp.empty()) {
				fow_response = std::addressof(fow_resp);
			}
		}

		renderer.call_and_clear_triangles();
		fill_stencil();
		renderer.set_stencil(false);
	}
#else
	invoke_visibility_calculations();
#endif

	light.render_all_lights(light_input);

	set_shader_with_matrix(shaders.illuminated);

	auto make_helper = [&]() {
		return helper_drawer {
			visible,
			cosm,
			make_drawing_input(),
			total_layer_scope
		};
	};

	make_helper().draw<
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

	{
		auto drawing_input = make_drawing_input();

		visible.for_each<render_layer::DIM_WANDERING_PIXELS>(cosm, [&](const auto e) {
			draw_wandering_pixels_as_sprites(wandering_pixels, e, game_images, drawing_input.make_input_for<invariants::sprite>());
		});
	}

	renderer.call_and_clear_triangles();

	set_shader_with_matrix(shaders.specular_highlights);

	renderer.call_and_clear_triangles();

	shaders.illuminated->set_as_current(renderer);

	make_helper().draw<
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

#if BUILD_STENCIL_BUFFER
	if (fog_of_war_effective) {
		renderer.call_and_clear_triangles();

		renderer.set_stencil(true);
		renderer.stencil_positive_test();

		{
			auto drawing_input = make_drawing_input();

			visible.for_each<render_layer::SENTIENCES>(cosm, [&](const auto handle) {
				if (handle.get_official_faction() != viewed_character.get_official_faction()) {
					::draw_border(handle, drawing_input, standard_border_provider);
				}
			});
		}

		renderer.call_and_clear_triangles();
		renderer.set_stencil(false);

		auto drawing_input = make_drawing_input();

		visible.for_each<render_layer::SENTIENCES>(cosm, [&](const auto handle) {
			if (handle.get_official_faction() == viewed_character.get_official_faction()) {
				::draw_border(handle, drawing_input, standard_border_provider);
			}
		});
	}
	else {
		make_helper().draw_border<render_layer::SENTIENCES>(standard_border_provider);
	}
#else
	make_helper().draw_border<render_layer::SENTIENCES>(standard_border_provider);
#endif

	renderer.call_and_clear_triangles();

	{
		const auto& markers = settings.draw_area_markers;

		if (markers.is_enabled) {
			visible.for_each<render_layer::AREA_MARKERS>(cosm, [&](const auto e) {
				e.template dispatch_on_having_all<invariants::box_marker>([&](const auto typed_handle) { 
					const auto where = typed_handle.get_logic_transform();
					const auto& marker_alpha = markers.value;
					::draw_area_indicator(typed_handle, get_line_drawer(), where, marker_alpha, drawn_indicator_type::INGAME);
				});
			});
		}
	}

	if (viewed_character) {
		if (const auto sentience = viewed_character.find<components::sentience>()) {
			if (const auto sentience_def = viewed_character.find<invariants::sentience>()) {
				if (sentience->use_button != use_button_state::IDLE) {
					if (const auto tr = viewed_character.find_viewing_transform(interp)) {
						const auto& a = sentience_def->use_button_angle;
						const auto& r = sentience_def->use_button_radius;

						const bool is_querying = sentience->use_button == use_button_state::QUERYING;
						auto col = is_querying ? gray : rgba(255, 0, 0, 180);

						if (is_querying && sentience->last_use_result == use_button_query_result::IN_RANGE_BUT_CANT) {
							col = green;
						}

						if (r > 0.f) {
							const auto& p = tr->pos;
							const auto dash_len = 5.f;

							get_line_drawer().dashed_circular_sector(
								p,
								r,
								col,
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

	shaders.illuminated->set_as_current(renderer);

	make_helper().draw<
		render_layer::DYNAMIC_BODY,
		render_layer::OVER_DYNAMIC_BODY,
		render_layer::SMALL_DYNAMIC_BODY,
		render_layer::OVER_SMALL_DYNAMIC_BODY,
		render_layer::GLASS_BODY
	>();
	

#if BUILD_STENCIL_BUFFER
	if (fog_of_war_effective) {
		const auto& appearance = settings.fog_of_war_appearance;

		renderer.call_and_clear_triangles();
		renderer.set_stencil(true);

		if (appearance.overlay_color_on_visible) {
			renderer.stencil_positive_test();
		}
		else {
			renderer.stencil_reverse_test();
		}

		set_shader_with_matrix(shaders.pure_color_highlight);

		const auto fow_size = settings.fog_of_war.get_real_size();

		get_drawer().aabb(
			ltrb::center_and_size(viewed_character_transform->pos, fow_size),
			appearance.overlay_color
		);

		renderer.call_and_clear_triangles();
		shaders.pure_color_highlight->set_projection(renderer, matrix);

		renderer.stencil_positive_test();

		shaders.illuminated->set_as_current(renderer);

		{
			auto drawing_input = make_drawing_input();

			visible.for_each<render_layer::SENTIENCES>(cosm, [&](const auto handle) {
				if (handle.get_official_faction() != viewed_character.get_official_faction()) {
					::draw_entity(handle, drawing_input);
				}
			});
		}

		renderer.call_and_clear_triangles();
		renderer.set_stencil(false);

		auto drawing_input = make_drawing_input();

		visible.for_each<render_layer::SENTIENCES>(cosm, [&](const auto handle) {
			if (handle.get_official_faction() == viewed_character.get_official_faction()) {
				::draw_entity(handle, drawing_input);
			}
		});
	}
	else {
		make_helper().draw<render_layer::SENTIENCES>();
	}
#else
	make_helper().draw<render_layer::SENTIENCES>();
#endif

	make_helper().draw<
		render_layer::INSECTS,
		render_layer::OVER_SENTIENCES
	>();

	renderer.call_and_clear_triangles();

	renderer.set_active_texture(1);

	bind_and_set_filter(fbos.smoke->get_texture());
	renderer.set_active_texture(0);

	shaders.smoke->set_as_current(renderer);

	renderer.fullscreen_quad();

	shaders.standard->set_as_current(renderer);
	
	make_helper().draw<
		render_layer::FLYING_BULLETS,
		render_layer::NEON_CAPTIONS
	>();
	
	if (settings.draw_crosshairs) {
		auto draw_crosshair = [&](const auto it) {
			if (const auto s = it.find_crosshair_def()) {
				auto drawing_input = make_drawing_input();
				auto sprite_in = drawing_input.make_input_for<invariants::sprite>();

				sprite_in.global_time_seconds = global_time_seconds;
				sprite_in.renderable_transform = it.get_world_crosshair_transform(interp) + in.pre_step_crosshair_displacement;

				if (is_zoomed_out) {
					sprite_in.renderable_transform.pos = cone.to_screen_space(sprite_in.renderable_transform.pos);
				}

				augs::draw(s->appearance, game_images, sprite_in);
			}
		};

		if (viewed_character) {
			if (is_zoomed_out) {
				renderer.call_and_clear_triangles();
				shaders.standard->set_projection(renderer, non_zoomed_matrix);
			}

			draw_crosshair(viewed_character);

			if (is_zoomed_out) {
				renderer.call_and_clear_triangles();
				shaders.standard->set_projection(renderer, matrix);
			}
		}
	}
	
	if (settings.draw_weapon_laser && viewed_character.alive()) {
		const auto laser = necessarys.at(assets::necessary_image_id::LASER);
		
		draw_crosshair_lasers({
			line_output_wrapper { get_line_drawer(), laser },
			dashed_line_output_wrapper  { get_line_drawer(), laser, 10.f, 40.f, global_time_seconds },
			interp, 
			viewed_character,
			in.pre_step_crosshair_displacement
		});

		renderer.call_and_clear_lines();
	}

	draw_particles(particle_layer::NEONING_PARTICLES);
	draw_particles(particle_layer::ILLUMINATING_PARTICLES);

	{
		auto drawing_input = make_drawing_input();

		visible.for_each<render_layer::ILLUMINATING_WANDERING_PIXELS>(cosm, [&](const auto e) {
			draw_wandering_pixels_as_sprites(wandering_pixels, e, game_images, drawing_input.make_input_for<invariants::sprite>());
		});
	}


	renderer.call_and_clear_triangles();

	set_shader_with_matrix(shaders.circular_bars);

	const auto set_center_uniform = [&](const augs::atlas_entry& tex) {
		set_uniform(shaders.circular_bars, U::texture_center, tex.get_center());
	};

	using D = augs::dedicated_triangle_buffer;

	auto& nicknames_output = renderer.dedicated[D::NICKNAMES];
	auto& health_numbers_output = renderer.dedicated[D::HEALTH_NUMBERS];
	auto& indicators_output = renderer.dedicated[D::INDICATORS];

	if (viewed_character) {
		auto make_input_for = [&](const auto& tex_type, const auto meter) {
			const auto tex = necessarys.at(tex_type);
			const bool is_health = meter == meter_id::of<health_meter_instance>();
			const bool draw_other_indicators = is_health;

			set_center_uniform(tex);

			real32 color_indicator_angle = 45.f;

			if (!settings.draw_nicknames) {
				color_indicator_angle = 0.f;
			}

			auto sentience_queried_cone = queried_cone;
			sentience_queried_cone.eye.zoom *= 0.8f;

			return draw_sentiences_hud_input {
				nicknames_output,
				health_numbers_output,
				indicators_output,
				cone,
				sentience_queried_cone,
				visible,
				settings,
				get_drawer(),
				renderer.get_special_buffer(),
				cosm,
				viewed_character,
				interp,
				global_time_seconds,
				gui_font,
				tex,
				meter,

				draw_other_indicators,

				necessarys.at(assets::necessary_image_id::BIG_COLOR_INDICATOR),
				necessarys.at(assets::necessary_image_id::DANGER_INDICATOR),
				necessarys.at(assets::necessary_image_id::DEATH_INDICATOR),
				necessarys.at(assets::necessary_image_id::BOMB_INDICATOR),
				necessarys.at(assets::necessary_image_id::DEFUSING_INDICATOR),

				color_indicator_angle,
				in.indicator_meta
			};
		};

		std::array<assets::necessary_image_id, 3> circles = {
			assets::necessary_image_id::CIRCULAR_BAR_LARGE,
			assets::necessary_image_id::CIRCULAR_BAR_UNDER_LARGE,
			assets::necessary_image_id::CIRCULAR_BAR_UNDER_UNDER_LARGE
		};

		int current_circle = 0;

		draw_sentiences_hud(
			make_input_for(
				circles[current_circle++], 
				meter_id::of<health_meter_instance>()
			)
		);

		renderer.call_and_clear_triangles();

		if (settings.draw_pe_bar) {
			draw_sentiences_hud(
				make_input_for(
					circles[current_circle++], 
					meter_id::of<personal_electricity_meter_instance>()
				)
			);

			renderer.call_and_clear_triangles();
		}

		if (settings.draw_cp_bar) {
			draw_sentiences_hud(
				make_input_for(
					circles[current_circle++], 
					meter_id::of<consciousness_meter_instance>()
				)
			);

			renderer.call_and_clear_triangles();
		}
	}
	
	{
		auto draw_explosives_hud = [&](const auto tex_id, const auto type) {
			const auto tex = necessarys.at(tex_id);

			set_center_uniform(tex);

			draw_hud_for_explosives({
				get_drawer(),
				renderer.get_special_buffer(),
				settings,
				interp,
				cosm,
				viewed_character,
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

	set_shader_with_non_zoomed_matrix(shaders.standard);

	if (settings.draw_nicknames) {
		renderer.call_triangles(nicknames_output);
	}

	if (settings.draw_health_numbers) {
		renderer.call_triangles(health_numbers_output);
	}

	renderer.call_triangles(indicators_output);

	if (settings.draw_tactical_indicators.is_enabled) {
		const auto alpha = settings.draw_tactical_indicators.value;

		for (const auto& special : special_indicators) {
			auto col = special.color;
			col.mult_alpha(alpha);

			const auto world_pos = special.transform.pos;
			const auto pos = cone.to_screen_space(world_pos);

			std::string primary_text;

			if (const auto callout = cosm[::get_current_callout(cosm, world_pos)]) {
				primary_text = callout.get_name();
			}

			const auto& tex = special.offscreen_tex;

			::draw_offscreen_indicator(
				get_drawer(),
				settings.draw_offscreen_indicators,
				special.draw_onscreen,
				col,
				tex,
				pos,
				screen_size,
				false,
				std::nullopt,
				in.gui_font,
				primary_text,
				{},
				cone.to_screen_space(viewed_character.get_viewing_transform(interp).pos),
				settings.offscreen_reference_mode
			);
		}
	}

	{
		const auto& callouts = settings.draw_callout_indicators;

		if (callouts.is_enabled) {
			visible.for_each<render_layer::CALLOUT_MARKERS, render_layer::OVERLAID_CALLOUT_MARKERS>(cosm, [&](const auto e) {
				e.template dispatch_on_having_all<invariants::box_marker>([&](const auto typed_handle) { 
					const auto where = typed_handle.get_logic_transform();
					const auto& callout_alpha = callouts.value;

					::draw_area_indicator(typed_handle, get_line_drawer(), where, callout_alpha, drawn_indicator_type::INGAME);

					using namespace augs::gui::text;

					const auto& callout_name = typed_handle.get_name();

					print_stroked(
						get_drawer(),
						cone.to_screen_space(typed_handle.get_logic_transform().pos),
						formatted_string { callout_name, { gui_font, white } },
						{ augs::ralign::CX, augs::ralign::CY }
					);
				});
			});

			renderer.call_and_clear_triangles();
		}
	}

	if (settings.print_current_character_callout) {
		if (const auto result = cosm[::get_current_callout(viewed_character, interp)]) {
			const auto& name = result.get_name();

			using namespace augs::gui::text;

			const auto indicator_pos = augs::get_screen_pos_from_offset(screen_size, settings.radar_pos); 

			print_stroked(
				get_drawer(),
				indicator_pos,
				formatted_string { name, { gui_font, yellow } }
			);
		}

		renderer.call_and_clear_triangles();
	}

	set_shader_with_matrix(shaders.exploding_rings);

	exploding_rings.draw_rings(
		get_drawer(),
		renderer.get_special_buffer(),
		queried_cone
	);

	renderer.call_and_clear_triangles();

	shaders.pure_color_highlight->set_as_current(renderer);

	{
		auto drawing_input = make_drawing_input();
		highlights.draw_highlights(cosm, drawing_input);

		for (const auto& h : additional_highlights) {
			draw_color_highlight(cosm[h.id], h.col, drawing_input);
		}
	}

	thunders.draw_thunders(
		get_line_drawer(),
		queried_cone
	);

	renderer.call_and_clear_triangles();
	renderer.call_and_clear_lines();

	shaders.standard->set_as_current(renderer);

	flying_numbers.draw_numbers(
		gui_font,
		get_drawer(), 
		cone
	);

	renderer.call_and_clear_triangles();
	renderer.call_and_clear_lines();

	shaders.standard->set_projection(renderer, matrix);

	if (in.general_atlas) {
		in.general_atlas->set_as_current(renderer);
	}
}
