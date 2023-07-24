#include "augs/log_direct.h"
#include "augs/graphics/renderer_backend.h"
#include "augs/graphics/OpenGL_includes.h"
#include "augs/graphics/fbo.h"
#include "augs/graphics/vertex.h"
#include "augs/graphics/backend_access.h"
#include "3rdparty/imgui/imgui.h"
#include "augs/graphics/renderer_command.h"
#include "augs/templates/remove_cref.h"

#include "augs/graphics/shader.h"
#include "augs/graphics/fbo.h"
#include "augs/graphics/texture.h"
#include "augs/log.h"

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
	(void)usage;
	GL_CHECK(glBufferSubData(target, 0, size, data));
#else
	GL_CHECK(glBufferData(target, size, data, usage));
#endif
}
#endif

/* A shortcut which will be heavily used from now on */

#if BUILD_OPENGL
template <class A, class B>
constexpr bool same = std::is_same_v<A, B>;
#endif

namespace augs {
	namespace graphics {
		struct renderer_backend::platform_data {
			GLuint triangle_buffer_id = 0xdeadbeef;
			GLuint special_buffer_id = 0xdeadbeef;
			GLuint imgui_elements_id = 0xdeadbeef;
			GLuint vao_buffer = 0xdeadbeef;
		};

		renderer_backend::~renderer_backend() = default;

		renderer_backend::renderer_backend() : platform(std::make_unique<renderer_backend::platform_data>()) {
			const char* fname = "gladLoadGL";
#if USE_GLFW 
			fname = "gladLoadGLLoader";
#endif

#if BUILD_OPENGL
			LOG("Calling %x.", fname);

#if USE_GLFW 
			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
#else
			if (!gladLoadGL()) {
#endif
				LOG("Calling %x failed.", fname);
				throw renderer_error("Failed to initialize GLAD!"); 		
			}
#endif
			GL_CHECK(LOG("GL Version: %x", glGetString(GL_VERSION)));

			LOG("Calling %x succeeded.", fname);

			set_blending(true);

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

			GL_CHECK(glGenVertexArrays(1, &platform->vao_buffer));
			GL_CHECK(glBindVertexArray(platform->vao_buffer));

			GL_CHECK(glGenBuffers(1, &platform->imgui_elements_id));

			GL_CHECK(glGenBuffers(1, &platform->triangle_buffer_id));
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, platform->triangle_buffer_id));

			GL_CHECK(glEnableVertexAttribArray(static_cast<int>(vertex_attribute::position)));
			GL_CHECK(glEnableVertexAttribArray(static_cast<int>(vertex_attribute::texcoord)));
			GL_CHECK(glEnableVertexAttribArray(static_cast<int>(vertex_attribute::color)));

			GL_CHECK(glVertexAttribPointer(static_cast<int>(vertex_attribute::position), 2, GL_FLOAT, GL_FALSE, sizeof(vertex), nullptr));
			GL_CHECK(glVertexAttribPointer(static_cast<int>(vertex_attribute::texcoord), 2, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<char*>(sizeof(float) * 2)));
			GL_CHECK(glVertexAttribPointer(static_cast<int>(vertex_attribute::color), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex), reinterpret_cast<char*>(sizeof(float) * 2 + sizeof(float) * 2)));

			GL_CHECK(glGenBuffers(1, &platform->special_buffer_id));
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, platform->special_buffer_id));

			enable_special_vertex_attribute();
			GL_CHECK(glVertexAttribPointer(static_cast<int>(vertex_attribute::special), sizeof(special) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(special), nullptr));
			disable_special_vertex_attribute();

			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, platform->triangle_buffer_id));

#if USE_BUFFER_SUB_DATA
			/* Preallocate necessary space */

			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, platform->triangle_buffer_id));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * 30000, nullptr, GL_STREAM_DRAW));
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, platform->special_buffer_id));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(special) * 6000, nullptr, GL_STREAM_DRAW));
			GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, platform->imgui_elements_id));
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
		}

		unsigned renderer_backend::get_max_texture_size() const {
			return max_texture_size;
		}

		void renderer_backend::perform(const drawcall_command& cmd) {
#if BUILD_OPENGL
			const auto& p = *platform;

			const auto triangles = cmd.triangles;
			const auto lines = cmd.lines;
			const auto specials = cmd.specials;

			const auto cnt = static_cast<GLsizei>(cmd.count);
			const auto specials_cnt = cnt * 3;

			if (specials) {
				enable_special_vertex_attribute();
				GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, p.special_buffer_id));
				GL_CHECK(buffer_data(GL_ARRAY_BUFFER, sizeof(special) * specials_cnt, specials, GL_STREAM_DRAW));
			}

			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, p.triangle_buffer_id));

			if (triangles) {
				GL_CHECK(buffer_data(GL_ARRAY_BUFFER, sizeof(vertex_triangle) * cnt, triangles, GL_STREAM_DRAW));
				GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, cnt * 3));
			}

			if (lines) {
				GL_CHECK(buffer_data(GL_ARRAY_BUFFER, sizeof(vertex_line) * cnt, lines, GL_STREAM_DRAW));
				GL_CHECK(glDrawArrays(GL_LINES, 0, cnt * 2));
			}

			if (specials) {
				disable_special_vertex_attribute();
			}
