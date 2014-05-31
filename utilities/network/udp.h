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

			struct send_result {
				int result; /* is io_operation enum */
				unsigned long bytes_transferred;
				send_result();
			};

			struct recv_result : send_result {
				ip sender_address;
				packet message;
			};

			/* more abstract calls */
			send_result send(const ip& to, packet& input_packet);

			/* WARNING!!! This should not be called from two or more threads on the same socket - I'm using buffer_for_receives member to
				avoid unnecessary allocations/deallocations
			*/

			recv_result recv();

			bool get_result(overlapped&) const;
			bool close();
		};
	}
}