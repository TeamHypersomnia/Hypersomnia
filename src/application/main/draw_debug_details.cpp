#include "augs/gui/text/printer.h"
#include "augs/templates/introspect.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

#include "view/frame_profiler.h"
#include "view/audiovisual_state/audiovisual_profiler.h"
#include "view/viewables/streaming/viewables_streaming_profiler.h"

#include "application/session_profiler.h"
#include "application/main/draw_debug_details.h"

#include "build_info.h"

void draw_debug_details(
	const augs::drawer output,
	const augs::baked_font& gui_font,
	const vec2i screen_size,
	const const_entity_handle viewed_character,
	const frame_profiler& frame_performance,
	const viewables_streaming_profiler& streaming_performance,
	const atlas_profiler& general_atlas_performance,
	const session_profiler& session_performance,
	const audiovisual_profiler& audiovisual_performance
) {
	using namespace augs::gui::text;

	print(
		output, 
		vec2i(screen_size.x - 300, 0),
		augs::gui::text::format_recent_program_log(gui_font).mult_alpha(150.f / 255),
		300
	);

	thread_local auto total_details = formatted_string();
	total_details.clear();

	static const auto text_style = style(
		gui_font,
		rgba(255, 255, 255, 150)
	);

	static const auto category_style = style(
		gui_font,
		rgba(yellow.rgb(), 150)
	);

#if 0
	total_details += {
		hypersomnia_version().get_summary() + static_allocations_info(),
		
		text_style
	};
#endif

	if (viewed_character.alive()) {
		if (const auto transform = viewed_character.find_logic_transform()) {
			const auto coords = transform->pos;
			const auto rot = transform->rotation;

			total_details += {
				typesafe_sprintf(
					"Entities: %x\nX: %f2\nY: %f2\nRot: %f2\n",
					viewed_character.get_cosmos().get_entities_count(),
					coords.x,
					coords.y,
					rot
				),

				text_style
			};
		}

		if (const auto maybe_body = viewed_character.find<components::rigid_body>()) {
			const auto vel = maybe_body.get_velocity();

			total_details += {
				typesafe_sprintf(
					"VelX: %x\nVelY : %x\n",
					vel.x,
					vel.y
				),
				
				text_style
			};
		}

	}

	total_details += { "Session\n", category_style };
	total_details += { session_performance.summary(), text_style };
	total_details += { "Frame\n", category_style };
	total_details += { frame_performance.summary(), text_style };

	total_details += { "Cosmos\n", category_style };

	if (viewed_character.alive()) {
		total_details += { viewed_character.get_cosmos().profiler.summary(), text_style };
	}

	total_details += { "Audiovisual\n", category_style };
	total_details += { audiovisual_performance.summary(), text_style };

	total_details += { "Viewables streaming\n", category_style };
	total_details += { streaming_performance.summary(), text_style };

	total_details += { "General atlas\n", category_style };
	total_details += { general_atlas_performance.summary(), text_style };

	print(output, { 0, 0 }, total_details);
}
