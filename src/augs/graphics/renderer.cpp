#include "augs/graphics/renderer.h"
#include "3rdparty/imgui/imgui.h"

#include "augs/templates/container_templates.h"

#include "augs/drawing/drawing.h"
#include "augs/graphics/imgui_payload.h"

#include "augs/texture_atlas/atlas_entry.h"
#include "augs/templates/corresponding_field.h"

namespace augs {
	renderer::renderer(const renderer_settings& settings) : current_settings(settings) {
		apply(settings, true);
	}

	void renderer::call_triangles(vertex_triangle_buffer&& buffer) {
		if (buffer.empty()) {
			return;
		}

		num_total_triangles_drawn += buffer.size();

		drawcall_command cmd;
		cmd.triangles = std::move(buffer);

		push_command(std::move(cmd));
	}

	void renderer::clear_triangles() {
		triangles.clear();
		specials.clear();
	}

	void renderer::call_and_clear_triangles() {
		if (triangles.empty()) {
			return;
		}

		num_total_triangles_drawn += triangles.size();

		drawcall_command cmd;
		cmd.triangles = std::move(triangles);
		cmd.specials = std::move(specials);

		push_command(std::move(cmd));

		clear_triangles();
	}

	void renderer::call_and_clear_lines() {
		if (lines.empty()) {
			return;
		}

		num_total_lines_drawn += lines.size();

		drawcall_command cmd;
		cmd.lines = std::move(lines);

		push_command(std::move(cmd));

		clear_lines();
	}

	void renderer::clear_lines() {
		lines.clear();
	}

	std::size_t renderer::get_triangle_count() const {
		return triangles.size();
	}

	vertex_triangle_buffer& renderer::get_triangle_buffer() {
		return triangles;
	}

	vertex_line_buffer& renderer::get_line_buffer() {
		return lines;
	}

	special_buffer& renderer::get_special_buffer() {
		return specials;
	}
	
	void renderer::save_debug_logic_step_lines_for_interpolation(const decltype(prev_logic_step_lines)& lines) {
		prev_logic_step_lines = lines;
	}

	void renderer::draw_call_imgui(
		const graphics::texture& imgui_atlas,
		const graphics::texture* game_world_atlas,
		const graphics::texture* avatar_atlas,
		const graphics::texture* avatar_preview_atlas
	) {
		const auto* const draw_data = ImGui::GetDrawData();

		graphics::texture::mark_current(*this);

		if (draw_data != nullptr) {
			imgui_atlas.set_as_current(*this);

			set_scissor(true);

			for (int n = 0; n < draw_data->CmdListsCount; ++n) {
				const ImDrawList* cmd_list = draw_data->CmdLists[n];

				if (cmd_list->CmdBuffer.Size == 0) {
					continue;
				}

				push_command(setup_imgui_list { cmd_list->CloneOutput() } );

				for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i) {
					const ImDrawCmd* const pcmd = &cmd_list->CmdBuffer[cmd_i];

					const auto atlas_type = static_cast<augs::imgui_atlas_type>(reinterpret_cast<intptr_t>(pcmd->TextureId));

					bool rebind = false;

					if (atlas_type == augs::imgui_atlas_type::GAME) {
						if (game_world_atlas != nullptr) {
							game_world_atlas->set_as_current(*this);
							rebind = true;
						}
					}

					if (atlas_type == augs::imgui_atlas_type::AVATARS) {
						if (avatar_atlas != nullptr) {
							avatar_atlas->set_as_current(*this);
							rebind = true;
						}
					}
					
					if (atlas_type == augs::imgui_atlas_type::AVATAR_PREVIEW) {
						if (avatar_preview_atlas != nullptr) {
							avatar_preview_atlas->set_as_current(*this);
							rebind = true;
						}
					}
					
					push_no_arg(no_arg_command::IMGUI_CMD);

					if (rebind) {
						imgui_atlas.set_as_current(*this);
					}
				}
			}

			set_scissor(false);

			graphics::texture::set_current_to_marked(*this);
		}
	}

	void renderer::draw_debug_lines(
		const debug_lines& logic_step_lines,
		const debug_lines& persistent_lines,
		const debug_lines& frame_lines,

		const augs::atlas_entry tex,
		const float interpolation_ratio
	) {
		const auto output = augs::line_drawer_with_default({ get_line_buffer(), tex });

		auto line_lambda = [&](const debug_line line) {
			output.line(line.a, line.b, line.col);
		};

		if (interpolate_debug_logic_step_lines && logic_step_lines.size() == prev_logic_step_lines.size()) {
			std::vector<debug_line> interpolated_logic_step_lines;

			interpolated_logic_step_lines.resize(logic_step_lines.size());

			for (size_t i = 0; i < logic_step_lines.size(); ++i) {
				interpolated_logic_step_lines[i].a = prev_logic_step_lines[i].a.lerp(logic_step_lines[i].a, interpolation_ratio);
				interpolated_logic_step_lines[i].b = prev_logic_step_lines[i].b.lerp(logic_step_lines[i].b, interpolation_ratio);
				interpolated_logic_step_lines[i].col = logic_step_lines[i].col;
			}

			for_each_in(interpolated_logic_step_lines, line_lambda);
		}
		else {
			for_each_in(logic_step_lines, line_lambda);
		}

		for_each_in(persistent_lines, line_lambda);
		for_each_in(frame_lines, line_lambda);
	}

	void renderer::apply(const renderer_settings& settings, const bool force) {
		(void)settings;
		(void)force;

		current_settings = settings;
	}

	const renderer_settings& renderer::get_current_settings() const {
		return current_settings;
	}

	void renderer::fullscreen_quad() {
		push_no_arg(no_arg_command::FULLSCREEN_QUAD);
	}

	void renderer::set_blending(const bool on) {
		push_toggle(toggle_command_type::BLENDING, on);
	}

	void renderer::set_clear_color(const rgba col) {
		push_command(set_clear_color_command { col });
	}

	void renderer::clear_current_fbo() {
		push_no_arg(no_arg_command::CLEAR_CURRENT_FBO);
	}

	void renderer::set_standard_blending() {
		push_no_arg(no_arg_command::SET_STANDARD_BLENDING);
	}

	void renderer::set_overwriting_blending() {
		push_no_arg(no_arg_command::SET_OVERWRITING_BLENDING);
	}

	void renderer::set_additive_blending() {
		push_no_arg(no_arg_command::SET_ADDITIVE_BLENDING);
	}

	void renderer::set_active_texture(const unsigned n) {
		push_command(set_active_texture_command { n });
	}

	void renderer::set_viewport(const xywhi xywh) {
		push_command(set_viewport_command { xywh });
	}

	void renderer::set_scissor(const bool f) {
		push_toggle(toggle_command_type::SCISSOR, f);
	}

	void renderer::set_scissor_bounds(const xywhi xywh) {
		push_command(set_scissor_bounds_command { xywh });
	}

	void renderer::set_stencil(const bool f) {
		push_toggle(toggle_command_type::STENCIL, f);
	}

	void renderer::clear_stencil() {
		push_no_arg(no_arg_command::CLEAR_STENCIL);
	}

	void renderer::start_writing_stencil() {
		push_no_arg(no_arg_command::START_WRITING_STENCIL);
	}

	void renderer::stencil_reverse_test() {
		push_no_arg(no_arg_command::STENCIL_REVERSE_TEST);
	}

	void renderer::stencil_positive_test() {
		push_no_arg(no_arg_command::STENCIL_POSITIVE_TEST);
	}

	void renderer::start_testing_stencil() {
		push_no_arg(no_arg_command::START_TESTING_STENCIL);
	}

}
