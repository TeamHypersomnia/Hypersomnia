#include "all.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/systems_stateless/render_system.h"
#include "game/systems_stateless/gui_system.h"

#include "game/components/gui_element_component.h"
#include "game/systems_temporary/dynamic_tree_system.h"
#include "game/resources/manager.h"
#include "augs/graphics/renderer.h"
#include "game/transcendental/step.h"
#include "game/transcendental/viewing_session.h"

#include "game/detail/gui/immediate_hud.h"
#include "augs/graphics/drawers.h"

#include "math/matrix.h"

#include "3rdparty/GL/OpenGL.h"

namespace rendering_scripts {
	void illuminated_rendering(viewing_step& step) {
		const auto& camera = step.camera;
		auto& renderer = step.renderer;
		auto& output = renderer.triangles;
		const auto& cosmos = step.cosm;
		const auto& dynamic_tree = cosmos.systems_temporary.get<dynamic_tree_system>();
		const auto& physics = cosmos.systems_temporary.get<physics_system>();
		const auto& controlled_entity = cosmos[step.viewed_character];
		const auto controlled_crosshair = controlled_entity[sub_entity_name::CHARACTER_CROSSHAIR];
		const auto& interp = step.session.systems_audiovisual.get<interpolation_system>();
		const auto& particles = step.session.systems_audiovisual.get<particles_simulation_system>();
		auto& wandering_pixels = step.session.systems_audiovisual.get<wandering_pixels_system>();
		const float global_time_seconds = static_cast<float>(step.get_interpolated_total_time_passed_in_seconds());

		auto all_visible = dynamic_tree.determine_visible_entities_from_camera(camera);
		const auto visible_from_physics = physics.query_camera(camera).entities;
		
		all_visible.insert(all_visible.end(), visible_from_physics.begin(), visible_from_physics.end());

		step.visible_entities = cosmos[all_visible];
		step.visible_per_layer = render_system().get_visible_per_layer(step.visible_entities);

		const auto matrix = augs::orthographic_projection<float>(0, camera.visible_world_area.x, camera.visible_world_area.y, 0, 0, 1);

		auto& default_shader = *get_resource_manager().find(assets::program_id::DEFAULT);
		auto& illuminated_shader = *get_resource_manager().find(assets::program_id::DEFAULT_ILLUMINATED);
		auto& specular_highlights_shader = *get_resource_manager().find(assets::program_id::SPECULAR_HIGHLIGHTS);
		auto& pure_color_highlight_shader = *get_resource_manager().find(assets::program_id::PURE_COLOR_HIGHLIGHT);
		auto& border_highlight_shader = pure_color_highlight_shader; // the same
		auto& circular_bars_shader = *get_resource_manager().find(assets::program_id::CIRCULAR_BARS);
		auto& smoke_shader = *get_resource_manager().find(assets::program_id::SMOKE);

		particles_simulation_system::drawing_input particles_input(output);
		particles_input.camera = camera;
		
		wandering_pixels_system::drawing_input wandering_input(output);
		wandering_input.camera = camera;

		default_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(default_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}

		renderer.smoke_fbo.use();
		glClear(GL_COLOR_BUFFER_BIT);

		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE); glerr;

		particles.draw(render_layer::DIM_SMOKES, particles_input);

