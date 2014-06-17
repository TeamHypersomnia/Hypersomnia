#include "network_interface.h"

namespace augs {
	namespace network {
		network_interface::network_interface() {
			peer = RakNet::RakPeerInterface::GetInstance();
		}

		int network_interface::listen(unsigned short port, unsigned max_players, unsigned max_connections) {
			RakNet::SocketDescriptor sd(port, 0);
			auto result = peer->Startup(max_connections, &sd, 1);
			peer->SetMaximumIncomingConnections(max_players);

			return result;
		}

		int network_interface::connect(const char* ip_address, unsigned port) {
			peer->Startup(1, &RakNet::SocketDescriptor(), 1);
			auto result = peer->Connect(ip_address, port, 0, 0);
			
			return result;
		}
		
		RakNet::BitStream& network_interface::packet::get_bitstream() {
			return result_bitstream;
		}

		void network_interface::packet::destroy() {
			if (owner && info)
				owner->DeallocatePacket(info);
			owner = nullptr;
			info = nullptr;
		}

		unsigned char network_interface::packet::byte(int i) const {
			return info->data[i];
		}

		unsigned network_interface::packet::length() const {
			return info->length;
		}

		RakNet::RakNetGUID network_interface::packet::guid() const {
			return info->guid;
		}

		bool network_interface::receive(packet& output) {
			output.destroy();

			output.info = peer->Receive();

			if (output.info) {
				output.owner = peer;

				RakNet::BitStream my_stream(output.info->data, output.info->length, false);

				/* we're not allocating memory in bitstream so it is safe */
				memcpy(&output.result_bitstream, &my_stream, sizeof(RakNet::BitStream));

				return true;
			}

			return false;
		}

		void network_interface::close_connection(const RakNet::RakNetGUID& guid, int disconnection_notification_priority) {
			peer->CloseConnection(guid, true, 0, (PacketPriority) disconnection_notification_priority);
		}

		void network_interface::shutdown(unsigned block_duration, int disconnection_notification_priority) {
			peer->Shutdown(block_duration, 0, (PacketPriority) disconnection_notification_priority);
		}

		unsigned network_interface::send(RakNet::BitStream& bitstream, int priority, int reliability, int channel, RakNet::RakNetGUID target, bool broadcast) {
			return peer->Send(&bitstream, (PacketPriority) priority, (PacketReliability) reliability, channel, target, broadcast);
		}

		network_interface::packet::~packet() {
			destroy();
		}

		network_interface::~network_interface() {
			RakNet::RakPeerInterface::DestroyInstance(peer);
		}
	}
}