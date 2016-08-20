#pragma once
#include "augs/misc/streams.h"

namespace augs {
	namespace network {
		struct network_interface {
			struct packet {
				RakNet::Packet* info = nullptr;
				RakNet::RakPeerInterface* owner = nullptr;
				augs::stream result_bitstream;

				unsigned char byte(int) const;
				unsigned length() const;
				RakNet::RakNetGUID guid() const;

				augs::stream& get_bitstream();
				void destroy();
				~packet();
			};

			RakNet::RakPeerInterface* peer;

			network_interface();
			~network_interface();
			
			int listen(unsigned short port, unsigned max_players, unsigned max_connections);
			int connect(const char* ip_address, unsigned port);

			bool receive(packet& output);

			void enable_lag(float loss, unsigned short latency, unsigned short jitter);

			void close_connection(const RakNet::RakNetGUID&, int disconnection_notification_priority);
			void shutdown(unsigned block_duration, int disconnection_notification_priority);

			void occasional_ping(bool);
			int get_last_ping(const RakNet::RakNetGUID&);
			int get_average_ping(const RakNet::RakNetGUID&);

			void set_timeout(unsigned ms, const RakNet::SystemAddress&);
			void set_timeout_all(unsigned ms);

			unsigned send(augs::stream&, int priority, int reliability, int channel, RakNet::RakNetGUID target, bool broadcast);

			network_interface(const network_interface&) = delete;
			network_interface& operator=(const network_interface&) = delete;

		};
	}
}