#pragma once
#include "augs/misc/constant_size_vector.h"

using message_bytes_type = augs::constant_size_vector<std::byte, max_message_size_v>;

template <class F>
bool net_read(
	const message_bytes_type&, 
	mode_entropy&, 
	F&& client_id_to_entity_id
) {
	(void)client_id_to_entity_id;
	return true;
}

template <class F>
bool net_write(
	message_bytes_type&, 
	const mode_entropy&,
	F&& entity_id_to_client_id
) {
	(void)entity_id_to_client_id;
	return true;
}

bool net_read(const message_bytes_type&, mode_player_entropy&);
bool net_write(message_bytes_type&, const mode_player_entropy&);
