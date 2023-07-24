#include "augs/ensure_rel.h"
#include "augs/graphics/renderer.h"
#include "3rdparty/imgui/imgui.h"

#include "augs/templates/container_templates.h"

#include "augs/drawing/drawing.hpp"
#include "augs/graphics/imgui_payload.h"

#include "augs/texture_atlas/atlas_entry.h"
#include "augs/templates/corresponding_field.h"

namespace augs {
	void renderer::next_frame() {
		commands.clear();

		triangle_buffers.reset();
		line_buffers.reset();
		special_buffers.reset();

		for (auto& d : dedicated.single) {
			d.clear();
		}

		for (auto& d : dedicated.vectors) {
			for (auto& v : d) {
				v.clear();
			}
		}
	}

	void renderer::call_triangles(const dedicated_buffer_vector type, const uint32_t index) {
		drawcall_dedicated_vector_command cmd;
		cmd.type = type;
		cmd.index = index;
		push_command(std::move(cmd));
	}

	void renderer::call_triangles(const dedicated_buffer type) {
		drawcall_dedicated_command cmd;
		cmd.type = type;
		push_command(std::move(cmd));
	}

	void renderer::call_triangles_direct_ptr(const vertex_triangle_buffer& buffer) {
		if (buffer.empty()) {
			return;
		}

		num_total_triangles_drawn += buffer.size();

		drawcall_command cmd;
		cmd.triangles = buffer.data();
		cmd.count = buffer.size();

		push_command(std::move(cmd));
	}

	void renderer::call_triangles(vertex_triangle_buffer&& buffer) {
		if (buffer.empty()) {
			return;
		}

		num_total_triangles_drawn += buffer.size();

		drawcall_custom_buffer_command cmd;
		cmd.buffer = std::move(buffer);

		push_command(std::move(cmd));
	}

	void renderer::call_and_clear_triangles() {
		const auto& triangles = triangle_buffers.get();
		const auto& specials = special_buffers.get();

		if (triangles.empty()) {
			return;
		}

		num_total_triangles_drawn += triangles.size();

		drawcall_command cmd;
		cmd.triangles = triangles.data();

		if (specials.size() > 0) {
			cmd.specials = specials.data();
			ensure_eq(specials.size(), triangles.size() * 3);
		}

		cmd.count = triangles.size();

		push_command(std::move(cmd));

		triangle_buffers.request_next();
		special_buffers.request_next();
	}

	void renderer::call_and_clear_lines() {
		const auto& lines = line_buffers.get();

		if (lines.empty()) {
			return;
		}

		num_total_lines_drawn += lines.size();

		drawcall_command cmd;
		cmd.lines = lines.data();
		cmd.count = lines.size();

		push_command(std::move(cmd));

		line_buffers.request_next();
	}

	std::size_t renderer::get_triangle_count() const {
		return triangle_buffers.get().size();
	}

	vertex_triangle_buffer& renderer::get_triangle_buffer() {
		return triangle_buffers.get();
	}

	vertex_line_buffer& renderer::get_line_buffer() {
		return line_buffers.get();
	}

	special_buffer& renderer::get_special_buffer() {
		return special_buffers.get();
	}
	
	void renderer::save_debug_logic_step_lines_for_interpolation(const decltype(prev_logic_step_lines)& lines) {
		prev_logic_step_lines = lines;
	}

	void renderer::draw_call_imgui(
		graphics::texture& imgui_atlas,
		graphics::texture* game_world_atlas,
		graphics::texture* avatar_atlas,
		graphics::texture* avatar_preview_atlas,
		graphics::texture* ad_hoc_atlas
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

				const auto& io = ImGui::GetIO();
				const int fb_height = static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y);

				const auto output = cmd_list->CloneOutput();
				ensure(output != nullptr);
				push_command(setup_imgui_list { output, fb_height } );

				for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i) {
					const ImDrawCmd* const pcmd = &cmd_list->CmdBuffer[cmd_i];

					const auto atlas_type = static_cast<augs::imgui_atlas_type>(reinterpret_cast<intptr_t>(pcmd->TextureId));

					bool rebind = false;

					auto bind_if = [this, atlas_type, &rebind](const augs::imgui_atlas_type type, auto* atlas) {
						if (atlas_type == type) {
							if (atlas != nullptr) {
								atlas->set_as_current(*this);

								if (type == augs::imgui_atlas_type::GAME) {
									/* 
										Mostly to properly view image-based widgets in editor.
										This is properly reset in illuminated rendering procedure to a user-preferred filtering value.
									*/

									atlas->set_filtering(*this, augs::filtering_type::NEAREST_NEIGHBOR);
								}

								rebind = true;
							}
						}
					};

					bind_if(augs::imgui_atlas_type::GAME, game_world_atlas);
					bind_if(augs::imgui_atlas_type::AVATARS, avatar_atlas);
					bind_if(augs::imgui_atlas_type::AVATAR_PREVIEW, avatar_preview_atlas);
					bind_if(augs::imgui_atlas_type::AD_HOC, ad_hoc_atlas);

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

	void renderer::screenshot(const xywhi bounds) {
		push_command(make_screenshot { bounds });
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

	void renderer::fullscreen_quad() {
		push_no_arg(no_arg_command::FULLSCREEN_QUAD);
	}

	void renderer::finish() {
		push_no_arg(no_arg_command::FINISH);
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

	void renderer::finish_writing_stencil() {
		push_no_arg(no_arg_command::FINISH_WRITING_STENCIL);
	}

}