		renderer.call_triangles();
		renderer.clear_triangles();

		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE); glerr;

		renderer.smoke_fbo.use_default();

		auto& light = step.session.systems_audiovisual.get<light_system>();
		
		light.render_all_lights(renderer, matrix, step, [&]() {
				draw_crosshair_lines(
					[&](const vec2 from, const vec2 to, const rgba col) {
						if (!step.settings.draw_weapon_laser) {
							return;
						}

						const auto& edge_tex = get_resource_manager().find(assets::texture_id::LASER_GLOW_EDGE)->tex;
						const vec2 edge_size = static_cast<vec2>(edge_tex.get_size());

						augs::draw_line(output, camera[from], camera[to], edge_size.y/3.f, get_resource_manager().find_neon_map(assets::texture_id::LASER)->tex, col);

						const auto edge_offset = (to - from).set_length(edge_size.x);

						augs::draw_line(output, camera[to], camera[to + edge_offset], edge_size.y / 3.f, get_resource_manager().find(assets::texture_id::LASER_GLOW_EDGE)->tex, col);
						augs::draw_line(output, camera[from - edge_offset], camera[from], edge_size.y / 3.f, get_resource_manager().find(assets::texture_id::LASER_GLOW_EDGE)->tex, col, true);
					},
					[](...){},
					interp, 
					controlled_crosshair, 
					controlled_entity
				);
			}
		);

		illuminated_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(illuminated_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}

		render_system().draw_entities(interp, global_time_seconds, output, step.visible_per_layer[render_layer::UNDER_GROUND], camera, renderable_drawing_type::NORMAL);
		render_system().draw_entities(interp, global_time_seconds, output, step.visible_per_layer[render_layer::GROUND], camera, renderable_drawing_type::NORMAL);
		render_system().draw_entities(interp, global_time_seconds, output, step.visible_per_layer[render_layer::ON_GROUND], camera, renderable_drawing_type::NORMAL);
		render_system().draw_entities(interp, global_time_seconds, output, step.visible_per_layer[render_layer::TILED_FLOOR], camera, renderable_drawing_type::NORMAL);

		renderer.call_triangles();
		renderer.clear_triangles();

		specular_highlights_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(specular_highlights_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}

		render_system().draw_entities(interp, global_time_seconds, output, step.visible_per_layer[render_layer::TILED_FLOOR], camera, renderable_drawing_type::SPECULAR_HIGHLIGHTS);

		renderer.call_triangles();
		renderer.clear_triangles();

		illuminated_shader.use();

		render_system().draw_entities(interp, global_time_seconds, output, step.visible_per_layer[render_layer::ON_TILED_FLOOR], camera, renderable_drawing_type::NORMAL);
		render_system().draw_entities(interp, global_time_seconds, output, step.visible_per_layer[render_layer::CAR_INTERIOR], camera, renderable_drawing_type::NORMAL);
		render_system().draw_entities(interp, global_time_seconds, output, step.visible_per_layer[render_layer::CAR_WHEEL], camera, renderable_drawing_type::NORMAL);
		render_system().draw_entities(interp, global_time_seconds, output, step.visible_per_layer[render_layer::CORPSES], camera, renderable_drawing_type::NORMAL);

		renderer.call_triangles();
		renderer.clear_triangles();

		border_highlight_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(border_highlight_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}
		
		render_system().draw_entities(interp, global_time_seconds,output, step.visible_per_layer[render_layer::SMALL_DYNAMIC_BODY], camera, renderable_drawing_type::BORDER_HIGHLIGHTS);
		
		renderer.call_triangles();
		renderer.clear_triangles();
		
		illuminated_shader.use();
		
		render_system().draw_entities(interp, global_time_seconds,output, step.visible_per_layer[render_layer::DYNAMIC_BODY], camera, renderable_drawing_type::NORMAL);
		render_system().draw_entities(interp, global_time_seconds,output, step.visible_per_layer[render_layer::SMALL_DYNAMIC_BODY], camera, renderable_drawing_type::NORMAL);
		
		renderer.call_triangles();
		renderer.clear_triangles();

		renderer.set_active_texture(1);
		renderer.bind_texture(renderer.smoke_fbo);
		renderer.set_active_texture(0);

		smoke_shader.use();

		renderer.fullscreen_quad();

		default_shader.use();
		
		for (int i = render_layer::FLYING_BULLETS; i >= 0; --i) {
			if (i == render_layer::CROSSHAIR && !step.settings.draw_crosshairs) {
				continue;
			}

			render_system().draw_entities(interp, global_time_seconds,output, step.visible_per_layer[i], camera, renderable_drawing_type::NORMAL);
		}
		
		if (step.settings.draw_crosshairs && step.settings.draw_weapon_laser) {
			draw_crosshair_lines(
				[&](const vec2 from, const vec2 to, const rgba col) {
					augs::draw_line(
						renderer.lines, 
						camera[from], 
						camera[to], 
						get_resource_manager().find(assets::texture_id::LASER)->tex, 
						col
					);
				},

				[&](const vec2 from, const vec2 to) {
					augs::draw_dashed_line(
						renderer.lines,
						camera[from],
						camera[to],
						get_resource_manager().find(assets::texture_id::LASER)->tex,
						white,
						10.f,
						40.f, 
						global_time_seconds
					);
				},

				interp, 
				controlled_crosshair, 
				controlled_entity
			);

			renderer.call_lines();
			renderer.clear_lines();
		}

		particles.draw(render_layer::EFFECTS, particles_input);

		for (const auto e : step.visible_per_layer[render_layer::WANDERING_PIXELS_EFFECTS]) {
			wandering_pixels.advance_wandering_pixels_for(e, global_time_seconds, step.get_delta());
			wandering_pixels.draw_wandering_pixels_for(e, wandering_input);
		}

		renderer.call_triangles();
		renderer.clear_triangles();

		circular_bars_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(circular_bars_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
			
			vec2 upper(0.0f, 0.0f);
			vec2 lower(1.0f, 1.0f);
			(*assets::texture_id::HUD_CIRCULAR_BAR_MEDIUM).get_uv(upper);
			(*assets::texture_id::HUD_CIRCULAR_BAR_MEDIUM).get_uv(lower);
			const auto center = (upper + lower) / 2;
		
			glUniform2f(glGetUniformLocation(circular_bars_shader.id, "texture_center"), center.x, center.y);
		
		}

		const auto& hud = step.session.hud;

		const auto& textual_infos = hud.draw_circular_bars_and_get_textual_info(step);

		renderer.call_triangles();
		renderer.clear_triangles();

		default_shader.use();

		renderer.call_triangles(textual_infos);

		pure_color_highlight_shader.use();

		hud.draw_pure_color_highlights(step);

		renderer.call_triangles();
		renderer.clear_triangles();

		default_shader.use();

		hud.draw_vertically_flying_numbers(step);

		if (step.settings.draw_gui_overlays && controlled_entity.has<components::gui_element>()) {
			components::gui_element::draw_complete_gui_for_camera_rendering_request(output, controlled_entity, step);
		}

		renderer.bind_texture(*get_resource_manager().find(assets::atlas_id::GAME_WORLD_ATLAS));

		renderer.call_triangles();
		renderer.clear_triangles();

		renderer.draw_debug_info(
			camera.visible_world_area,
			camera.transform,
			assets::texture_id::BLANK,
			{},
			step.get_delta().view_interpolation_ratio());
	}
}