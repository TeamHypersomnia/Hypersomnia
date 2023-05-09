#pragma once
#include "augs/math/declare_math.h"
#include "game/cosmos/entity_handle_declaration.h"

struct session_profiler;
struct cosmic_profiler;
struct audiovisual_profiler;
struct frame_profiler;
struct atlas_profiler;
struct network_profiler;
struct network_info;
struct server_network_info;

namespace augs {
	struct drawer;
	struct baked_font;
}

struct debug_details_summaries {
	std::string frame;
	std::string network;
	std::string network_stats;
	std::string server_stats;
	std::string streaming;
	std::string general_atlas;
	std::string session;
	std::string audiovisual;
	std::string cosmic;

	void acquire(
		const cosmos&,
		const frame_profiler& frame_performance,
		const network_profiler& network_performance,
		const network_info& network_stats,
		const server_network_info& server_stats,
		const viewables_streaming_profiler& streaming_performance,
		const atlas_profiler& general_atlas_performance,
		const session_profiler& session_performance,
		const audiovisual_profiler& audiovisual_performance
	);
};

void show_performance_details(
	const augs::drawer output,
	const augs::baked_font& gui_font,
	const vec2i screen_size,
	const const_entity_handle viewed_character,
	const debug_details_summaries&
);

void show_recent_logs(
	const augs::drawer output,
	const augs::baked_font& gui_font,
	const vec2i screen_size
);