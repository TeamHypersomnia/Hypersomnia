#include <GL/OpenGL.h>
#include <cassert>
#include "shader.h"
#include "augs/ensure.h"

namespace augs {
	namespace graphics {
		void printShaderInfoLog(GLuint obj, std::string source_code) {
			int infologLength = 0;
			int charsWritten = 0;
			char *infoLog;

			glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength); glerr

			if (infologLength > 1) {
				infoLog = (char *) malloc(infologLength);
				glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog); glerr
				printf("---------------------------------\n Source code: %s ---------------------------------\n%s\n", source_code.c_str(), infoLog);
				free(infoLog);
				ensure(0);
			}
		}

		void printProgramInfoLog(GLuint obj) {
			int infologLength = 0;
			int charsWritten = 0;
			char *infoLog;

			glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength); glerr

			if (infologLength > 1) {
				infoLog = (char *) malloc(infologLength);
				glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog); glerr
				printf("---------------------------------\n%s\n", infoLog);
				free(infoLog);
				ensure(0);
			}
		}


		shader::shader() : built(false) {}
		
		shader::shader(type shader_type, std::string source_code) : shader() {
			create(shader_type, source_code);
		}

		shader::~shader() {
			destroy();
		}

		void shader::create(type shader_type, std::string source_code) {
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

		shader_program::shader_program() : id(0u), created(false), built(false) {}
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

		void shader_program::use() {
			if (!built) build();

			currently_used_program = id;
			glUseProgram(id); glerr
		}

		void shader_program::guarded_use() {
			if (currently_used_program != id)
				use();
		}
	}
}
