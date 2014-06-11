#pragma once

#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"

namespace augs {
	namespace network {
		struct network_interface {
			struct packet {
				RakNet::Packet* info = nullptr;
				RakNet::RakPeerInterface* owner = nullptr;

				unsigned char byte(int) const;
				unsigned length() const;
				unsigned long guid() const;

				void destroy();
				~packet();
			};

			RakNet::RakPeerInterface* peer;

			network_interface();
			~network_interface();
			
			int listen(unsigned short port, unsigned max_players, unsigned max_connections);
			int connect(const char* ip_address, unsigned port);

			bool receive(packet& output);

			network_interface(const network_interface&) = delete;
			network_interface& operator=(const network_interface&) = delete;

		};

	}
}