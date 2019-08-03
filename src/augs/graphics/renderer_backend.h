#pragma once
#include <memory>
#include "augs/graphics/renderer_command_enums.h"
#include "augs/graphics/rgba.h"
#include "augs/math/rects.h"
#include "augs/math/vec2.h"

#include "augs/templates/exception_templates.h"

namespace augs {
	struct dedicated_buffers;
	struct drawcall_command;

	namespace graphics {
		struct renderer_error : error_with_typesafe_sprintf {
			using error_with_typesafe_sprintf::error_with_typesafe_sprintf; 
		};

		struct renderer_command;

		class renderer_backend {
			struct platform_data;
			std::unique_ptr<platform_data> platform;

			unsigned max_texture_size = static_cast<unsigned>(-1);

			void set_active_texture(const unsigned);
			void fullscreen_quad();

			void set_clear_color(const rgba);
			void clear_current_fbo();

			void set_blending(bool);
			void set_scissor(bool);
			void set_stencil(bool);

			void set_scissor_bounds(xywhi);

			void set_standard_blending();
			void set_overwriting_blending();
			void set_additive_blending();
			
			void enable_special_vertex_attribute();
			void disable_special_vertex_attribute();
			void set_viewport(const xywhi);

			void clear_stencil();

			void start_writing_stencil();
			void start_testing_stencil();

			void stencil_positive_test();
			void stencil_reverse_test();

			void perform(const drawcall_command&);

		public:
			unsigned get_max_texture_size() const;

			renderer_backend();
			~renderer_backend();

			renderer_backend(renderer_backend&&) = delete;
			renderer_backend& operator=(renderer_backend&&) = delete;

			renderer_backend(const renderer_backend&) = delete;
			renderer_backend& operator=(const renderer_backend&) = delete;

			void perform(
				const renderer_command*, 
				std::size_t n,
				const dedicated_buffers&
			);
		};
	}
}
