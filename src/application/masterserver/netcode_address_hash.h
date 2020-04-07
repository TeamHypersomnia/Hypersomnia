#pragma once
#include <unordered_map>
#include "augs/templates/hash_templates.h"
#include "3rdparty/yojimbo/netcode.io/netcode.h"

namespace std {
	template <>
	struct hash<netcode_address_t> {
		size_t operator()(const netcode_address_t& k) const {
			if (k.type == NETCODE_ADDRESS_IPV4) {
				uint32_t ip;
				std::memcpy(&ip, &k.data.ipv4, sizeof(ip));
				return augs::hash_multiple(ip, k.port);
			}

			uint64_t ip_parts[2];
			std::memcpy(&ip_parts, &k.data.ipv6, sizeof(ip_parts));
			return augs::hash_multiple(ip_parts[0], ip_parts[1], k.port);
		}
	};
}
