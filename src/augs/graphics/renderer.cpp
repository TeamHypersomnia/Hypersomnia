#include "3rdparty/imgui/imgui.h"

#include "augs/templates/container_templates.h"
#include "augs/graphics/OpenGL_includes.h"
#include "augs/graphics/renderer.h"
#include "augs/graphics/fbo.h"

#include "augs/drawing/drawing.h"
#include "augs/graphics/imgui_payload.h"

#include "augs/texture_atlas/atlas_entry.h"
#include "augs/templates/corresponding_field.h"

#if PLATFORM_UNIX
#define USE_BUFFER_SUB_DATA 0
#endif

#if BUILD_OPENGL
void buffer_data(
	GLenum target,
  	GLsizeiptr size,
  	const GLvoid * data,
  	GLenum usage
) {
#if USE_BUFFER_SUB_DATA
	GL_CHECK(glBufferSubData(target, 0, size, data));
#else
	GL_CHECK(glBufferData(target, size, data, usage));
#endif
}
#endif

namespace augs {
	renderer::renderer(const renderer_settings& settings) : current_settings(settings) {
#if BUILD_OPENGL
		LOG("Calling gladLoadGL: %x.", intptr_t(gladLoadGL));
		if (!gladLoadGL()) {
			LOG("Calling gladLoadGL failed.");
			throw renderer_error("Failed to initialize GLAD!"); 		
		}
#endif

		LOG("glBlendFuncSeparate ADDR: %x", intptr_t(glBlendFuncSeparate));

		LOG("Calling gladLoadGL succeeded.");
		
		GL_CHECK(glEnable(GL_BLEND));
		
		set_standard_blending();
		set_clear_color(black);
		 
		GL_CHECK(glDisable(GL_DITHER));
		GL_CHECK(glDisable(GL_LINE_SMOOTH));
		GL_CHECK(glDisable(GL_POLYGON_SMOOTH));
		GL_CHECK(glDisable(GL_MULTISAMPLE));
		GL_CHECK(glDisable(GL_DEPTH_TEST));
		GL_CHECK(glDepthMask(GL_FALSE));

		GL_CHECK(glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST));
		GL_CHECK(glHint(GL_TEXTURE_COMPRESSION_HINT, GL_FASTEST));
		
		GL_CHECK(glGenVertexArrays(1, &vao_buffer));
		GL_CHECK(glBindVertexArray(vao_buffer));
		
		GL_CHECK(glGenBuffers(1, &imgui_elements_id));

		GL_CHECK(glGenBuffers(1, &triangle_buffer_id));
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id));

		GL_CHECK(glEnableVertexAttribArray(static_cast<int>(vertex_attribute::position)));
		GL_CHECK(glEnableVertexAttribArray(static_cast<int>(vertex_attribute::texcoord)));
		GL_CHECK(glEnableVertexAttribArray(static_cast<int>(vertex_attribute::color)));

		GL_CHECK(glVertexAttribPointer(static_cast<int>(vertex_attribute::position), 2, GL_FLOAT, GL_FALSE, sizeof(vertex), nullptr));
		GL_CHECK(glVertexAttribPointer(static_cast<int>(vertex_attribute::texcoord), 2, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<char*>(sizeof(float) * 2)));
		GL_CHECK(glVertexAttribPointer(static_cast<int>(vertex_attribute::color), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex), reinterpret_cast<char*>(sizeof(float) * 2 + sizeof(float) * 2)));

		GL_CHECK(glGenBuffers(1, &special_buffer_id));
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, special_buffer_id));

		enable_special_vertex_attribute();
		GL_CHECK(glVertexAttribPointer(static_cast<int>(vertex_attribute::special), sizeof(special) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(special), nullptr));
		disable_special_vertex_attribute();

		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id));
	
#if USE_BUFFER_SUB_DATA
		/* Preallocate necessary space */

		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * 30000, nullptr, GL_STREAM_DRAW));
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, special_buffer_id));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(special) * 6000, nullptr, GL_STREAM_DRAW));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, imgui_elements_id));
		GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * 12000, nullptr, GL_STREAM_DRAW));
