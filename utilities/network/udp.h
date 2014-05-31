#pragma once
#include "../threads/threads.h"
#include "network.h"

namespace augs  {
	namespace network {
		class udp {
			friend class augs::threads::iocp;
			SOCKET sock;
			ip addr;

			char buffer_for_receives[66000];
		public:
			udp(); ~udp();
			bool open();
			bool bind(unsigned short port, char* ipv4 = ip::get_local_ipv4());

			void set_blocking(bool);

			/* raw calls, return io_result */
			int send(const ip& to, const wsabuf& input_packet, unsigned long& result);
			int recv(ip& from, wsabuf& b, unsigned long& result);

			/* abstracted calls, 0 - error, 1 - successful */
			bool send(const ip& to, const packet& input_packet, unsigned long& result);
			bool recv(ip& from, const packet& output_packet, unsigned long& result);

			bool get_result(overlapped&) const;
			bool close();
		};
	}
}