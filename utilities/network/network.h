#pragma once
#include "../threads/threads.h"

namespace augs  {
	namespace network {
		bool init(), deinit();

		enum io_result {
			SUCCESSFUL,
			FAILED,
			INCOMPLETE
		};

		struct packet {
			std::vector<char> data;
			
			template <typename T>
			T read() {
				if (data.size() < sizeof(T)) 
					throw std::runtime_error("attempting to read more bytes than buffer has!");

				size_t old_size = data.size();
				
				T return_value;
				memcpy(&return_value, data.data() + old_size - sizeof(T), sizeof(T));
				
				data.resize(old_size - sizeof(T));

				return return_value;
			}

			template <typename T>
			void write(const T& value) {
				size_t old_size = data.size();

				data.resize(old_size + sizeof(value));
				memcpy(data.data() + old_size, &value, sizeof(value));
			}

			template <>
			void write<std::string>(const std::string& value) {
				size_t old_size = data.size();

				data.resize(old_size + value.length());
				memcpy(data.data() + old_size, value.c_str(), value.length());
			}
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