#else
			(void)cmd;
#endif
		}

		void renderer_backend::perform(
			renderer_backend_result& output,
			const renderer_command* const c, 
			const std::size_t n,
			const dedicated_buffers& dedicated
		) {
#if BUILD_OPENGL
			auto& lists_to_delete = output.imgui_lists_to_delete;

			using A = backend_access;
			A access;

			const auto& p = *platform;

			ImDrawList* cmd_list = nullptr;
			std::size_t cmd_i = 0;
			int fb_height = -1;

			for (std::size_t i = 0; i < n; ++i) {
				const auto& cmd = c[i];

				auto command_handler = [&](const auto& typed_cmd) {
					using C = remove_cref<decltype(typed_cmd)>;

					auto perform_drawcall_for = [&](const auto& buffers) {
						if (const auto lines_n = buffers.lines.size(); lines_n > 0) {
							drawcall_command translated_cmd;

							translated_cmd.lines = buffers.lines.data();
							translated_cmd.count = lines_n;

							perform(translated_cmd);
						}

						if (const auto triangles_n = buffers.triangles.size(); triangles_n > 0) {
							drawcall_command translated_cmd;

							translated_cmd.triangles = buffers.triangles.data();
							translated_cmd.count = triangles_n;

							if (buffers.specials.size() > 0) {
								translated_cmd.specials = buffers.specials.data();
							}

							perform(translated_cmd);
						}
					};

					if constexpr(std::is_invocable_v<C, A>) {
						typed_cmd(access);
					}
					else if constexpr(same<C, drawcall_command>) {
						perform(typed_cmd);
					}
					else if constexpr(same<C, drawcall_custom_buffer_command>) {
						drawcall_command translated_cmd;

						translated_cmd.triangles = typed_cmd.buffer.data();
						translated_cmd.count = typed_cmd.buffer.size();

						perform(translated_cmd);
					}
					else if constexpr(same<C, drawcall_dedicated_command>) {
						const auto& buffers = dedicated[typed_cmd.type];
						perform_drawcall_for(buffers);
					}
					else if constexpr(same<C, drawcall_dedicated_vector_command>) {
						const auto& buffers = dedicated[typed_cmd.type][typed_cmd.index];
						perform_drawcall_for(buffers);

					}
					else if constexpr(same<C, setup_imgui_list>) {
						cmd_list = typed_cmd.cmd_list;
						fb_height = typed_cmd.fb_height;

						GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, p.triangle_buffer_id));
						buffer_data(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

						GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p.imgui_elements_id));
						buffer_data(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

						lists_to_delete.emplace_back(cmd_list);
						cmd_i = 0;
					}
					else if constexpr(same<C, make_screenshot>) {
						const auto& bounds = typed_cmd.bounds;
						const auto screenshot_size = bounds.get_size();

						auto& screenshot_buffer = output.result_screenshot;
						screenshot_buffer.emplace(screenshot_size);

						GL_CHECK(glReadPixels(bounds.x, bounds.y, bounds.w, bounds.h, GL_RGBA, GL_UNSIGNED_BYTE, screenshot_buffer->data()));
					}
					else if constexpr(same<C, no_arg_command>) {
						using N = no_arg_command;

						switch (typed_cmd) {
							case N::FULLSCREEN_QUAD: fullscreen_quad(); break;
							case N::CLEAR_CURRENT_FBO: clear_current_fbo(); break;
							case N::SET_STANDARD_BLENDING: set_standard_blending(); break;
							case N::SET_OVERWRITING_BLENDING: set_overwriting_blending(); break;
							case N::SET_ADDITIVE_BLENDING: set_additive_blending(); break;
							case N::CLEAR_STENCIL: clear_stencil(); break;
							case N::START_WRITING_STENCIL: start_writing_stencil(); break;
							case N::FINISH_WRITING_STENCIL: finish_writing_stencil(); break;
							case N::STENCIL_POSITIVE_TEST: stencil_positive_test(); break;
							case N::STENCIL_REVERSE_TEST: stencil_reverse_test(); break;
							case N::FINISH: finish(); break;

							case N::IMGUI_CMD: {
								ensure(fb_height > -1);
								const auto& cc = cmd_list->CmdBuffer[cmd_i++];
								const auto bounds = xywhi(
									(int)cc.ClipRect.x, (int)(fb_height - cc.ClipRect.w), (int)(cc.ClipRect.z - cc.ClipRect.x), (int)(cc.ClipRect.w - cc.ClipRect.y)
								);

								set_scissor_bounds(bounds);

								GL_CHECK(glDrawElements(
									GL_TRIANGLES, 
									(GLsizei)cc.ElemCount, 
									sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, 
									(void*)(intptr_t)(cc.IdxOffset * sizeof(ImDrawIdx))
								));

								break;
							}

						}

					}
					else if constexpr(same<C, toggle_command>) {
						const auto f = typed_cmd.flag;

						switch (typed_cmd.type) {
							case toggle_command_type::STENCIL: set_stencil(f); break;
							case toggle_command_type::BLENDING: set_blending(f); break;
							case toggle_command_type::SCISSOR: set_scissor(f); break;
							default: break;
						}
					}
					else if constexpr(same<C, set_active_texture_command>) {
						set_active_texture(typed_cmd.num);
					}
					else if constexpr(same<C, set_clear_color_command>) {
						set_clear_color(typed_cmd.col);
					}
					else if constexpr(same<C, set_scissor_bounds_command>) {
						set_scissor_bounds(typed_cmd.bounds);
					}
					else if constexpr(same<C, set_viewport_command>) {
						set_viewport(typed_cmd.bounds);
					}
					else {
						static_assert(always_false_v<C>, "Unimplemented command type!");
					}
				};

				std::visit(command_handler, cmd.payload);
			}
