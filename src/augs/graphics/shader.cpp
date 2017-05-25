#include "shader.h"

#include "augs/graphics/OpenGL_includes.h"

#include "augs/ensure.h"
#include "augs/filesystem/file.h"

namespace augs {
	namespace graphics {
		void printShaderInfoLog(const GLuint obj, const std::string& source_code) {
			int infologLength = 0;
			int charsWritten = 0;
			char *infoLog;

			glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength); glerr

			if (infologLength > 1) {
				infoLog = (char *) malloc(infologLength);
				glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog); glerr
				LOG("---------------------------------\n Source code: %x ---------------------------------\n%x\n", source_code.c_str(), infoLog);
				free(infoLog);
				ensure(false);
			}
		}

		void printProgramInfoLog(const GLuint obj) {
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

		shader::~shader() {
			destroy();
		}


		void shader::create_from_file(
			const type shader_type,
			const std::string& path
		) {
			create(shader_type, get_file_contents(path));
		}

		void shader::create(
			const type shader_type,
			const std::string& source_code
		) {
			if (!built) {
				built = true;

				if (shader_type == type::VERTEX)
					id = glCreateShader(GL_VERTEX_SHADER); glerr;
				if (shader_type == type::FRAGMENT)
					id = glCreateShader(GL_FRAGMENT_SHADER); glerr;

				auto* source_ptr = source_code.c_str();
				glShaderSource(id, 1, &source_ptr, nullptr); glerr
				glCompileShader(id); glerr
				printShaderInfoLog(id, source_code);
			}
		}

		void shader::destroy() {
			if (built) {
				glDeleteShader(id); glerr
				built = false;
			}
		}

		/***********************************************************************************************************************************/
		/***********************************************************************************************************************************/

		int shader_program::currently_used_program = 0;

		shader_program::~shader_program() { 
			destroy(); 
		}

		void shader_program::create() {
			if (!created) {
				id = glCreateProgram(); glerr
				created = true;
			}
		}

		void shader_program::attach(shader& s) {
			create();

			attached_shaders.push_back(&s);
			glAttachShader(id, s.id); glerr
		}
		
		void shader_program::build() {
			ensure(created && !built);
			
			built = true;
			glLinkProgram(id); glerr
			printProgramInfoLog(id);

			for (auto it : attached_shaders) {
				//glDetachShader(id, it->id);
				it->destroy();
			}

			attached_shaders.clear();
		}

		void shader_program::destroy() {
			if (created) {
				built = created = false;

				if (currently_used_program == id) 
					use_default();

				glDeleteProgram(id); glerr
			}
		}

		void shader_program::use_default() {
			if (currently_used_program != 0) {
				glUseProgram(0); glerr
				currently_used_program = 0;
			}
		}

		void shader_program::use() const {
			ensure(built);
			currently_used_program = id;
			glUseProgram(id); glerr
		}

		void shader_program::guarded_use() const {
			if (currently_used_program != id) {
				use();
			}
		}
	}
}
