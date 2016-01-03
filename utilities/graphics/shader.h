#pragma once
#include <string>
#include <vector>

namespace augs {
	namespace graphics {
		struct shader {
			enum class type {
				VERTEX,
				FRAGMENT
			};

			unsigned int id;
			bool built;

			shader(type, std::string source_code);
			shader();
			~shader();

			void create(type shader_type, std::string source_code);
			void destroy();
		};

		struct shader_program {
			static int currently_used_program;

			unsigned int id;
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
