#include "augs/gui/text/printer.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "view/audiovisual_state/audiovisual_profiler.h"

#include "application/session_profiler.h"
#include "application/main/draw_debug_details.h"

#include "generated/introspectors.h"
#include "hypersomnia_version.h"

void draw_debug_details(
	const augs::drawer output,
	const augs::baked_font& gui_font,
	const vec2i screen_size,
	const const_entity_handle viewed_character,
	const session_profiler& session_performance,
	const audiovisual_profiler& audiovisual_performance
) {
	using namespace augs::gui::text;

	print(
		output, 
		vec2i(screen_size.x - 300, 0),
		augs::gui::text::format_recent_program_log(gui_font).multiply_alpha(150.f / 255),
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

	static const auto version = hypersomnia_version();

	total_details += {
		to_wstring(typesafe_sprintf(
			"Revision no.: %x %x\nDate: %x\nMessage:\n%x\n",
			version.commit_number ? std::to_string(version.commit_number) : "Unknown",
			version.commit_number ? version.working_tree_changes.empty() ? "(clean)" : "(dirty)" : "",
			version.commit_date,
			version.commit_message.size() < 30 ? version.commit_message : version.commit_message.substr(0, 30) + "(...)"
		)),
		
		text_style
	};

	if (viewed_character.alive()) {
		{
			const auto coords = viewed_character.get_logic_transform().pos;
			const auto rot = viewed_character.get_logic_transform().rotation;

			total_details += {
				typesafe_sprintf(
					L"Entities: %x\nX: %f2\nY: %f2\nRot: %f2\n",
					viewed_character.get_cosmos().get_entities_count(),
					coords.x,
					coords.y,
					rot
				),

				text_style
			};
		}

		if (const auto maybe_body = viewed_character.find<components::rigid_body>()) {
			const auto vel = maybe_body.velocity();

			total_details += {
				typesafe_sprintf(
					L"VelX: %x\nVelY : %x\n",
					vel.x,
					vel.y
				),
				
				text_style
			};
		}

	}

	total_details += { L"Session\n", category_style };
	total_details += { session_performance.summary(), text_style };
	total_details += { L"Audiovisual\n", category_style };
	total_details += { audiovisual_performance.summary(), text_style };
	total_details += { L"Cosmos\n", category_style };

	if (viewed_character.alive()) {
		total_details += { viewed_character.get_cosmos().profiler.summary(), text_style };
	}

	print(output, { 0, 0 }, total_details);
}