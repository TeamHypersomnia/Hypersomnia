#pragma once
#include "augs/readwrite/memory_stream.h"
#include "augs/templates/hash_templates.h"
#include "augs/readwrite/byte_readwrite_declaration.h"

namespace augs {
	namespace network {
		using packet = std::vector<std::byte>;

		class endpoint_address {
			unsigned ip = 0;
			unsigned short port = 0;

		public:
			endpoint_address() = default;

			bool operator==(const endpoint_address& b) const;

			std::string get_readable() const;
			std::string get_readable_ip() const;

			unsigned short get_port() const;
		};

	}

	struct network_message {
		enum class type {
			CONNECT,
			DISCONNECT,
			PAYLOAD
		};

		// GEN INTROSPECTOR struct augs::network_message
		type message_type = type::CONNECT;
		network::endpoint_address address;
		std::vector<std::byte> payload;
		unsigned messages_to_skip = 0;
		// END GEN INTROSPECTOR
	};

	namespace network {
		using message = network_message;
	}
}

namespace std {
	template <>
	struct hash<augs::network::endpoint_address> {
		std::size_t operator()(const augs::network::endpoint_address& k) const {
			return std::hash<std::string>()(k.get_readable());
		}
	};
}