#endif

#if BUILD_OPENGL
		GLint read_size = 0;
		GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &read_size));
		ensure(read_size >= 0);
		max_texture_size = static_cast<unsigned>(read_size);
#else
		max_texture_size = 0u;
#endif

		apply(settings, true);
	}

	void renderer::enable_special_vertex_attribute() {
		GL_CHECK(glEnableVertexAttribArray(static_cast<int>(vertex_attribute::special)));
	}

	void renderer::disable_special_vertex_attribute() {
		GL_CHECK(glDisableVertexAttribArray(static_cast<int>(vertex_attribute::special)));
	}

	void renderer::set_clear_color(const rgba col) {
		const auto v = vec4(col);
		(void)v;
		GL_CHECK(glClearColor(v[0], v[1], v[2], v[3]));
	}

	void renderer::clear_current_fbo() {
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
	}

	void renderer::set_standard_blending() {
		GL_CHECK(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE));
	}

	void renderer::set_additive_blending() {
		GL_CHECK(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE));
	}

	void renderer::set_active_texture(const unsigned n) {
		GL_CHECK(glActiveTexture(GL_TEXTURE0 + n));
		(void)n;
	}

	void renderer::call_triangles() {
		if (triangles.empty()) {
			return;
		}

		if (!specials.empty()) {
			enable_special_vertex_attribute();
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, special_buffer_id));
			GL_CHECK(buffer_data(GL_ARRAY_BUFFER, sizeof(special) * specials.size(), specials.data(), GL_STREAM_DRAW));
		}

		call_triangles(triangles);

		if (!specials.empty()) {
			disable_special_vertex_attribute();
		}
	}

	void renderer::call_triangles(const vertex_triangle_buffer& buffer) {
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id));

		GL_CHECK(buffer_data(GL_ARRAY_BUFFER, sizeof(vertex_triangle) * buffer.size(), buffer.data(), GL_STREAM_DRAW));
		GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(buffer.size()) * 3));
		num_total_triangles_drawn += buffer.size();
	}

	void renderer::call_lines() {
		if (lines.empty()) return;

		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id));
		GL_CHECK(buffer_data(GL_ARRAY_BUFFER, sizeof(vertex_line) * lines.size(), lines.data(), GL_STREAM_DRAW));
		GL_CHECK(glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines.size()) * 2));
	}


	void renderer::set_viewport(const xywhi xywh) {
		GL_CHECK(glViewport(xywh.x, xywh.y, xywh.w, xywh.h));
		(void)xywh;
	}

	void renderer::clear_special_vertex_data() {
		specials.clear();
	}

	void renderer::clear_triangles() {
		triangles.clear();
		specials.clear();
	}

	void renderer::call_and_clear_triangles() {
		call_triangles();
		clear_triangles();
	}

	void renderer::call_and_clear_lines() {
		call_lines();
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

	void renderer::fullscreen_quad() {
		float vertices[] = {
			1.f, 1.f,
			1.f, 0.f,
			0.f, 0.f,

			0.f, 0.f,
			0.f, 1.f,
			1.f, 1.f
		};
		(void)vertices;

		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id));

		GL_CHECK(glDisableVertexAttribArray(static_cast<int>(vertex_attribute::texcoord)));
		GL_CHECK(glDisableVertexAttribArray(static_cast<int>(vertex_attribute::color)));
		GL_CHECK(glVertexAttribPointer(static_cast<int>(vertex_attribute::position), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));
		GL_CHECK(buffer_data(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW));

		GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));

		GL_CHECK(glEnableVertexAttribArray(static_cast<int>(vertex_attribute::position)));
		GL_CHECK(glEnableVertexAttribArray(static_cast<int>(vertex_attribute::texcoord)));
		GL_CHECK(glEnableVertexAttribArray(static_cast<int>(vertex_attribute::color)));

		GL_CHECK(glVertexAttribPointer(static_cast<int>(vertex_attribute::position), 2, GL_FLOAT, GL_FALSE, sizeof(vertex), nullptr));
		GL_CHECK(glVertexAttribPointer(static_cast<int>(vertex_attribute::texcoord), 2, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<char*>(sizeof(float) * 2)));
		GL_CHECK(glVertexAttribPointer(static_cast<int>(vertex_attribute::color), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex), reinterpret_cast<char*>(sizeof(float) * 2 + sizeof(float) * 2)));
	}

	void renderer::draw_call_imgui(
		const graphics::texture& imgui_atlas,
		const graphics::texture* game_world_atlas
	) {
		const auto* const draw_data = ImGui::GetDrawData();

		if (draw_data != nullptr) {
			imgui_atlas.bind();

			ImGuiIO& io = ImGui::GetIO();
			// const int fb_width = static_cast<int>(io.DisplaySize.x * io.DisplayFramebufferScale.x);
			const int fb_height = static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y);
			(void)fb_height;

			GL_CHECK(glEnable(GL_SCISSOR_TEST));

			for (int n = 0; n < draw_data->CmdListsCount; ++n) {
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				const ImDrawIdx* idx_buffer_offset = 0;

				GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id));
				GL_CHECK(buffer_data(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW));

				GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, imgui_elements_id));
				GL_CHECK(buffer_data(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW));

				for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i) {
					const ImDrawCmd* const pcmd = &cmd_list->CmdBuffer[cmd_i];

					const auto atlas_type = static_cast<augs::imgui_atlas_type>(reinterpret_cast<intptr_t>(pcmd->TextureId));

					bool rebind = false;

					if (atlas_type == augs::imgui_atlas_type::GAME) {
						if (game_world_atlas != nullptr) {
							game_world_atlas->bind();
							rebind = true;
						}
					}
					
					GL_CHECK(glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y)));
					GL_CHECK(glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset));

					idx_buffer_offset += pcmd->ElemCount;

					if (rebind) {
						imgui_atlas.bind();
					}
				}
			}

			GL_CHECK(glDisable(GL_SCISSOR_TEST));

			if (game_world_atlas != nullptr) {
				game_world_atlas->bind();
			}
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

	unsigned renderer::get_max_texture_size() const {
		return max_texture_size;
	}

	void renderer::finish() {
		GL_CHECK(glFinish());
	}

	GLsync renderer::fence() const {
#if BUILD_OPENGL
		const auto s = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		return s;
#else
		return 0;
#endif
	}

	bool renderer::wait_sync(const GLsync sync, const GLuint64 timeout) const {
#if BUILD_OPENGL
		const auto result = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, timeout);
		return !(result == GL_TIMEOUT_EXPIRED || result == GL_WAIT_FAILED);
