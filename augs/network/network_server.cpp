#include <enet/enet.h>
#undef min
#undef max

#include "network_server.h"
#include "augs/misc/templated_readwrite.h"

namespace augs {
	namespace network {
		bool server::listen(unsigned short port, unsigned max_connections) {
			ENetAddress addr;

			addr.host = ENET_HOST_ANY;
			addr.port = port;

			return host.init(&addr /* the address to bind the server host to */,
				max_connections      /* allow up to 32 clients and/or outgoing connections */,
				2      /* allow up to 2 channels to be used, 0 and 1 */,
				0      /* assume any amount of incoming bandwidth */,
				0);
		}

		bool server::post_redundant(const packet& payload, const endpoint_address& target) {
			packet stream = payload;
			return peer_map[target].redundancy.sender.post_message(stream);
		}

		bool server::has_endpoint(const endpoint_address& target) const {
			return peer_map.find(target) != peer_map.end();
		}

		bool server::send_pending_redundant() {
			bool result = true;

			for (auto& e : peer_map) {
				augs::stream payload;
				
				e.second.redundancy.build_next_packet(payload);

				ENetPacket * const packet = enet_packet_create(payload.data(), payload.size(), ENET_PACKET_FLAG_UNSEQUENCED);
				result = result && !enet_peer_send(e.second, 0, packet);
				enet_host_flush(host.get());
			}

			return result;
		}

		bool server::send_reliable(const packet& payload, const endpoint_address& target) {
			ENetPacket * const packet = enet_packet_create(payload.data(), payload.size(), ENET_PACKET_FLAG_RELIABLE);
			auto result = !enet_peer_send(peer_map[target], 1, packet);
			enet_host_flush(host.get());

			return result;
		}

		std::vector<message> server::collect_entropy() {
			std::vector<message> total;

			if (host.get() == nullptr)
				return total;

			ENetEvent event;

			while (enet_host_service(host.get(), &event, 0) > 0) {
				message new_event;
				bool add_event = true;

				switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
					new_event.message_type = message::type::CONNECT;
					new_event.address = event.peer->address;
					
					peer_map[new_event.address] = event.peer;

					break;
				case ENET_EVENT_TYPE_RECEIVE:
					new_event.payload.reserve(event.packet->dataLength);
					augs::write_bytes(new_event.payload, event.packet->data, event.packet->dataLength);
					new_event.message_type = message::type::RECEIVE;
					new_event.address = event.peer->address;

					if (event.channelID == 0) {
						auto result = peer_map[new_event.address].redundancy.handle_incoming_packet(new_event.payload);

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

					peer_map.erase(new_event.address);
				}
				
				if(add_event)
					total.emplace_back(new_event);
			}

			return std::move(total);
		}
	}
}