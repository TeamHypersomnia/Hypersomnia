#include "augs/gui/text/printer.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

#include "view/frame_profiler.h"
#include "view/audiovisual_state/audiovisual_profiler.h"
#include "view/viewables/streaming/viewables_streaming_profiler.h"

#include "application/session_profiler.h"
#include "application/main/draw_debug_details.h"
#include "augs/network/network_types.h"

#include "augs/templates/introspect.h"

#include "build_info.h"

void debug_details_summaries::acquire(
	const cosmos& cosm,
	const frame_profiler& frame_performance,
	const network_profiler& network_performance,
	const network_info& network_stats,
	const server_network_info& server_stats,
	const viewables_streaming_profiler& streaming_performance,
	const atlas_profiler& general_atlas_performance,
	const session_profiler& session_performance,
	const audiovisual_profiler& audiovisual_performance
) {
	frame_performance.summary(frame);
	network_performance.summary(network);
	streaming_performance.summary(streaming);
	general_atlas_performance.summary(general_atlas);
	session_performance.summary(session);
	audiovisual_performance.summary(audiovisual);

	if (cosm.completely_unset()) {
		cosmic.clear();
	}
	else {
		cosm.profiler.summary(cosmic);
	}

	auto make_readable = [&](const auto kbits) {
		const bool in_bytes = false;

		if (in_bytes) {
			return readable_bytesize(kbits * 1000 / 8);
		}

		return readable_bitsize(kbits * 1000);
	};

	if (server_stats.are_set()) {
		this->server_stats = typesafe_sprintf(
			"Sent: %x/s" "\n"
			"Rcvd: %x/s" "\n",
			make_readable(server_stats.sent_kbps),
			make_readable(server_stats.received_kbps)
		);
	}

	if (network_stats.are_set()) {
		const auto loss_percent = 
			typesafe_sprintf("Loss: %2f", network_stats.loss_percent) + "%" "\n"
		;

		this->network_stats = loss_percent + typesafe_sprintf(
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
	}
}

void show_recent_logs(
	const augs::drawer output,
	const augs::baked_font& gui_font,
	const vec2i screen_size
) {
	(void)screen_size;
	const auto max_lines = 30;

	print(
		output, 
		vec2i(9, 0),
		augs::gui::text::format_recent_program_log(gui_font, max_lines).mult_alpha(150.f / 255)
	);
}

void show_performance_details(
	const augs::drawer output,
	const augs::baked_font& gui_font,
	const vec2i screen_size,
	const const_entity_handle viewed_character,
	const debug_details_summaries& summaries
) {
	using namespace augs::gui::text;

	const auto av_line_width = 500;
	const auto net_line_width = 400;
	const auto cosm_line_width = 500;

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

	if (viewed_character.alive()) {
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
	total_details += { summaries.session, text_style };
	total_details += { "Frame\n", category_style };
	total_details += { summaries.frame, text_style };

	print(output, { 0, 0 }, total_details);

	{
		total_details.clear();

		total_details += { "Cosmos\n", category_style };
		total_details += { summaries.cosmic, text_style };

		print(
			output, 
			{ screen_size.x - net_line_width - cosm_line_width - av_line_width, 0 }, 
			total_details, 
			cosm_line_width
		);
	}

	{
		total_details.clear();

		total_details += { "Network\n", category_style };
		total_details += { summaries.network, text_style };

		if (summaries.network_stats.size() > 0) {
			total_details += { "Connection stats\n", category_style };
			total_details += { summaries.network_stats, text_style };
		}

		if (summaries.server_stats.size() > 0) {
			total_details += { "Server stats\n", category_style };
			total_details += { summaries.server_stats, text_style };
		}

		print(
			output, 
			{ screen_size.x - net_line_width - av_line_width, 0 }, 
			total_details, 
			net_line_width
		);
	}

	{
		total_details.clear();

		total_details += { "Audiovisual\n", category_style };
		total_details += { summaries.audiovisual, text_style };

		total_details += { "Viewables streaming\n", category_style };
		total_details += { summaries.streaming, text_style };

		total_details += { "General atlas\n", category_style };
		total_details += { summaries.general_atlas, text_style };

		print(
			output, 
			{ screen_size.x - av_line_width, 0 }, 
			total_details, 
			av_line_width
		);
	}

}
