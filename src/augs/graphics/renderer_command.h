#pragma once
#include <variant>
#include "augs/templates/object_command.h"

#include "augs/graphics/texture_commands.h"
#include "augs/graphics/shader_commands.h"

#include "augs/templates/settable_as_current_op.h"
#include "augs/graphics/renderer_commands.h"

namespace augs {
	namespace graphics {
		class fbo;
		class texture;
		class shader_program;

		using renderer_command_payload = std::variant<
			drawcall_command,
			drawcall_custom_buffer_command,
			drawcall_dedicated_command,
			drawcall_dedicated_vector_command,
			no_arg_command,
			setup_imgui_list,

			toggle_command,
			set_active_texture_command,
			set_clear_color_command,
			set_scissor_bounds_command,
			set_viewport_command,

			object_command<texture, texImage2D_command>,
			object_command<texture, set_filtering_command>,

			object_command<const shader_program, set_uniform_command>,
			object_command<const shader_program, set_projection_command>,

			object_command<const texture, settable_as_current_op_type>,
			static_object_command<const texture, settable_as_current_op_type>,

			object_command<const shader_program, settable_as_current_op_type>,
			static_object_command<const shader_program, settable_as_current_op_type>,

			object_command<const fbo, settable_as_current_op_type>,
			static_object_command<const fbo, settable_as_current_op_type>,

			make_screenshot
		>;

		struct renderer_command {
			renderer_command_payload payload;
		};
	}
}
