#pragma once
#include "augs/misc/randomization.h"
#include "augs/network/netcode_socket_includes.h"
#include "application/nat/stun_request_structs.h"

#define XOR_MAPPED_ADDRESS_TYPE 0x0020

// RFC 5389 Section 15 STUN Attributes
struct STUNAttributeHeader
{
    // Attibute Type
    uint16_t type;
    
    // Payload length of this attribute
    uint16_t length;
};

#define IPv4_ADDRESS_FAMILY 0x01;
#define IPv6_ADDRESS_FAMILY 0x02;

// RFC 5389 Section 15.2 XOR-MAPPED-ADDRESS
struct STUNXORMappedIPv4Address
{
    uint8_t reserved;
    uint8_t family;
    uint16_t port;
    uint32_t address;
};

auto make_stun_request(randomization& rng) {
	STUNMessageHeader request;
	request.type = htons(0x0001);
	request.length = htons(0x0000);
	request.cookie = htonl(0x2112A442);

	for (auto& id : request.identifier) {
		id = rng.randval(uint32_t(0), std::numeric_limits<uint32_t>::max() - 1);
	}

	return request;
}

inline std::optional<netcode_address_t> read_stun_response(
	const STUNMessageHeader& source_request,
	const std::byte* packet_buffer,
	const std::size_t packet_bytes
) {
	try {
		auto buf = augs::cpointer_to_buffer { packet_buffer, packet_bytes };
		auto cptr = augs::cptr_memory_stream(buf);

		const auto response = augs::read_bytes<STUNMessageHeader>(cptr);

		if (response.type != htons(0x0101)) {
			return std::nullopt;
		}

		if (!source_request.identifiers_match(response)) {
			return std::nullopt;
		}

		while (cptr.has_unread_bytes()) {
			const auto header = augs::read_bytes<STUNAttributeHeader>(cptr);

            if (header.type == htons(XOR_MAPPED_ADDRESS_TYPE)) {
				const auto xorAddress = augs::read_bytes<STUNXORMappedIPv4Address>(cptr);

				const uint32_t address = htonl(xorAddress.address) ^ 0x2112A442;
				const uint16_t port = ntohs(xorAddress.port) ^ 0x2112;

				netcode_address_t result;
				result.type = NETCODE_ADDRESS_IPV4;
				result.port = port;

				std::memcpy(&result.data.ipv4, &address, sizeof(address));

				return result;
			}
		}
	}
	catch (const augs::stream_read_error& err) {

	}

	return std::nullopt;
}
