#include "all.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/systems_stateless/render_system.h"
#include "game/systems_stateless/gui_system.h"

#include "game/detail/gui/character_gui.h"
#include "game/assets/assets_manager.h"
#include "augs/graphics/renderer.h"
#include "game/view/viewing_step.h"
#include "game/view/viewing_session.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/render_component.h"

#include "augs/graphics/drawers.h"

#include "augs/math/matrix.h"

#include "augs/graphics/OpenGL_includes.h"

#include <imgui/imgui.h>

namespace rendering_scripts {
	void illuminated_rendering(const viewing_step step) {
		auto& renderer = step.renderer;
		auto& output = renderer.triangles;
		const auto& cosmos = step.cosm;
		const auto camera = step.camera;
		const auto controlled_entity = cosmos[step.viewed_character];
		const auto controlled_crosshair = controlled_entity.alive() ? controlled_entity[child_entity_name::CHARACTER_CROSSHAIR] : cosmos[entity_id()];
		const auto& interp = step.session.systems_audiovisual.get<interpolation_system>();
		const auto& particles = step.session.systems_audiovisual.get<particles_simulation_system>();
		const auto& wandering_pixels = step.session.systems_audiovisual.get<wandering_pixels_system>();
		const auto& exploding_rings = step.session.systems_audiovisual.get<exploding_ring_system>();
		const auto& flying_numbers = step.session.systems_audiovisual.get<vertically_flying_number_system>();
		const auto& highlights = step.session.systems_audiovisual.get<pure_color_highlight_system>();
		const auto& thunders = step.session.systems_audiovisual.get<thunder_system>();
		const auto global_time_seconds = (step.get_interpolated_total_time_passed_in_seconds());
		const auto settings = step.session.config.drawing_settings;

		const auto screen_size = vec2i(camera.visible_world_area);

		const auto matrix = augs::orthographic_projection<float>(
			0.f, 
			camera.visible_world_area.x, 
			camera.visible_world_area.y, 
			0.f, 
			0.f, 
			1.f
		);

		const auto& visible_per_layer = step.visible.per_layer;

		const auto& manager = get_assets_manager();

		const auto& default_shader = manager[assets::program_id::DEFAULT];
		const auto& illuminated_shader = manager[assets::program_id::DEFAULT_ILLUMINATED];
		const auto& specular_highlights_shader = manager[assets::program_id::SPECULAR_HIGHLIGHTS];
		const auto& pure_color_highlight_shader = manager[assets::program_id::PURE_COLOR_HIGHLIGHT];
		const auto& border_highlight_shader = pure_color_highlight_shader; // the same
		const auto& circular_bars_shader = manager[assets::program_id::CIRCULAR_BARS];
		const auto& smoke_shader = manager[assets::program_id::SMOKE];
		const auto& illuminating_smoke_shader = manager[assets::program_id::ILLUMINATING_SMOKE];
		const auto& exploding_rings_shader = manager[assets::program_id::EXPLODING_RING];
		
		default_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(default_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}

		renderer.smoke_fbo.use();
		glClear(GL_COLOR_BUFFER_BIT);

		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE); glerr;
		
		{
			particles.draw(
				output,
				render_layer::DIM_SMOKES,
				camera
			);

			particles.draw(
				output,
				render_layer::ILLUMINATING_SMOKES,
				camera
			);
		}

		renderer.call_triangles();
		renderer.clear_triangles();

		renderer.illuminating_smoke_fbo.use();
		glClear(GL_COLOR_BUFFER_BIT);

		{
			particles.draw(
				output,
				render_layer::ILLUMINATING_SMOKES,
				camera
			);
		}

