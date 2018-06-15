#if BUILD_ENET
#include <enet/enet.h>
#undef min
#undef max
#endif

#include "network_server.h"
#include "augs/readwrite/byte_readwrite.h"

namespace augs {
	namespace network {
		bool server::listen(const unsigned short port, const unsigned max_connections) {
#if BUILD_ENET
			ENetAddress addr;

			addr.host = ENET_HOST_ANY;
			addr.port = port;

			return host.init(&addr /* the address to bind the server host to */,
				max_connections      /* allow up to 32 clients and/or outgoing connections */,
				2      /* allow up to 2 channels to be used, 0 and 1 */,
				0      /* assume any amount of incoming bandwidth */,
				0);
#else
			(void)port;
			(void)max_connections;
			LOG("Warning! ENet wasn't built.");
			return false;
#endif
		}

		bool server::post_redundant(const packet& payload, const endpoint_address& target) {
			packet stream = payload;
			return peer_map.at(target).redundancy.sender.post_message(stream);
		}

		bool server::has_endpoint(const endpoint_address& target) const {
			return peer_map.find(target) != peer_map.end();
		}

		bool server::has_timed_out(const endpoint_address& target, const float sequence_interval_ms, const float ms) const {
			return has_endpoint(target) && peer_map.at(target).redundancy.timed_out(static_cast<size_t>(ms / sequence_interval_ms));
		}

		bool server::send_pending_redundant() {
#if BUILD_ENET
			bool result = true;

			for (auto& e : peer_map) {
				augs::memory_stream payload;
				
				e.second.redundancy.build_next_packet(payload);

				ENetPacket * const packet = enet_packet_create(payload.data(), payload.size(), ENET_PACKET_FLAG_UNSEQUENCED);
				result = result && !enet_peer_send(e.second, 0, packet);
				enet_host_flush(host.get());
			}
			return result;
#endif
			return false;
		}

		bool server::send_reliable(const packet& payload, const endpoint_address& target) {
#if BUILD_ENET
			ENetPacket * const packet = enet_packet_create(payload.data(), payload.size(), ENET_PACKET_FLAG_RELIABLE);
			auto result = !enet_peer_send(peer_map.at(target), 1, packet);
			enet_host_flush(host.get());

			return result;
#else
			(void)payload;
			(void)target;
#endif
			return false;
		}

		void server::disconnect(const endpoint_address& target) {
#if BUILD_ENET
			enet_peer_disconnect(peer_map.at(target), 0);
#else
			(void)target;
#endif
		}

		void server::forceful_disconnect(const endpoint_address& target) {
#if BUILD_ENET
			enet_peer_reset(peer_map.at(target));
#else
			(void)target;
#endif
			peer_map.erase(target);
		}

		std::vector<message> server::collect_entropy() {
			std::vector<message> total;

#if BUILD_ENET
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
					LOG("SPort: %x", event.peer->address.port);
					peer_map[new_event.address] = event.peer;

					break;
				case ENET_EVENT_TYPE_RECEIVE:
					new_event.payload.reserve(event.packet->dataLength);
					new_event.payload.write(reinterpret_cast<const std::byte*>(event.packet->data), event.packet->dataLength);
					new_event.message_type = message::type::RECEIVE;
					new_event.address = event.peer->address;

					if (event.channelID == 0) {
						auto result = peer_map.at(new_event.address).redundancy.handle_incoming_packet(new_event.payload);

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

					forceful_disconnect(new_event.address);
					break;
					default: break;
				}
				
				if(add_event)
					total.emplace_back(new_event);
			}
#endif
			return total;
		}
	}
}