#pragma once
#include "../threads/threads.h"
#include "network.h"

namespace augs  {
	namespace network {
		class udp {
			friend class augs::threads::iocp;
			SOCKET sock;
			ip addr;
		public:
			udp(); ~udp();
			bool open();
			bool bind(unsigned short port, char* ipv4 = ip::get_local_ipv4());

			void set_blocking(bool);

			// 0 - error, 1 - pending, 2 - completed
			int send(overlapped* request);
			//int send(const ip& to, const buf* bufs, int bufcnt, overlapped* request);
			int recv(overlapped* request);
			//int recv(ip& from, buf* bufs, int bufcnt, overlapped* request);

			// 0 - error, 1 - successful
			bool send(const ip& to, const wsabuf& b, unsigned long& result);
			//bool send(const ip& to, const buf* bufs, int bufcnt, unsigned long& result, unsigned long flags);
			bool recv(ip& from, wsabuf& b, unsigned long& result);
			//bool recv(ip& from, buf* bufs, int bufcnt, unsigned long& result, unsigned long& flags);

			bool get_result(overlapped&) const;
			bool close();
		};
	}
}