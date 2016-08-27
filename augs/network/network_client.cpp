#include <enet/enet.h>
#undef min
#undef max

#include "augs/misc/templated_readwrite.h"
#include "network_client.h"

namespace augs {
	namespace network {
		bool client::connect(const std::string host_address, const unsigned short port, const unsigned timeout_ms) {
			host.init(nullptr /* create a client host */,
				1 /* only allow 1 outgoing connection */,
				2 /* allow up 2 channels to be used, 0 and 1 */,
				0,
				0);

			ENetEvent event;
			ENetAddress addr;

			/* Connect to some.server.net:1234. */
			enet_address_set_host(&addr, host_address.c_str());
			addr.port = port;

			this->address = addr;

			/* Initiate the connection, allocating the two channels 0 and 1. */
			peer = enet_host_connect(host.get(), &addr, 2, 0);
			if (peer == nullptr) {
				LOG("No available peers for initiating an ENet connection.\n");
				return false;
			}

			/* Wait up to timeout for the connection attempt to succeed. */
			if (enet_host_service(host.get(), &event, timeout_ms) > 0 &&
				event.type == ENET_EVENT_TYPE_CONNECT) {
				LOG("Connection to %x:%x succeeded.", host_address, port);
				return true;
			}
			else {
				/* Either the 5 seconds are up or a disconnect event was */
				/* received. Reset the peer in the event the 5 seconds   */
				/* had run out without any significant event.            */
				enet_peer_reset(peer);
				LOG("Connection to %x:%x failed.", host_address, port);
				return false;
			}
		}

		bool client::post_redundant(const packet& payload) {
			packet stream = payload;
			redundancy.sender.post_message(stream);
			return true;
		}

		bool client::send_pending_redundant() {
			augs::stream payload;

			redundancy.build_next_packet(payload);

			ENetPacket * const packet = enet_packet_create(payload.data(), payload.size(), ENET_PACKET_FLAG_UNSEQUENCED);
			const auto result = !enet_peer_send(peer, 0, packet);
			enet_host_flush(host.get());

			return result;
		}

		bool client::send_reliable(const packet& payload) {
			ENetPacket * const packet = enet_packet_create(payload.data(), payload.size(), ENET_PACKET_FLAG_RELIABLE);
			const auto result = !enet_peer_send(peer, 1, packet);
			enet_host_flush(host.get());

			return result;
		}

		std::vector<message> client::collect_entropy() {
			std::vector<message> total;

			ENetEvent event;

			while (enet_host_service(host.get(), &event, 0) > 0) {
				message new_event;
				bool add_event = true;

				switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
					new_event.message_type = message::type::CONNECT;
					new_event.address = event.peer->address;

					break;
				case ENET_EVENT_TYPE_RECEIVE:
					new_event.payload.reserve(event.packet->dataLength);
					augs::write_bytes(new_event.payload, event.packet->data, event.packet->dataLength);
					new_event.message_type = message::type::RECEIVE;
					new_event.address = event.peer->address;

					if (event.channelID == 0) {
						auto result = redundancy.handle_incoming_packet(new_event.payload);

						if (result.result_type == result.NOTHING_RECEIVED) {
							add_event = false;
						}
						else {
							new_event.messages_to_skip = result.messages_to_skip;
						}
					}

					enet_packet_destroy(event.packet);

					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					new_event.message_type = message::type::DISCONNECT;
					new_event.address = event.peer->address;
				}

				if(add_event)
					total.emplace_back(new_event);
			}

			return std::move(total);
		}
	}
}