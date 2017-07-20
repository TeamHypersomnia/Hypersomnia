#include "shader.h"

#include "augs/graphics/OpenGL_includes.h"

#include "augs/ensure.h"
#include "augs/filesystem/file.h"
#include "augs/math/matrix.h"

namespace augs {
	namespace graphics {
		void log_shader(const GLuint obj, const std::string& source_code) {
			int infologLength = 0;
			int charsWritten = 0;
			char *infoLog;

			glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength); glerr

			if (infologLength > 1) {
				infoLog = (char *) malloc(infologLength);
				glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog); glerr
				LOG("---------------------------------\n Source code: \n%x ---------------------------------\n%x\n", source_code.c_str(), infoLog);
				free(infoLog);
				ensure(false);
			}
		}

		void log_shader_program(const GLuint obj) {
			int infologLength = 0;
			int charsWritten = 0;
			char *infoLog;

			glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength); glerr

			if (infologLength > 1) {
				infoLog = (char *) malloc(infologLength);
				glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog); glerr
				LOG("---------------------------------\n%x\n", infoLog);
				free(infoLog);
				ensure(false);
			}
		}

		shader::shader(
			const type shader_type,
			const std::string& path
		) {
			create(shader_type, "// " + path + "\n" + get_file_contents(path));
		}

		shader::~shader() {
			destroy();
		}

		shader::shader(shader&& b) : 
			built(b.built), 
			id(b.id) 
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
				id = glCreateShader(GL_VERTEX_SHADER); 
				glerr;
			}
			if (shader_type == type::FRAGMENT) {
				id = glCreateShader(GL_FRAGMENT_SHADER); 
				glerr;
			}

			auto* const source_ptr = source_code.c_str();
			glShaderSource(id, 1, &source_ptr, nullptr); glerr
			glCompileShader(id); glerr

			log_shader(id, source_code);
		}

		void shader::destroy() {
			if (built) {
				glDeleteShader(id); glerr
				built = false;
			}
		}
		
		shader_program::shader_program(shader_program&& b) :
			settable_as_current_base(static_cast<settable_as_current_base&&>(b)),
			built(b.built),
			id(b.id)
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
			const std::string& vertex_shader_path,
			const std::string& fragment_shader_path
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
			id = glCreateProgram(); glerr

			glAttachShader(id, new_vertex.id); glerr
			glAttachShader(id, new_fragment.id); glerr

			glLinkProgram(id); glerr
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

				glDeleteProgram(id); glerr
			}
		}

		void shader_program::set_current_to_none_impl() {
			glUseProgram(0); glerr
		}

		bool shader_program::set_as_current_impl() const {
			glUseProgram(id); glerr
			return true;
		}
		
		GLint shader_program::get_uniform_location(const std::string& uniform_name) const {
			return glGetUniformLocation(id, uniform_name.c_str());
		}

		void shader_program::set_projection(const std::array<float, 16> matrix) const {
			glUniformMatrix4fv(get_uniform_location("projection_matrix"), 1, GL_FALSE, matrix.data());
		}

		void shader_program::set_projection(const vec2 for_screen_size) const {
			set_projection(augs::orthographic_projection(for_screen_size));
		}

		void shader_program::set_uniform(const GLint id, const vec2 v) const {
			glUniform2f(id, v.x, v.y);
		}

		void shader_program::set_uniform(const GLint id, const vec2i v) const {
			glUniform2i(id, v.x, v.y);
		}

		void shader_program::set_uniform(const GLint id, const rgba r) const {
			set_uniform(id, vec4(r));
		}

		void shader_program::set_uniform(const GLint id, const rgba::rgb_type r) const {
			set_uniform(id, vec3(r));
		}

		void shader_program::set_uniform(const GLint id, const vec3 v) const {
			glUniform3f(id, v[0], v[1], v[2]);
		}
		
		void shader_program::set_uniform(const GLint id, const vec4 v) const {
			glUniform4f(id, v[0], v[1], v[2], v[3]);
		}

		void shader_program::set_uniform(const GLint id, const float v) const {
			glUniform1f(id, v);
		}

		void shader_program::set_uniform(const GLint id, const int v) const {
			glUniform1i(id, v);
		}
	}
}
