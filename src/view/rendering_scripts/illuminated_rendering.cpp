#include "augs/math/camera_cone.h"
#include "augs/math/matrix.h"

#include "augs/drawing/drawing.hpp"

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
#include "application/main/cached_visibility_data.h"
#include "augs/templates/thread_pool.h"
#include "game/detail/get_hovered_world_entity.h"

#include "view/rendering_scripts/enqueue_illuminated_rendering_jobs.hpp"

void illuminated_rendering(const illuminated_rendering_input in) {
	using U = augs::common_uniform_name;
	using D = augs::dedicated_buffer;
	using DV = augs::dedicated_buffer_vector;

	const auto& av = in.audiovisuals;
	const auto& interp = av.get<interpolation_system>();

	const auto viewed_character = in.camera.viewed_character;
	const auto viewed_character_transform = viewed_character ? viewed_character.find_viewing_transform(interp) : std::optional<transformr>();
	const auto& cosm = viewed_character.get_cosmos();

	const auto cone = in.camera.cone;
	const auto screen_size = cone.screen_size;
	
	const auto& light = av.get<light_system>();
	const auto& exploding_rings = av.get<exploding_ring_system>();
	const auto& damage_indication = av.get<damage_indication_system>();
	const auto& highlights = av.get<pure_color_highlight_system>();
	const auto global_time_seconds = cosm.get_total_seconds_passed(in.interpolation_ratio);
	const auto settings = in.drawing;
	const auto matrix = cone.get_projection_matrix();

	//const bool is_zoomed_out = cone.eye.zoom < 1.f;

	const auto non_zoomed_matrix = [&]() {
		auto non_zoomed_cone = cone;
		non_zoomed_cone.eye.zoom = 1.f;
		non_zoomed_cone.eye.transform.pos = screen_size / 2;

		return non_zoomed_cone.get_projection_matrix();
	}();

	const auto& visible = in.all_visible;
	const auto& shaders = in.shaders;
	auto& fbos = in.fbos;
	const auto& necessarys = in.necessary_images;
	const auto& game_images = in.game_images;
	const auto blank = necessarys.at(assets::necessary_image_id::BLANK);

	const auto queried_cone = in.queried_cone;

	auto& profiler = in.frame_performance;
	auto& renderer = in.renderer;

	/* Shader manipulation lambdas */

	auto set_uniform = [&](auto&& sh, auto&&... args) {
		sh->set_uniform(renderer, std::forward<decltype(args)>(args)...);
	};

	auto set_shader_with_matrix = [&](auto& shader) {
		shader->set_as_current(renderer);
		shader->set_projection(renderer, matrix);
	};

	auto set_shader_with_non_zoomed_matrix = [&](auto& shader) {
		shader->set_as_current(renderer);
		shader->set_projection(renderer, non_zoomed_matrix);
	};

	auto bind_and_update_filtering = [&](auto& tex) {
		const auto filtering = in.renderer_settings.default_filtering;

		tex.set_as_current(renderer);
		tex.set_filtering(renderer, filtering);
	};

	/* Output buffer getters */

	auto get_drawer = [&]() {
		return augs::drawer_with_default { renderer.get_triangle_buffer(), blank };
	};

	auto get_line_drawer = [&]() {
		return augs::line_drawer_with_default{ renderer.get_line_buffer(), blank };
	};

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

	/* Drawing lambdas */

	auto draw_particles = [&](const particle_layer layer) {
		renderer.call_triangles_direct_ptr(in.drawn_particles.diffuse[layer]);
	};

	auto draw_particles_neons = [&]() {
		renderer.call_triangles_direct_ptr(in.drawn_particles.neons);
	};

#if BUILD_STENCIL_BUFFER
	const bool fog_of_war_effective = 
		viewed_character_transform.has_value() 
		&& settings.fog_of_war.is_enabled()
	;
#else
	const bool fog_of_war_effective = false;
#endif

	auto write_fow_to_stencil = [&]() {
		if (viewed_character.dead()) {
			return;
		}

		renderer.set_stencil(true);

		renderer.start_writing_stencil();
		renderer.set_clear_color(rgba(0, 0, 0, 0));
		renderer.clear_stencil();

		const auto eye_pos = viewed_character_transform->pos;

		set_shader_with_matrix(shaders.fog_of_war);

		auto dir = calc_crosshair_displacement(viewed_character) + in.pre_step_crosshair_displacement;

		if (dir.is_zero()) {
			dir.set(1, 0);
		}

		const auto& angle = settings.fog_of_war.angle;
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

		renderer.call_triangles(D::FOG_OF_WAR);

		renderer.finish_writing_stencil();
		renderer.set_stencil(false);
	};

	auto draw_sentience_borders = [&]() {
		if (fog_of_war_effective) {
			renderer.set_stencil(true);
			renderer.stencil_positive_test();

			renderer.call_triangles(D::BORDERS_ENEMY_SENTIENCES);
			renderer.set_stencil(false);

			renderer.call_triangles(D::BORDERS_FRIENDLY_SENTIENCES);
		}
		else {
			renderer.call_triangles(D::BORDERS_FRIENDLY_SENTIENCES);
		}

		renderer.call_and_clear_triangles();
	};

	auto draw_sentiences = [&](auto& shader) {
		if (fog_of_war_effective) {
			renderer.stencil_positive_test();

			shader.set_as_current(renderer);

			renderer.set_stencil(true);
			renderer.call_triangles(D::ENEMY_SENTIENCES);
			renderer.set_stencil(false);

			renderer.call_triangles(D::FRIENDLY_SENTIENCES);
		}
		else {
			renderer.call_triangles(D::FRIENDLY_SENTIENCES);
		}
	};

	auto draw_sentience_HUDs = [&]() {
		if (settings.cinematic_mode) {
			return;
		}

		set_shader_with_matrix(shaders.circular_bars);

		const auto set_center_uniform = [&](const auto& tex_id) {
			set_uniform(shaders.circular_bars, U::texture_center, necessarys[tex_id].get_center());
		};

		if (viewed_character) {
			std::array<assets::necessary_image_id, 3> circles = {
				assets::necessary_image_id::CIRCULAR_BAR_LARGE,
				assets::necessary_image_id::CIRCULAR_BAR_UNDER_LARGE,
				assets::necessary_image_id::CIRCULAR_BAR_UNDER_UNDER_LARGE
			};

			int current_circle = 0;

			if (settings.draw_hp_bar) {
				set_center_uniform(circles[current_circle++]);
				renderer.call_triangles(DV::SENTIENCE_HUDS, 0);
			}

			if (settings.draw_pe_bar) {
				set_center_uniform(circles[current_circle++]);
				renderer.call_triangles(DV::SENTIENCE_HUDS, 1);
			}

			if (settings.draw_cp_bar) {
				set_center_uniform(circles[current_circle++]);
				renderer.call_triangles(DV::SENTIENCE_HUDS, 2);
			}
		}

		{
			int current_hud = 0;

			auto draw_explosives_hud = [&](const auto tex_id) {
				set_center_uniform(tex_id);
				renderer.call_triangles(DV::EXPLOSIVE_HUDS, current_hud++);
			};

			draw_explosives_hud(assets::necessary_image_id::CIRCULAR_BAR_SMALL);
			draw_explosives_hud(assets::necessary_image_id::CIRCULAR_BAR_MEDIUM);
			draw_explosives_hud(assets::necessary_image_id::CIRCULAR_BAR_OVER_MEDIUM);
		}
	};

	auto draw_fog_of_war_overlay = [&]() {
		if (!fog_of_war_effective) {
			return;
		}

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
		renderer.set_stencil(false);
	};

	auto draw_interaction_sensor = [&]() {
		if (!viewed_character) {
			return;
		}

		if (const auto sentience = viewed_character.find<components::sentience>()) {
			if (const auto sentience_def = viewed_character.find<invariants::sentience>()) {
				if (sentience->is_requesting_interaction) {
					if (const auto tr = viewed_character.find_viewing_transform(interp)) {
						auto col = gray;

						switch (sentience->last_interaction_result) {
							case interaction_result_type::IN_RANGE_BUT_CANT:
								col = yellow;
								break;
							case interaction_result_type::CAN_BEGIN:
							case interaction_result_type::IN_PROGRESS:
								col = green;
								break;
							default:
								break;
						}

						const auto& a = sentience_def->interaction_hitbox_range;
						const auto& r = sentience_def->interaction_hitbox_radius;

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
	};

	auto draw_area_markers = [&]() {
		const auto& markers = settings.draw_area_markers;

		if (!markers.is_enabled) {
			return;
		}

		visible.for_each<render_layer::AREA_MARKERS, render_layer::AREA_SENSORS>(cosm, [&](const auto e) {
			e.template dispatch_on_having_all<invariants::area_marker>([&](const auto typed_handle) { 
				const auto where = typed_handle.get_logic_transform();
				const auto& marker_alpha = markers.value;
				::draw_area_indicator(typed_handle, get_line_drawer(), where, marker_alpha, drawn_indicator_type::INGAME, 1.0f);
			});
		});
	};

	auto draw_crosshairs = [&]() {
		if (!settings.draw_crosshairs) {
			return;
		}

		auto draw_crosshair = [&](const auto it) {
			//const vec2 screen_space_pos = screen_size / 2 + ::calc_crosshair_displacement(it) + in.pre_step_crosshair_displacement;
			//LOG_NVPS(screen_space_pos.x, screen_space_pos.y, screen_size.x);

			const vec2 world_space_pos = it.get_world_crosshair_transform(interp, false).pos + in.pre_step_crosshair_displacement;
			const vec2 projection_space_pos = cone.to_screen_space(world_space_pos);

			float maximum_heat = 0.0f;

			for (const auto& gun_id : it.get_wielded_guns())
			{
				maximum_heat = std::max(maximum_heat, cosm[gun_id].template get<components::gun>().recoil.pattern_progress);
			}

			::draw_crosshair_procedurally(settings.crosshair, get_drawer(), projection_space_pos, maximum_heat);
		};

		if (viewed_character) {
			//if (is_zoomed_out)
			{
				renderer.call_and_clear_triangles();
				shaders.standard->set_projection(renderer, non_zoomed_matrix);
			}

			draw_crosshair(viewed_character);

			//if (is_zoomed_out)
			{
				renderer.call_and_clear_triangles();
				shaders.standard->set_projection(renderer, matrix);
			}
		}
	};

	auto draw_weapon_laser = [&]() {
		if (settings.draw_weapon_laser && viewed_character.alive()) {
			const auto laser = necessarys.at(assets::necessary_image_id::LASER);
			
			draw_crosshair_lasers({
				line_output_wrapper { get_line_drawer(), laser },
				dashed_line_output_wrapper  { get_line_drawer(), laser, 10.f, 40.f, global_time_seconds },
				interp, 
				viewed_character,
				in.pre_step_crosshair_displacement,
				screen_size
			});

			renderer.call_and_clear_lines();
		}
	};

	auto draw_pure_color_highlights = [&]() {
		shaders.pure_color_highlight->set_as_current(renderer);

		auto drawing_input = make_drawing_input();
		highlights.draw_highlights(cosm, drawing_input);

		for (const auto& h : in.additional_highlights) {
			draw_color_highlight(cosm[h.id], h.col, drawing_input);
		}

		renderer.call_and_clear_triangles();
	};

	auto neon_occlusion_callback = [&](bool is_foreground) {
		auto& shader = shaders.neon_occluder;

		set_shader_with_matrix(shader);

		const auto& ambient_color = cosm.get_common_significant().light.ambient_color;
		set_uniform(shader, U::global_color, ambient_color);

		if (!is_foreground) {
			if (settings.occlude_neons_under_sentiences) {
				draw_sentiences(*shader);
			}
		}
	};

	auto light_pass = [&]() {
		const auto cast_highlight = necessarys.at(assets::necessary_image_id::CAST_HIGHLIGHT);

		const auto light_input = light_system_input {
			renderer,
			profiler,
			cosm, 
			matrix,
			fbos.light.value(),
			*shaders.light, 
			*shaders.textured_light, 
			*shaders.standard, 
			neon_occlusion_callback,
			[&]() {
				draw_particles_neons();

				if (viewed_character) {
					const auto laser_glow = necessarys.at(assets::necessary_image_id::LASER_GLOW);
					const auto glow_edge_tex = necessarys.at(assets::necessary_image_id::LASER_GLOW_EDGE);

					draw_crosshair_lasers({
						[&](const vec2 from, const vec2 to, rgba col) {
							if (!settings.draw_weapon_laser) {
								return;
							}

							col.mult_alpha(0.5f);

							const vec2 edge_size = static_cast<vec2>(glow_edge_tex.get_original_size());

							get_drawer().line(laser_glow, from, to, edge_size.y / 5.f, col);

							const auto edge_dir = (to - from).normalize();
							const auto edge_offset = edge_dir * edge_size.x;

							get_drawer().line(glow_edge_tex, to, to + edge_offset, edge_size.y / 3.f, col);
							get_drawer().line(glow_edge_tex, from - edge_offset + edge_dir, from + edge_dir, edge_size.y / 5.f, col, flip_flags::make_horizontally());
						},
						[](const vec2, const vec2, const rgba) {},
						interp,
						viewed_character,
						in.pre_step_crosshair_displacement,
						screen_size
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
				bind_and_update_filtering(fbos.illuminating_smoke->get_texture());
				renderer.set_active_texture(0);

				shaders.illuminating_smoke->set_as_current(renderer);

				renderer.fullscreen_quad();

				shaders.standard->set_as_current(renderer);

				exploding_rings.draw_highlights_of_explosions(
					get_drawer(),
					cast_highlight,
					queried_cone
				);

				exploding_rings.draw_highlights_of_continuous_rings(
					cosm,
					get_drawer(),
					cast_highlight,
					queried_cone
				);
			},
			write_fow_to_stencil,
			cone,
			fog_of_war_effective ? viewed_character : std::optional<entity_id>(),
			in.queried_cone,
			visible,
			cast_highlight,
			make_drawing_input,
			in.perf_settings,
			in.light_requests
		};

		light.render_all_lights(light_input);
	};

	auto overlay_smoke_texture = [&]() {
		renderer.set_active_texture(1);
		bind_and_update_filtering(fbos.smoke->get_texture());
		renderer.set_active_texture(0);

		shaders.smoke->set_as_current(renderer);

		renderer.fullscreen_quad();
	};

	/* Flow */

	if (in.general_atlas) {
		bind_and_update_filtering(*in.general_atlas);
	}

	augs::graphics::fbo::mark_current(in.renderer);

	set_shader_with_matrix(shaders.standard);

	fbos.smoke->set_as_current(renderer);

	renderer.clear_current_fbo();
	renderer.set_additive_blending();

	draw_particles(particle_layer::DIM_SMOKES);
	draw_particles(particle_layer::ILLUMINATING_SMOKES);

	renderer.call_and_clear_triangles();

	fbos.illuminating_smoke->set_as_current(renderer);
	renderer.clear_current_fbo();

	draw_particles(particle_layer::ILLUMINATING_SMOKES);

	renderer.call_and_clear_triangles();
	
	renderer.set_standard_blending();

	augs::graphics::fbo::set_current_to_marked(in.renderer);

	if (fog_of_war_effective) {
		renderer.call_and_clear_triangles();
		write_fow_to_stencil();
	}

	light_pass();

	set_shader_with_matrix(shaders.illuminated);

	renderer.call_triangles(D::GROUND);
	renderer.call_triangles(D::DIM_WANDERING_PIXELS);

	{
		set_shader_with_matrix(shaders.pure_color_highlight);

		draw_sentience_borders();
		draw_area_markers();
		draw_interaction_sensor();

		renderer.call_and_clear_lines();
	}

	shaders.illuminated->set_as_current(renderer);

	renderer.call_triangles(D::SOLID_OBSTACLES);
	renderer.call_triangles(D::REMNANTS);

	set_shader_with_matrix(shaders.pure_color_highlight);
	renderer.call_triangles(D::DROPPED_ITEMS_SHADOWS);
	renderer.call_triangles(D::DROPPED_ITEMS_BORDERS);

	set_shader_with_matrix(shaders.illuminated);
	renderer.call_triangles(D::DROPPED_ITEMS_DIFFUSE);

	set_shader_with_matrix(shaders.pure_color_highlight);
	renderer.call_triangles(D::DROPPED_ITEMS_OVERLAYS);

	set_shader_with_matrix(shaders.illuminated);

	draw_fog_of_war_overlay();
	draw_sentiences(*shaders.illuminated);

	renderer.call_triangles(D::FOREGROUND);

	overlay_smoke_texture();
	
	shaders.standard->set_as_current(renderer);
	renderer.call_triangles(D::FOREGROUND_GLOWS);

	draw_crosshairs();
	draw_weapon_laser();

	draw_particles(particle_layer::NEONING_PARTICLES);
	draw_particles(particle_layer::ILLUMINATING_PARTICLES);

	renderer.call_triangles(D::ILLUMINATING_WANDERING_PIXELS);

	draw_sentience_HUDs();

	set_shader_with_non_zoomed_matrix(shaders.standard);

	if (settings.draw_nicknames) {
		renderer.call_triangles(D::NICKNAMES);
	}

	if (settings.draw_health_numbers) {
		renderer.call_triangles(D::HEALTH_NUMBERS);
	}

	if (settings.draw_small_health_bars) {
		renderer.call_triangles(D::SMALL_HEALTH_BARS);
	}

	renderer.call_triangles(D::SENTIENCE_INDICATORS);
	renderer.call_triangles(D::INDICATORS_AND_CALLOUTS);

	set_shader_with_matrix(shaders.exploding_rings);

	renderer.call_triangles(D::EXPLODING_RINGS);
	draw_pure_color_highlights();
	renderer.call_triangles(D::THUNDERS);

	shaders.standard->set_as_current(renderer);

	auto is_reasonably_in_view = [&](const const_entity_handle character) {
		if (!fog_of_war_effective) {
			return true;
		}

		return ::is_reasonably_in_view(
			viewed_character,
			character,
			in.pre_step_crosshair_displacement,
			interp,
			settings.fog_of_war.angle
		);
	};

	if (settings.draw_damage_indicators) {
		damage_indication.draw_indicators(
			is_reasonably_in_view,
			viewed_character,
			interp,
			in.damage_indication,
			game_images,
			in.fonts,
			get_drawer(), 
			cone
		);
	}

	renderer.call_and_clear_triangles();
	renderer.call_and_clear_lines();

	shaders.standard->set_projection(renderer, matrix);

	if (in.general_atlas) {
		in.general_atlas->set_as_current(renderer);
	}
}

float special_physics::get_teleport_alpha() const {
	if (inside_portal.is_set()) {
		return 0.0f;
	}

	if (teleport_progress == 0.0f) {
		return 1.0f;
	}

	auto result = 1.0f - std::clamp(teleport_progress, 0.0f, 1.0f);
	result *= result;

	const auto unit = float(teleport_decrease_opacity_to) / 255.0f;
	return augs::interp(unit, 1.0f, result);
}

