#pragma once

struct netcode_queued_packet {
	netcode_address_t to;
	std::vector<std::byte> bytes;
	std::optional<int> ttl;

	template <class V>
	netcode_queued_packet(
		const netcode_address_t& to,
		V&& bytes, 
		std::optional<int> ttl = std::nullopt
	) : to(to), bytes(std::forward<V>(bytes)), ttl(ttl) {}
};
