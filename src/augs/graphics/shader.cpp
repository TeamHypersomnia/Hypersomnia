#include "augs/log.h"
#include "augs/graphics/shader.h"
#include "augs/ensure.h"
#include "augs/filesystem/file.h"
#include "augs/math/matrix.h"

#include "augs/graphics/OpenGL_includes.h"
#include "augs/graphics/backend_access.h"
#include "augs/graphics/renderer.h"
#include "augs/templates/enum_introspect.h"
#include "augs/window_framework/window.h"

namespace augs {
	namespace graphics {
		void log_shader(
			const shader::type shader_type,
			const GLuint obj, 
			const std::string& source_code
		) {
			int info_log_length = 0;

			GL_CHECK(glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &info_log_length));

			if (info_log_length > 1) {
				std::string info_log;
				info_log.resize(info_log_length);

				int chars_written = 0;
				(void)chars_written;

				GL_CHECK(glGetShaderInfoLog(
					obj, 
					info_log_length, 
					&chars_written, 
					info_log.data()
				));

				info_log.pop_back();

				throw shader_compilation_error(shader_type, source_code, info_log);
			}

			(void)obj;
		}

		void log_shader_program(const GLuint obj) {
			int info_log_length = 0;

			GL_CHECK(glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &info_log_length));

			if (info_log_length > 1) {
				std::string info_log;
				info_log.resize(info_log_length);

				int chars_written = 0;
				(void)chars_written;

				GL_CHECK(glGetProgramInfoLog(
					obj, 
					info_log_length, 
					&chars_written, 
					info_log.data()
				));
				
				info_log.pop_back();

				throw shader_program_build_error(obj, info_log);
			}
		}

		shader::shader(
			const type shader_type,
			const path_type& path
		) try {
#if PLATFORM_MACOS
			const auto glsl_version = "#version 150";
#elif PLATFORM_WEB
			const auto glsl_version = "#version 300 es";
#else
			const auto glsl_version = "#version 130";
#endif
			create(shader_type, std::string(glsl_version) + "\n" + "// " + path.string() + "\n" + file_to_string(path));
		}
		catch (const augs::file_open_error& err) {
			const auto cwd = std::filesystem::current_path().string();
			
			throw shader_error(
				"Failed to load shader file %x:\n%x (cwd: %x)", path, err.what(), cwd
			);
		}

		shader::~shader() {
			destroy();
		}

		shader::shader(shader&& b) : 
			id(b.id),
			built(b.built)
		{
			b.built = false;
		}

		shader& shader::operator=(shader&& b) {
			destroy();

			built = b.built;
			id = b.id;

			b.built = false;

			return *this;
		}

		void shader::create(
			const type shader_type,
			const std::string& source_code
		) {
#if BUILD_OPENGL
			built = true;

			augs::window::get_current().check_current_context();

			if (shader_type == type::VERTEX) {
				GL_CHECK(id = glCreateShader(GL_VERTEX_SHADER)); 
			}
			if (shader_type == type::FRAGMENT) {
				GL_CHECK(id = glCreateShader(GL_FRAGMENT_SHADER));
			}

			auto* const source_ptr = source_code.c_str();
			(void)source_ptr;
			GL_CHECK(glShaderSource(id, 1, &source_ptr, nullptr));
			GL_CHECK(glCompileShader(id));

			GLint compilation_status = GL_FALSE;
			GL_CHECK(glGetShaderiv(id, GL_COMPILE_STATUS, std::addressof(compilation_status)));

			if (compilation_status == GL_FALSE) {
				LOG("GL_COMPILE_STATUS returned GL_FALSE. Printing error log.");
				log_shader(shader_type, id, source_code);
			}
			else {

			}
#else
			(void)shader_type;
			(void)source_code;
#endif
		}

		void shader::destroy() {
			if (built) {
				augs::window::get_current().check_current_context();

				GL_CHECK(glDeleteShader(id));
				built = false;
			}
		}
		
#if 0
		shader_program::shader_program(shader_program&& b) :
			settable_as_current_base(static_cast<settable_as_current_base&&>(b)),
			id(b.id),
			built(b.built)
#if STORE_SHADERS_IN_PROGRAM
			, vertex(std::move(b.vertex)),
			fragment(std::move(b.fragment))
#endif
		{
			b.built = false;
		}

		shader_program& shader_program::operator=(shader_program&& b) {
			settable_as_current_base::operator=(static_cast<settable_as_current_base&&>(b));

			destroy();

			built = b.built;
			id = b.id;
#if STORE_SHADERS_IN_PROGRAM
			vertex = std::move(b.vertex);
			fragment = std::move(b.fragment);
#endif

			b.built = false;

			return *this;
		}
