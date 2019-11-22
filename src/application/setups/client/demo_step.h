#pragma once
#include "augs/templates/filter_types.h"
#include "augs/readwrite/memory_stream_declaration.h"
#include "application/network/network_messages.h"
#include "game/modes/mode_entropy.h"

template <class T>
struct is_server_to_client : std::bool_constant<T::server_to_client> {};

using server_message_variant = transform_types_in_list_t<
	filter_types_in_list_t<
		is_server_to_client, 
		transform_types_in_list_t<net_messages::all_t, std::remove_pointer_t>
	>, 
	std::add_pointer_t
>;

struct demo_step {
	// GEN INTROSPECTOR struct demo_step
	std::optional<mode_entropy> local_entropy;
	std::vector<std::vector<std::byte>> serialized_messages;
	// END GEN INTROSPECTOR
};

using client_setup_snapshot = std::vector<std::byte>;
