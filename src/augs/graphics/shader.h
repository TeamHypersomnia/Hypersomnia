#pragma once
#include <string>
#include <array>

#include "augs/graphics/rgba.h"
#include "augs/templates/settable_as_current_mixin.h"
#include "augs/templates/exception_templates.h"
#include "augs/math/vec2.h"
#include "augs/filesystem/path.h"

using GLuint = unsigned int;
using GLint = int;

#define STORE_SHADERS_IN_PROGRAM 0

namespace augs {
	namespace graphics {
		class shader {
		public:
			enum class type {
				VERTEX,
				FRAGMENT
			};

		private:
			friend class shader_program;

			GLuint id = 0xdeadbeef;
			bool built = false;

			void create(
				const type shader_type,
				const std::string& source_code
			);

			void destroy();

		public:
			shader(
				const type shader_type,
				const path_type& source_path
			);

			~shader();

			shader(shader&&);
			shader& operator=(shader&&);
			
			shader(const shader&) = delete;
			shader& operator=(const shader&) = delete;
		};

		class shader_program : public settable_as_current_mixin<const shader_program> {
			GLuint id = 0xdeadbeef;
			bool built = false;

#if STORE_SHADERS_IN_PROGRAM
			shader vertex;
			shader fragment;
#endif

			using settable_as_current_base = settable_as_current_mixin<const shader_program>;
			friend settable_as_current_base;

			bool set_as_current_impl() const;
			static void set_current_to_none_impl();

			void destroy();
		public:
			shader_program(
				shader&& vertex,
				shader&& fragment
			);

			shader_program(
				const path_type& vertex_shader_path,
				const path_type& fragment_shader_path
			);

			~shader_program();

			shader_program(shader_program&&);
			shader_program& operator=(shader_program&&);

			shader_program(const shader_program&) = delete;
			shader_program& operator=(const shader_program&) = delete;

			GLint get_uniform_location(const std::string& uniform_name) const;
			
			void set_projection(const std::array<float, 16> matrix) const;
			void set_projection(const vec2 for_screen_size) const;

			void set_uniform(const GLint id, const vec2) const;
			void set_uniform(const GLint id, const vec2i) const;
			void set_uniform(const GLint id, const rgba) const;
			void set_uniform(const GLint id, const rgba::rgb_type) const;
			void set_uniform(const GLint id, const vec3) const;
			void set_uniform(const GLint id, const vec4) const;
			void set_uniform(const GLint id, const float) const;
			void set_uniform(const GLint id, const int) const;

			template <class... Args>
			void set_uniform(const std::string& name, Args&&... args) const {
				set_uniform(get_uniform_location(name), std::forward<Args>(args)...);
			}
		};

		struct shader_error : error_with_typesafe_sprintf {
			using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
		};

		struct shader_compilation_error : shader_error {
			static auto describe_shader_type(const shader::type t) {
				std::string result;

				switch (t) {
					case shader::type::VERTEX: result = "a vertex shader"; break;
					case shader::type::FRAGMENT: result = "a fragment shader"; break;
					default: result = "an unknown shader"; break;
				}

				return result;
			}

			explicit shader_compilation_error(
				shader::type type,
				const std::string& shader_source,
				const std::string& error_message
			) : 
				shader_error(
					"Failed to compile %x. Error message:\n%x\nSource code: %x",
					describe_shader_type(type),
					shader_source,
					error_message
				) 
			{}
		};

		struct shader_program_build_error : shader_error {
			explicit shader_program_build_error(
				const GLuint id,
				const std::string& error_message
			) : 
				shader_error(
					"Failed to build shader program id: %x. Error message:\n%x",
					id,
					error_message
				) 
			{}
		};
	}
}
