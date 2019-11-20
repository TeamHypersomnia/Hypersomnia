#pragma once
#include "augs/templates/filter_types.h"
#include "augs/readwrite/memory_stream_declaration.h"
#include "augs/templates/snapshotted_player.h"

template <class T>
struct is_server_to_client : std::bool_constant<T::server_to_client> {};

using server_message_variant = replace_list_type_t<
	filter_types_in_list_t<
		is_server_to_client, 
		transform_types_in_list_t<net_messages::all_t, std::remove_pointer_t>
	>,
	std::variant
>;

struct demo_step {
	// GEN INTROSPECTOR struct demo_step
	std::vector<std::vector<std::byte>> serialized_messages;
	// END GEN INTROSPECTOR
};

using client_setup_snapshot = std::vector<std::byte>;

using client_setup_player_base = augs::snapshotted_player<
	client_setup_snapshot,
	demo_step
>;
