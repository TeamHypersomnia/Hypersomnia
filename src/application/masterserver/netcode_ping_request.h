#pragma once

inline auto make_ping_request_message_bytes(const uint64_t sequence) {
	constexpr auto request_size = sizeof(uint8_t) + sizeof(uint64_t);
	std::array<std::byte, request_size> bytes;

	{
		auto buf = augs::pointer_to_buffer { bytes.data(), request_size }; 
		auto out = augs::ptr_memory_stream { buf };

		augs::write_bytes(out, uint8_t(NETCODE_PING_REQUEST_PACKET));
		augs::write_bytes(out, sequence);
	}

	return bytes;
}

inline auto make_ping_request_message_bytes(const netcode_address_t& pingback_address) {
	constexpr auto request_size = sizeof(uint8_t) + sizeof(uint64_t) + sizeof(netcode_address_t);
	std::array<std::byte, request_size> bytes;

	{
		auto buf = augs::pointer_to_buffer { bytes.data(), request_size };
		auto out = augs::ptr_memory_stream(buf);

		const auto sequence_dummy = uint64_t(-1);

		augs::write_bytes(out, uint8_t(NETCODE_PING_REQUEST_PACKET));
		augs::write_bytes(out, sequence_dummy);
		augs::write_bytes(out, pingback_address);
	}

	return bytes;
}

