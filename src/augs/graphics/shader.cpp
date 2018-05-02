#include "augs/graphics/shader.h"
#include "augs/ensure.h"
#include "augs/filesystem/file.h"
#include "augs/math/matrix.h"

#include "augs/graphics/OpenGL_includes.h"

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

				GL_CHECK(glGetShaderInfoLog(
					obj, 
					info_log_length, 
					&chars_written, 
					info_log.data()
				));

				info_log.pop_back();

				throw shader_compilation_error(shader_type, source_code, info_log);
			}
		}

		void log_shader_program(const GLuint obj) {
			int info_log_length = 0;

			GL_CHECK(glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &info_log_length));

			if (info_log_length > 1) {
				std::string info_log;
				info_log.resize(info_log_length);

				int chars_written = 0;

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
			create(shader_type, "// " + path.string() + "\n" + file_to_string(path));
		}
		catch (const augs::file_open_error& err) {
			throw shader_error(
				"Failed to load shader file %x:\n%x", path, err.what()
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
			built = true;

			if (shader_type == type::VERTEX) {
				GL_CHECK(id = glCreateShader(GL_VERTEX_SHADER)); 
			}
			if (shader_type == type::FRAGMENT) {
				GL_CHECK(id = glCreateShader(GL_FRAGMENT_SHADER));
			}

			auto* const source_ptr = source_code.c_str();
			GL_CHECK(glShaderSource(id, 1, &source_ptr, nullptr));
			GL_CHECK(glCompileShader(id));

			log_shader(shader_type, id, source_code);
		}

		void shader::destroy() {
			if (built) {
				GL_CHECK(glDeleteShader(id));
				built = false;
			}
		}
		
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
			GL_CHECK(id = glCreateProgram());

			GL_CHECK(glAttachShader(id, new_vertex.id));
			GL_CHECK(glAttachShader(id, new_fragment.id));

			GL_CHECK(glLinkProgram(id));
			log_shader_program(id);

#if !STORE_SHADERS_IN_PROGRAM
			new_vertex.destroy();
			new_fragment.destroy();
#endif

			built = true;
		}

		void shader_program::destroy() {
			if (built) {
				built = false;

				GL_CHECK(glDeleteProgram(id));
			}
		}

		void shader_program::set_current_to_none_impl() {
			GL_CHECK(glUseProgram(0));
		}

		bool shader_program::set_as_current_impl() const {
			GL_CHECK(glUseProgram(id));
			return true;
		}
		
		GLint shader_program::get_uniform_location(const std::string& uniform_name) const {
#if BUILD_OPENGL
			return glGetUniformLocation(id, uniform_name.c_str());
#else
			return 0xdeadbeef;
#endif
		}

		void shader_program::set_projection(const std::array<float, 16> matrix) const {
			GL_CHECK(glUniformMatrix4fv(get_uniform_location("projection_matrix"), 1, GL_FALSE, matrix.data()));
		}

		void shader_program::set_projection(const vec2 for_screen_size) const {
			set_projection(augs::orthographic_projection(for_screen_size));
		}

		void shader_program::set_uniform(const GLint id, const vec2 v) const {
			GL_CHECK(glUniform2f(id, v.x, v.y));
		}

		void shader_program::set_uniform(const GLint id, const vec2i v) const {
			GL_CHECK(glUniform2i(id, v.x, v.y));
		}

		void shader_program::set_uniform(const GLint id, const rgba r) const {
			set_uniform(id, vec4(r));
		}

		void shader_program::set_uniform(const GLint id, const rgba::rgb_type r) const {
			set_uniform(id, vec3(r));
		}

		void shader_program::set_uniform(const GLint id, const vec3 v) const {
			GL_CHECK(glUniform3f(id, v[0], v[1], v[2]));
		}
		
		void shader_program::set_uniform(const GLint id, const vec4 v) const {
			GL_CHECK(glUniform4f(id, v[0], v[1], v[2], v[3]));
		}

		void shader_program::set_uniform(const GLint id, const float v) const {
			GL_CHECK(glUniform1f(id, v));
		}

		void shader_program::set_uniform(const GLint id, const int v) const {
			GL_CHECK(glUniform1i(id, v));
		}
	}
}
