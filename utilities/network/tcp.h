#pragma once
#include "../threads/threads.h"
#include "network.h"

namespace augs  {
	namespace network {
		class overlapped_accept : public augs::threads::overlapped {
			friend class tcp;
			SOCKET accepted;
			bool create_acceptable();
			char addr_local[sizeof(sockaddr_in) + 16], addr_remote[sizeof(sockaddr_in) + 16];
		public:
			overlapped_accept(augs::threads::overlapped_userdata* new_userdata = nullptr);
			~overlapped_accept();
		};

		class tcp {
			friend class augs::threads::iocp;
			friend class overlapped_accept;
			friend bool augs::network::init();
			SOCKET sock;
			static LPFN_ACCEPTEX acceptex;
			static LPFN_GETACCEPTEXSOCKADDRS getacceptexsockaddrs;
			static LPFN_CONNECTEX connectex;
		public:
			ip addr;
			tcp(); ~tcp();
			bool open();
			bool open(overlapped_accept& from);
			// options
			bool nagle(bool onoff);
			bool linger(bool onoff = true, int seconds = 0);

			bool bind(unsigned short port = 0, char* ipv4 = ip::get_local_ipv4());

			bool connect(ip& to, overlapped* request);

			bool listen(int backlog = SOMAXCONN);
			bool accept(overlapped_accept* request);


			// 0 - error, 1 - pending, 2 - completed
			int send(const wsabuf& b, overlapped* request);
			int send(const wsabuf* bufs, int bufcnt, overlapped* request);
			int recv(wsabuf& b, overlapped* request);
			int recv(wsabuf* bufs, int bufcnt, overlapped* request);

			// 0 - error, 1 - successful
			bool send(const wsabuf& b, unsigned long& result, unsigned long flags);
			bool send(const wsabuf* bufs, int bufcnt, unsigned long& result, unsigned long flags);
			bool recv(wsabuf& b, unsigned long& result, unsigned long& flags);
			bool recv(wsabuf* bufs, int bufcnt, unsigned long& result, unsigned long& flags);

			bool close();
		};
	}
}