#pragma once
#include <array>

// RFC 5389 Section 6 STUN Message Structure
struct STUNMessageHeader {
	// Message Type (Binding Request / Response)
	uint16_t type;

	// Payload length of this message
	uint16_t length;

	// Magic Cookie
	uint32_t cookie;

	// Unique Transaction ID
	std::array<uint32_t, 3> identifier;

	bool identifiers_match(const STUNMessageHeader& b) const {
		return identifier == b.identifier;
	}
};
