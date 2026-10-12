#pragma once
#include "3rdparty/yojimbo/netcode/netcode.h"

namespace augs {
	/*
		The network an address belongs to, for limits and bans per address: an IPv4 address as it is,
		an IPv6 address as its /64 (one home usually holds a whole /64 and may switch addresses within it),
		an IPv4-mapped IPv6 address as the IPv4 address. The port is dropped.
	*/

	inline netcode_address_t network_of(netcode_address_t a) {
		a.port = 0;

		if (a.type == NETCODE_ADDRESS_IPV6) {
			const auto& g = a.data.ipv6;
			const bool ipv4_mapped = g[0] == 0 && g[1] == 0 && g[2] == 0 && g[3] == 0 && g[4] == 0 && g[5] == 0xffff;

			if (ipv4_mapped) {
				auto v4 = netcode_address_t();
				v4.type = NETCODE_ADDRESS_IPV4;
				v4.data.ipv4[0] = static_cast<uint8_t>(g[6] >> 8);
				v4.data.ipv4[1] = static_cast<uint8_t>(g[6] & 0xff);
				v4.data.ipv4[2] = static_cast<uint8_t>(g[7] >> 8);
				v4.data.ipv4[3] = static_cast<uint8_t>(g[7] & 0xff);
				return v4;
			}

			for (int i = 4; i < 8; ++i) {
				a.data.ipv6[i] = 0;
			}
		}

		return a;
	}
}
