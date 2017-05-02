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

			unsigned int id = 0xdeadbeef;
			bool built = false;

			shader() = default;
			~shader();

			shader(const shader&) = delete;
			shader(shader&&) = delete;
			shader& operator=(const shader&) = delete;
			shader& operator=(shader&&) = delete;

			void create(
				const type shader_type, 
				const std::string& path
			);

			void create_from_file(
				const type shader_type,
				const std::string& filename
			);

			void destroy();
		};

		struct shader_program {
			static int currently_used_program;

			unsigned int id = 0xdeadbeef;
			bool created = false;
			bool built = false;

			std::vector<shader*> attached_shaders;

			shader_program() = default;
			~shader_program();

			shader_program(const shader_program&) = delete;
			shader_program(shader_program&&) = delete;
			shader_program& operator=(const shader_program&) = delete;
			shader_program& operator=(shader_program&&) = delete;

			void create();
			void attach(shader&);
			void build();
			void destroy();

			void use() const;
			void guarded_use() const;

			static void use_default();
		};
	}
}