#endif

		shader_program::~shader_program() { 
			destroy(); 
		}

		shader_program::shader_program(
			const path_type& vertex_shader_path,
			const path_type& fragment_shader_path
		) : shader_program(
			shader(shader::type::VERTEX, vertex_shader_path),
			shader(shader::type::FRAGMENT, fragment_shader_path)
		) {

		}

		shader_program::shader_program(
			shader&& new_vertex, 
			shader&& new_fragment
		) 
#if STORE_SHADERS_IN_PROGRAM
			:
			vertex(std::move(new_vertex)),
			fragment(std::move(new_fragment))
#endif
		{
#if BUILD_OPENGL
			augs::window::get_current().check_current_context();

			GL_CHECK(id = glCreateProgram());

			GL_CHECK(glAttachShader(id, new_vertex.id));
			GL_CHECK(glAttachShader(id, new_fragment.id));

			GL_CHECK(glBindAttribLocation(id, 0, "position"));
			GL_CHECK(glBindAttribLocation(id, 1, "texcoord"));
			GL_CHECK(glBindAttribLocation(id, 2, "color"));
			GL_CHECK(glBindAttribLocation(id, 3, "special"));

			GL_CHECK(glLinkProgram(id));

			GLint link_status = GL_FALSE;
			GL_CHECK(glGetProgramiv(id, GL_LINK_STATUS, std::addressof(link_status)));

			if (link_status == GL_FALSE) {
				LOG("GL_LINK_STATUS returned GL_FALSE. Printing error log.");
				log_shader_program(id);
			}
			else {

			}

#if !STORE_SHADERS_IN_PROGRAM
			new_vertex.destroy();
			new_fragment.destroy();
#endif

			built = true;

			for_each_enum_except_bounds([&](const common_uniform_name u) {
				uniform_map[u] = get_uniform_location(enum_to_string(u));
			});
#else
			(void)new_vertex;
			(void)new_fragment;
#endif
		}

		void shader_program::destroy() {
			if (built) {
				built = false;

				GL_CHECK(glDeleteProgram(id));
			}
		}

		void shader_program::set_current_to_none_impl(backend_access) {
			GL_CHECK(glUseProgram(0));
		}

		bool shader_program::set_as_current_impl(backend_access) const {
			GL_CHECK(glUseProgram(id));
			return true;
		}
		
		void shader_program::perform(backend_access, const set_uniform_command& cmd) const {
			auto uniform_setter = [&](const auto& typed_payload){
				set_uniform(cmd.uniform_id, typed_payload);
			};

			std::visit(uniform_setter, cmd.payload);
		}

		void shader_program::perform(backend_access, const set_projection_command& cmd) const {
			set_projection(cmd.payload);
		}

		void shader_program::set_projection(renderer& r, const std::array<float, 16> matrix) const {
			r.push_object_command(*this, set_projection_command { matrix });
		}

		GLint shader_program::get_uniform_location(const std::string& uniform_name) const {
#if BUILD_OPENGL
			return glGetUniformLocation(id, uniform_name.c_str());
#else
			(void)uniform_name;
			return 0xdeadbeef;
#endif
		}

		void shader_program::set_projection(const std::array<float, 16> matrix) const {
			GL_CHECK(glUniformMatrix4fv(uniform_map[common_uniform_name::projection_matrix], 1, GL_FALSE, matrix.data()));
			(void)matrix;
		}

		void shader_program::set_uniform(const GLint id, const vec2 v) const {
			GL_CHECK(glUniform2f(id, v.x, v.y));
			(void)id;
			(void)v;
		}

		void shader_program::set_uniform(const GLint id, const vec2i v) const {
			GL_CHECK(glUniform2i(id, v.x, v.y));
			(void)id;
			(void)v;
		}

		void shader_program::set_uniform(const GLint id, const rgba r) const {
			set_uniform(id, vec4(r));
		}

		void shader_program::set_uniform(const GLint id, const rgba::rgb_type r) const {
			set_uniform(id, vec3(r));
		}

		void shader_program::set_uniform(const GLint id, const vec3 v) const {
			GL_CHECK(glUniform3f(id, v[0], v[1], v[2]));
			(void)id;
			(void)v;
		}

		void shader_program::set_uniform(const GLint id, const vec4 v) const {
			GL_CHECK(glUniform4f(id, v[0], v[1], v[2], v[3]));
			(void)id;
			(void)v;
		}

		void shader_program::set_uniform(const GLint id, const float v) const {
			GL_CHECK(glUniform1f(id, v));
			(void)id;
			(void)v;
		}

		void shader_program::set_uniform(const GLint id, const int v) const {
			GL_CHECK(glUniform1i(id, v));
			(void)id;
			(void)v;
		}
	}
}
