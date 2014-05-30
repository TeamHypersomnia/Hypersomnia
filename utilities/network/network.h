#pragma once
#include "../threads/threads.h"

namespace augs  {
	namespace network {
		bool init(), deinit();

		struct ip {
			struct sockaddr_in addr;
			static int size;
			ip();
			ip(unsigned short port, char* ipv4);
			char* get_ipv4();
			unsigned short get_port();
			static char* get_local_ipv4();
		};

		class wsabuf {
			WSABUF b;
		public:
			wsabuf(void* data = nullptr, int len = 0);
			void set(void* data, int len),
				*get() const;
			int get_len() const;
		};

		struct overlapped : public augs::threads::overlapped {
			DWORD flags;
			overlapped(augs::threads::overlapped_userdata* new_userdata = nullptr);

			ip associated_address;
			wsabuf associated_buffer;

			void reset();
		};
	}
}
