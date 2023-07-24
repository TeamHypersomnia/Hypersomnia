#pragma once
#include "augs/graphics/renderer_command_enums.h"
#include "3rdparty/imgui/imgui.h"
#include "augs/graphics/vertex.h"
#include "augs/graphics/dedicated_buffers.h"

namespace augs {
	enum class no_arg_command {
		FULLSCREEN_QUAD,
		CLEAR_CURRENT_FBO,
		SET_STANDARD_BLENDING,
		SET_OVERWRITING_BLENDING,
		SET_ADDITIVE_BLENDING,
		CLEAR_STENCIL,
		START_WRITING_STENCIL,
		FINISH_WRITING_STENCIL,
		STENCIL_POSITIVE_TEST,
		STENCIL_REVERSE_TEST,
		FINISH,

		IMGUI_CMD
	};

	enum class toggle_command_type : unsigned char {
		BLENDING,
		STENCIL,
		SCISSOR
	};

	struct toggle_command {
		toggle_command_type type;
		bool flag;
	};

	struct set_active_texture_command {
		unsigned num;
	};

	struct set_clear_color_command {
		rgba col;
	};

	struct drawcall_custom_buffer_command {
		vertex_triangle_buffer buffer;
	};

	struct drawcall_command {
		const vertex_triangle* triangles = nullptr;
		const vertex_line* lines = nullptr;
		const special* specials = nullptr;

		uint32_t count = 0;
	};

	struct drawcall_dedicated_command {
		dedicated_buffer type;
	};

	struct drawcall_dedicated_vector_command {
		dedicated_buffer_vector type;
		uint32_t index = 0;
	};

	struct set_viewport_command {
		xywhi bounds;
	};

	struct set_scissor_bounds_command {
		xywhi bounds;
	};

	struct make_screenshot {
		xywhi bounds;
	};

	struct setup_imgui_list {
		ImDrawList* cmd_list;
		int fb_height = -1;
	};
}