#else
			(void)c;
			(void)n;
			(void)output;
			(void)dedicated;
#endif
		}

		void renderer_backend::fullscreen_quad() {
			float vertices[] = {
				1.f, 1.f,
				1.f, 0.f,
				0.f, 0.f,

				0.f, 0.f,
				0.f, 1.f,
				1.f, 1.f
			};

			(void)vertices;

			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, platform->triangle_buffer_id));

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

		void renderer_backend::set_blending(const bool on) {
			if (on) {
				GL_CHECK(glEnable(GL_BLEND));
			}
			else {
				GL_CHECK(glDisable(GL_BLEND));
			}
		}

		void renderer_backend::enable_special_vertex_attribute() {
			GL_CHECK(glEnableVertexAttribArray(static_cast<int>(vertex_attribute::special)));
		}

		void renderer_backend::disable_special_vertex_attribute() {
			GL_CHECK(glDisableVertexAttribArray(static_cast<int>(vertex_attribute::special)));
		}

		void renderer_backend::set_clear_color(const rgba col) {
			const auto v = vec4(col);
			(void)v;
			GL_CHECK(glClearColor(v[0], v[1], v[2], v[3]));
		}

		void renderer_backend::clear_current_fbo() {
			GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
		}

		void renderer_backend::set_standard_blending() {
			GL_CHECK(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE));
		}

		void renderer_backend::set_overwriting_blending() {
			GL_CHECK(glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO));
		}

		void renderer_backend::set_additive_blending() {
			GL_CHECK(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE));
		}

		void renderer_backend::set_active_texture(const unsigned n) {
			GL_CHECK(glActiveTexture(GL_TEXTURE0 + n));
			(void)n;
		}


		void renderer_backend::set_viewport(const xywhi xywh) {
			GL_CHECK(glViewport(xywh.x, xywh.y, xywh.w, xywh.h));
			(void)xywh;
		}

		void renderer_backend::set_scissor(const bool on) {
			if (on) {
				GL_CHECK(glEnable(GL_SCISSOR_TEST));
			}
			else {
				GL_CHECK(glDisable(GL_SCISSOR_TEST));
			}
		}

		void renderer_backend::set_scissor_bounds(const xywhi bounds) {
			(void)bounds;
			GL_CHECK(glScissor(bounds.x, bounds.y, bounds.w, bounds.h));
		}

		void renderer_backend::set_stencil(const bool on) {
			if (on) {
				GL_CHECK(glEnable(GL_STENCIL_TEST));
			}
			else {
				GL_CHECK(glDisable(GL_STENCIL_TEST));
			}
		}

		void renderer_backend::clear_stencil() {
			GL_CHECK(glClear(GL_STENCIL_BUFFER_BIT));
		}

		void renderer_backend::start_writing_stencil() {
			GL_CHECK(glStencilFunc(GL_ALWAYS, 1, 0xFF));
			GL_CHECK(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
			GL_CHECK(glStencilMask(0xFF));
			GL_CHECK(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
		}

		void renderer_backend::stencil_reverse_test() {
			GL_CHECK(glStencilFunc(GL_EQUAL, 0, 0xFF));
		}

		void renderer_backend::stencil_positive_test() {
			GL_CHECK(glStencilFunc(GL_EQUAL, 1, 0xFF));
		}

		void renderer_backend::finish_writing_stencil() {
			stencil_reverse_test();
			GL_CHECK(glStencilMask(0x00));
			GL_CHECK(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
		}

		void renderer_backend::finish() {
			GL_CHECK(glFinish());
		}
	}
}