		renderer.call_triangles();
		renderer.clear_triangles();

		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE); glerr;

		renderer.illuminating_smoke_fbo.use_default();

		const auto& light = step.session.systems_audiovisual.get<light_system>();
		
		light.render_all_lights(renderer, matrix, step, [&]() {
				if (controlled_entity.alive()) {
					draw_crosshair_lasers(
						[&](const vec2 from, const vec2 to, const rgba col) {
							if (!settings.draw_weapon_laser) {
								return;
							}

							const auto& edge_tex = manager[assets::game_image_id::LASER_GLOW_EDGE].texture_maps[texture_map_type::DIFFUSE];
							const vec2 edge_size = static_cast<vec2>(edge_tex.get_size());

							augs::draw_line(output, camera[from], camera[to], edge_size.y/3.f, manager[assets::game_image_id::LASER].texture_maps[texture_map_type::NEON], col);

							const auto edge_offset = (to - from).set_length(edge_size.x);

							augs::draw_line(output, camera[to], camera[to + edge_offset], edge_size.y / 3.f, edge_tex, col);
							augs::draw_line(output, camera[from - edge_offset], camera[from], edge_size.y / 3.f, edge_tex, col, true);
						},
						[](...){},
						interp, 
						controlled_crosshair, 
						controlled_entity
					);
				}

				draw_cast_spells_highlights(
					output,
					interp,
					camera,
					cosmos,
					global_time_seconds
				);

				renderer.set_active_texture(3);
				renderer.bind_texture(renderer.illuminating_smoke_fbo);
				renderer.set_active_texture(0);
				
				illuminating_smoke_shader.use();
				
				renderer.fullscreen_quad();
				
				default_shader.use();

				exploding_rings.draw_highlights_of_rings(
					output,
					camera,
					cosmos
				);
			}
		);

		illuminated_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(illuminated_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}

		render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::UNDER_GROUND], camera, renderable_drawing_type::NORMAL);
		render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::GROUND], camera, renderable_drawing_type::NORMAL);
		render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::ON_GROUND], camera, renderable_drawing_type::NORMAL);
		render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::TILED_FLOOR], camera, renderable_drawing_type::NORMAL);

		renderer.call_triangles();
		renderer.clear_triangles();

		specular_highlights_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(specular_highlights_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}

		render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::TILED_FLOOR], camera, renderable_drawing_type::SPECULAR_HIGHLIGHTS);

		renderer.call_triangles();
		renderer.clear_triangles();

		illuminated_shader.use();

		render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::ON_TILED_FLOOR], camera, renderable_drawing_type::NORMAL);
		render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::CAR_INTERIOR], camera, renderable_drawing_type::NORMAL);
		render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::CAR_WHEEL], camera, renderable_drawing_type::NORMAL);

		renderer.call_triangles();
		renderer.clear_triangles();

		border_highlight_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(border_highlight_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}
		
		render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::SMALL_DYNAMIC_BODY], camera, renderable_drawing_type::BORDER_HIGHLIGHTS);
		
		renderer.call_triangles();
		renderer.clear_triangles();
		
		illuminated_shader.use();
		
		render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::DYNAMIC_BODY], camera, renderable_drawing_type::NORMAL);
		render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::SMALL_DYNAMIC_BODY], camera, renderable_drawing_type::NORMAL);
		
		renderer.call_triangles();
		renderer.clear_triangles();

		renderer.set_active_texture(1);
		renderer.bind_texture(renderer.smoke_fbo);
		renderer.set_active_texture(0);

		smoke_shader.use();

		renderer.fullscreen_quad();

		default_shader.use();
		
		render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::FLYING_BULLETS], camera, renderable_drawing_type::NORMAL);
		render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::NEON_CAPTIONS], camera, renderable_drawing_type::NORMAL);
		
		if (settings.draw_crosshairs) {
			render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::CROSSHAIR], camera, renderable_drawing_type::NORMAL);
		}
		
		render_system().draw_entities(interp, global_time_seconds, output, cosmos, visible_per_layer[render_layer::OVER_CROSSHAIR], camera, renderable_drawing_type::NORMAL);

		if (settings.draw_crosshairs && settings.draw_weapon_laser && controlled_entity.alive()) {
			draw_crosshair_lasers(
				[&](const vec2 from, const vec2 to, const rgba col) {
					augs::draw_line(
						renderer.lines, 
						camera[from], 
						camera[to], 
						manager[assets::game_image_id::LASER].texture_maps[texture_map_type::DIFFUSE], 
						col
					);
				},

				[&](const vec2 from, const vec2 to) {
					augs::draw_dashed_line(
						renderer.lines,
						camera[from],
						camera[to],
						manager[assets::game_image_id::LASER].texture_maps[texture_map_type::DIFFUSE],
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

		{
			particles.draw(
				output,
				render_layer::ILLUMINATING_PARTICLES,
				camera
			);
		}

		{
			wandering_pixels_system::drawing_input wandering_input(output);
			wandering_input.camera = camera;

			for (const auto e : visible_per_layer[render_layer::WANDERING_PIXELS_EFFECTS]) {
				wandering_pixels.draw_wandering_pixels_for(cosmos[e], wandering_input);
			}
		}

		renderer.call_triangles();
		renderer.clear_triangles();

		circular_bars_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(circular_bars_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());

		}

		const auto set_center_uniform = [&](const auto image_id) {
			const auto upper = manager[image_id].texture_maps[texture_map_type::DIFFUSE].get_atlas_space_uv({ 0.0f, 0.0f });
			const auto lower = manager[image_id].texture_maps[texture_map_type::DIFFUSE].get_atlas_space_uv({ 1.f, 1.f });
			const auto center = (upper + lower) / 2;

			glUniform2f(glGetUniformLocation(circular_bars_shader.id, "texture_center"), center.x, center.y);
		};

		set_center_uniform(assets::game_image_id::CIRCULAR_BAR_MEDIUM);

		const auto textual_infos = draw_circular_bars_and_get_textual_info(step);

		renderer.call_triangles();
		renderer.clear_triangles();
		
		set_center_uniform(assets::game_image_id::CIRCULAR_BAR_SMALL);

		draw_hud_for_released_explosives(
			output,
			renderer.specials,
			interp,
			camera,
			cosmos,
			global_time_seconds
		);

		renderer.call_triangles();
		renderer.clear_triangles();

		default_shader.use();

		renderer.call_triangles(textual_infos);

		exploding_rings_shader.use();
		{
			const auto projection_matrix_uniform = glGetUniformLocation(exploding_rings_shader.id, "projection_matrix");
			glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, matrix.data());
		}

		exploding_rings.draw_rings(
			output,
			renderer.specials,
			camera,
			cosmos
		);

		renderer.call_triangles();
		renderer.clear_triangles();

		pure_color_highlight_shader.use();

		highlights.draw_highlights(
			output,
			camera,
			cosmos,
			interp
		);

		thunders.draw_thunders(
			renderer.lines,
			camera
		);

		renderer.call_triangles();
		renderer.call_lines();
		renderer.clear_triangles();
		renderer.clear_lines();

		default_shader.use();

		flying_numbers.draw_numbers(
			output, 
			camera
		);

		if (settings.draw_character_gui && controlled_entity.alive()) {
			if (controlled_entity.has<components::item_slot_transfers>()) {
				step.session.systems_audiovisual.get<gui_element_system>().get_character_gui(controlled_entity).draw(
					step,
					step.session.config.hotbar
				);
			}
		}

		renderer.bind_texture(manager[assets::gl_texture_id::GAME_WORLD_ATLAS]);

		renderer.call_triangles();
		renderer.clear_triangles();

		renderer.draw_debug_info(
			camera,
			assets::game_image_id::BLANK,
			{},
			step.get_interpolation_ratio()
		);

		{
			const auto* const draw_data = ImGui::GetDrawData();

			if (draw_data != nullptr) {
				ImGuiIO& io = ImGui::GetIO();
				const int fb_width = int(io.DisplaySize.x * io.DisplayFramebufferScale.x);
				const int fb_height = int(io.DisplaySize.y * io.DisplayFramebufferScale.y);

				glEnable(GL_SCISSOR_TEST);

				for (int n = 0; n < draw_data->CmdListsCount; ++n) {
					const ImDrawList* cmd_list = draw_data->CmdLists[n];
					const ImDrawIdx* idx_buffer_offset = 0;

					glBindBuffer(GL_ARRAY_BUFFER, renderer.triangle_buffer_id);
					glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer.imgui_elements_id);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

					for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
						const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
						if (pcmd->UserCallback) {
							pcmd->UserCallback(cmd_list, pcmd);
						}
						else {
							renderer.bind_texture(manager[assets::gl_texture_id { reinterpret_cast<intptr_t>(pcmd->TextureId) }]);
							glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
							glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
						}
						idx_buffer_offset += pcmd->ElemCount;
					}
				}

				glDisable(GL_SCISSOR_TEST);
			}
		}

		renderer.bind_texture(manager[assets::gl_texture_id::GAME_WORLD_ATLAS]);
	}
}