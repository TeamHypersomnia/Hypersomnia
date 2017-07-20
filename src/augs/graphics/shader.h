#pragma once
#include <string>
#include <array>

#include "augs/graphics/rgba.h"
#include "augs/templates/settable_as_current_mixin.h"
#include "augs/math/vec2.h"

using GLuint = unsigned int;
using GLint = int;

using vec3 = std::array<float, 3>;
using vec4 = std::array<float, 4>;

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
				const std::string& path
			);

			void destroy();

		public:
			shader(
				const type shader_type,
				const std::string& file_path
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

			friend class settable_as_current_base;

			bool set_as_current_impl() const;
			static void set_current_to_none_impl();

			void destroy();
		public:
			shader_program(
				shader&& vertex,
				shader&& fragment
			);

			shader_program(
				const std::string& vertex_shader_path,
				const std::string& fragment_shader_path
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
	}
}
