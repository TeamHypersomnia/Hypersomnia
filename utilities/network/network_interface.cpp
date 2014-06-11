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
			output.owner = peer;

			return output.info != nullptr;
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