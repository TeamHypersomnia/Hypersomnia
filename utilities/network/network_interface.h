#pragma once

#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"

#include "PacketPriority.h"

namespace augs {
	namespace network {
		struct network_interface {
			struct packet {
				RakNet::Packet* info = nullptr;
				RakNet::RakPeerInterface* owner = nullptr;
				RakNet::BitStream result_bitstream;

				unsigned char byte(int) const;
				unsigned length() const;
				RakNet::RakNetGUID guid() const;

				RakNet::BitStream& get_bitstream();
				void destroy();
				~packet();
			};

			RakNet::RakPeerInterface* peer;

			network_interface();
			~network_interface();
			
			int listen(unsigned short port, unsigned max_players, unsigned max_connections);
			int connect(const char* ip_address, unsigned port);

			bool receive(packet& output);

			unsigned send(RakNet::BitStream&, int priority, int reliability, int channel, RakNet::RakNetGUID target, bool broadcast);

			network_interface(const network_interface&) = delete;
			network_interface& operator=(const network_interface&) = delete;

		};

	}
}

namespace std {
	template <>
	struct hash<RakNet::RakNetGUID> {
		std::size_t operator()(const RakNet::RakNetGUID& k) const {
			return RakNet::RakNetGUID::ToUint32(k);
		}
	};

}