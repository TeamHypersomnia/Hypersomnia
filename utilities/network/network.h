#pragma once
#include "../threads/threads.h"

namespace augs  {
	namespace network {
		bool init(), deinit();

		struct overlapped : public augs::threads::overlapped {
			DWORD flags;
			overlapped();
		};

		class overlapped_accept : public augs::threads::overlapped {
			friend class tcp;
			SOCKET accepted;
			bool create_acceptable();
			char addr_local[sizeof(sockaddr_in) + 16], addr_remote[sizeof(sockaddr_in) + 16];
		public:
			overlapped_accept();
			~overlapped_accept();
		};

		struct ip {
				struct sockaddr_in addr;
				static int size;
				ip();
				ip(unsigned short port, char* ipv4);
				char* get_ipv4();
				unsigned short get_port();
				static char* get_local_ipv4();
		};

		class buf {
			WSABUF b;
		public:
			buf(void* data, int len);
			void set(void* data, int len),
				*get() const;
			int get_len() const;
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
			int send(const buf& b, overlapped* request);
			int send(const buf* bufs, int bufcnt, overlapped* request);
			int recv(buf& b, overlapped* request);
			int recv(buf* bufs, int bufcnt, overlapped* request);

			// 0 - error, 1 - successful
			bool send(const buf& b, unsigned long& result, unsigned long flags);
			bool send(const buf* bufs, int bufcnt, unsigned long& result, unsigned long flags);
			bool recv(buf& b, unsigned long& result, unsigned long& flags);
			bool recv(buf* bufs, int bufcnt, unsigned long& result, unsigned long& flags);

			bool close();
		};

		class udp {
			friend class augs::threads::iocp;
			SOCKET sock;
			ip addr;
		public:
			udp(); ~udp();
			bool open();
			bool bind(unsigned short port, char* ipv4 = ip::get_local_ipv4());
			
			// 0 - error, 1 - pending, 2 - completed
			int send(const ip& to, const buf& b, overlapped* request);
			int send(const ip& to, const buf* bufs, int bufcnt, overlapped* request);
			int recv(ip& from, buf& b, overlapped* request);
			int recv(ip& from, buf* bufs, int bufcnt, overlapped* request);

			// 0 - error, 1 - successful
			bool send(const ip& to, const buf& b, unsigned long& result, unsigned long flags);
			bool send(const ip& to, const buf* bufs, int bufcnt, unsigned long& result, unsigned long flags);
			bool recv(ip& from, buf& b, unsigned long& result, unsigned long& flags);
			bool recv(ip& from, buf* bufs, int bufcnt, unsigned long& result, unsigned long& flags);

			bool get_result(overlapped&) const;
			bool close();

		};
	}
}
