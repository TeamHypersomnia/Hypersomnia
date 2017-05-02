#pragma once
#include "augs/misc/streams.h"
#include "augs/templates/hash_templates.h"

struct _ENetAddress;
typedef struct _ENetAddress ENetAddress;

namespace augs {
	namespace network {
		typedef augs::stream packet;

		class endpoint_address {
			unsigned ip;
			unsigned short port;
		public:
			endpoint_address();
			endpoint_address(const ENetAddress& addr);

			bool operator==(endpoint_address b) const {
				return ip == b.ip && port == b.port;
			}

			std::string get_readable_ip() const;
			unsigned get_ip() const;
			unsigned short get_port() const;
		};

		struct message {
			enum class type {
				INVALID,
				CONNECT,
				DISCONNECT,
				RECEIVE
			};

			type message_type = type::INVALID;
			endpoint_address address;
			packet payload;
			unsigned messages_to_skip = 0;

			bool message::operator==(const message& b) const {
				return
					message_type == b.message_type
					&& address == b.address
					&& messages_to_skip == b.messages_to_skip
				;
			}
		};
	}


	template<class A>
	void read_object(A& ar, network::message& s) {
		read(ar, s.message_type);
		read(ar, s.address);
		read_stream_with_properties(ar, s.payload);
		read(ar, s.messages_to_skip);
	}

	template<class A>
	void write_object(A& ar, const network::message& s) {
		write(ar, s.message_type);
		write(ar, s.address);
		write_stream_with_properties(ar, s.payload);
		write(ar, s.messages_to_skip);
	}
}

namespace std {
	template <>
	struct hash<augs::network::endpoint_address> {
		std::size_t operator()(const augs::network::endpoint_address& k) const {
			return augs::simple_two_hash(k.get_ip(), k.get_port());
		}
	};
}