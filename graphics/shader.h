#pragma once
#include <string>
#include <vector>

namespace augmentations {
	namespace graphics {
		struct shader {
			GLuint id;
			bool built;

			shader(GLuint shader_type, std::string source_code);
			shader();
			~shader();

			void create(GLuint shader_type, std::string source_code);
			void destroy();
		};

		struct shader_program {
			static int currently_used_program;

			GLuint id;
			bool created, built;
			std::vector<shader*> attached_shaders;

			shader_program();
			~shader_program();

			void create();
			void attach(shader&);
			void build();
			void destroy();

			void use();
			void guarded_use();

			static void use_default();
		};
	}
}
