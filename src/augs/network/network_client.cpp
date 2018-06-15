#if BUILD_ENET
#include <enet/enet.h>
#undef min
#undef max
#endif

#include "augs/misc/readable_bytesize.h"
#include "augs/network/network_client.h"

namespace augs {
	namespace network {
		bool client::connect(const std::string host_address, const unsigned short port, const unsigned timeout_ms) {
#if BUILD_ENET
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
#else
			LOG("Warning! ENet wasn't built.");
			(void)host_address;
			(void)port;
			(void)timeout_ms;
			return false;
#endif
		}

		bool client::post_redundant(const packet& payload) {
#if BUILD_ENET
			if (peer == nullptr || host.get() == nullptr)
				return false;

			packet stream = payload;
			return redundancy.sender.post_message(stream);
#else
			(void)payload;
			return false;
#endif
		}

		bool client::send_pending_redundant() {
#if BUILD_ENET
			if (peer == nullptr || host.get() == nullptr)
				return false;

			augs::memory_stream payload;

			redundancy.build_next_packet(payload);

			ENetPacket * const packet = enet_packet_create(payload.data(), payload.size(), ENET_PACKET_FLAG_UNSEQUENCED);
			const auto result = !enet_peer_send(peer, 0, packet);
			sent_size.measure(payload.size());
			enet_host_flush(host.get());

			return result;
#else
			return false;
#endif
		}

		bool client::send_reliable(const packet& payload) {
#if BUILD_ENET
			if (peer == nullptr || host.get() == nullptr)
				return false;

			ENetPacket * const packet = enet_packet_create(payload.data(), payload.size(), ENET_PACKET_FLAG_RELIABLE);
			const auto result = !enet_peer_send(peer, 1, packet);
			sent_size.measure(payload.size());
			enet_host_flush(host.get());

			return result;
#else
			(void)payload;
			return false;
#endif
		}

		bool client::has_timed_out(const float sequence_interval_ms, const float ms) const {
			return peer != nullptr && host.get() != nullptr && redundancy.timed_out(static_cast<size_t>(ms / sequence_interval_ms));
		}

		unsigned client::total_bytes_received() const {
#if BUILD_ENET
			return host.get() == nullptr ? 0 : host.get()->totalReceivedData;
#else
			return 0;
#endif
		}

		unsigned client::total_bytes_sent() const {
#if BUILD_ENET
			return host.get() == nullptr ? 0 : host.get()->totalSentData;
#else
			return 0;
#endif
		}

		unsigned client::total_packets_sent() const {
#if BUILD_ENET
			return host.get() == nullptr ? 0 : host.get()->totalSentPackets;
#else
			return 0;
#endif
		}

		unsigned client::total_packets_received() const {
#if BUILD_ENET
			return host.get() == nullptr ? 0 : host.get()->totalReceivedPackets;
#else
			return 0;
#endif
		}

		std::string client::format_transmission_details() const {
			return typesafe_sprintf("RTT:\nSent: %x (%x)\nReceived: %x (%x)\nLast sent: %x\nLast received: %x",
				total_packets_sent(), readable_bytesize(total_bytes_sent()), total_packets_received(), readable_bytesize(total_bytes_received()),
				readable_bytesize(static_cast<unsigned>(sent_size.get_average_units())),
				readable_bytesize(static_cast<unsigned>(recv_size.get_average_units()))
			);
		}

		void client::disconnect() {
#if BUILD_ENET
			enet_peer_disconnect(peer, 0);
#endif
		}

		void client::forceful_disconnect() {
#if BUILD_ENET
			enet_peer_reset(peer);
#endif
		}

		std::vector<message> client::collect_entropy() {
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

					break;
				case ENET_EVENT_TYPE_RECEIVE:
					new_event.payload.reserve(event.packet->dataLength);
					recv_size.measure(event.packet->dataLength);
					new_event.payload.write(reinterpret_cast<const std::byte*>(event.packet->data), event.packet->dataLength);
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