#else
		(void)sync;
		(void)timeout;

		(void)triangle_buffer_id;
		(void)special_buffer_id;
		(void)imgui_elements_id;
		return false;
#endif
	}

	void renderer::enable_stencil() {
		GL_CHECK(glEnable(GL_STENCIL_TEST));
	}

	void renderer::disable_stencil() {
		GL_CHECK(glDisable(GL_STENCIL_TEST));
	}

	void renderer::clear_stencil() {
		GL_CHECK(glClear(GL_STENCIL_BUFFER_BIT));
	}

	void renderer::start_writing_stencil() {
		GL_CHECK(glStencilFunc(GL_ALWAYS, 1, 0xFF));
		GL_CHECK(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
		GL_CHECK(glStencilMask(0xFF));
		GL_CHECK(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
	}

	void renderer::stencil_reverse_test() {
		GL_CHECK(glStencilFunc(GL_EQUAL, 0, 0xFF));
	}

	void renderer::stencil_positive_test() {
		GL_CHECK(glStencilFunc(GL_EQUAL, 1, 0xFF));
	}

	void renderer::start_testing_stencil() {
		stencil_reverse_test();
		GL_CHECK(glStencilMask(0x00));
		GL_CHECK(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
	}

	void renderer::apply(const renderer_settings& settings, const bool force) {
		(void)settings;
		(void)force;

		current_settings = settings;
	}

	const renderer_settings& renderer::get_current_settings() const {
		return current_settings;
	}
}
