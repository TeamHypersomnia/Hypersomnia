#include "augs/gui/text/printer.h"
#include "augs/templates/introspect.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

#include "view/frame_profiler.h"
#include "view/audiovisual_state/audiovisual_profiler.h"
#include "view/viewables/streaming/viewables_streaming_profiler.h"

#include "application/session_profiler.h"
#include "application/main/draw_debug_details.h"
#include "augs/network/network_types.h"

#include "build_info.h"

void draw_debug_details(
	const augs::drawer output,
	const augs::baked_font& gui_font,
	const vec2i screen_size,
	const const_entity_handle viewed_character,
	const frame_profiler& frame_performance,
	const network_profiler& network_performance,
	const network_info& network_stats,
	const server_network_info& server_stats,
	const viewables_streaming_profiler& streaming_performance,
	const atlas_profiler& general_atlas_performance,
	const session_profiler& session_performance,
	const audiovisual_profiler& audiovisual_performance
) {
	using namespace augs::gui::text;

	const auto log_line_width = 600;
	const auto net_line_width = 400;
	const auto cosm_line_width = 400;

	{
		print(
			output, 
			vec2i(screen_size.x - log_line_width, 0),
			augs::gui::text::format_recent_program_log(gui_font).mult_alpha(150.f / 255),
			log_line_width
		);
	}

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

	const auto& cosm = viewed_character.get_cosmos();

	total_details += { typesafe_sprintf("Entities: %x\n", cosm.get_entities_count()), text_style };

	if (false && viewed_character.alive()) {
		if (const auto transform = viewed_character.find_logic_transform()) {
			const auto coords = transform->pos;
			const auto rot = transform->rotation;

			total_details += {
				typesafe_sprintf(
					"X: %f2\nY: %f2\nRot: %f2\n",
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


	total_details += { "Audiovisual\n", category_style };
	total_details += { audiovisual_performance.summary(), text_style };

	total_details += { "Viewables streaming\n", category_style };
	total_details += { streaming_performance.summary(), text_style };

	total_details += { "General atlas\n", category_style };
	total_details += { general_atlas_performance.summary(), text_style };

	print(output, { 0, 0 }, total_details);

	{
		total_details.clear();

		total_details += { "Network\n", category_style };
		total_details += { network_performance.summary(), text_style };

		const bool in_bytes = false;

		auto make_readable = [&](const auto kbits) {
			if (in_bytes) {
				return readable_bytesize(kbits / 8 * 1000);
			}
			
			return readable_bitsize(kbits * 1000);
		};

		if (network_stats.are_set()) {
			total_details += { "Connection stats\n", category_style };

			const auto loss_percent = 
				typesafe_sprintf("Loss: %2f", network_stats.loss_percent) + "%" "\n"
			;

			const auto stats_summary = loss_percent + typesafe_sprintf(
				"RTT: %x ms" "\n"
				"Sent: %x/s" "\n"
				"Rcvd: %x/s" "\n"
				"Ackd: %x/s" "\n"
				"# sent: %x" "\n"
				"# rcvd: %x" "\n"
				"# ackd: %x" "\n",
				network_stats.rtt_ms,
				make_readable(network_stats.sent_kbps),
				make_readable(network_stats.received_kbps),
				make_readable(network_stats.acked_kbps),
				network_stats.packets_sent,
				network_stats.packets_received,
				network_stats.packets_acked
			);

			total_details += { stats_summary, text_style };
		}

		if (server_stats.are_set()) {
			total_details += { "Server stats\n", category_style };

			const auto stats_summary = typesafe_sprintf(
				"Sent: %x/s" "\n"
				"Rcvd: %x/s" "\n",
				make_readable(server_stats.sent_kbps),
				make_readable(server_stats.received_kbps)
			);

			total_details += { stats_summary, text_style };
		}

		print(
			output, 
			{ screen_size.x - log_line_width - net_line_width, 0 }, 
			total_details, 
			net_line_width
		);
	}

	{
		total_details.clear();

		total_details += { "Cosmos\n", category_style };

		if (false && viewed_character.alive()) {
			total_details += { viewed_character.get_cosmos().profiler.summary(), text_style };
		}

		print(
			output, 
			{ screen_size.x - log_line_width - net_line_width - cosm_line_width, 0 }, 
			total_details, 
			cosm_line_width
		);
	}

}
