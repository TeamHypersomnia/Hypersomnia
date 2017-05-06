#pragma once
#include "augs/log.h"

struct _ENetHost;
typedef struct _ENetHost ENetHost;

namespace augs {
	namespace network {
		class ENetHost_raii {
			ENetHost* ptr = nullptr;
		public:
			template<class... Args>
			bool init(Args... args) {
				ptr = enet_host_create(args...);

				if (ptr == nullptr) {
					LOG("An error occurred while trying to create an ENet client host.");
					return false;
				}

				return true;
			}

			ENetHost_raii() = default;
			~ENetHost_raii();
			
			ENetHost* get();
			const ENetHost* get() const;

			ENetHost_raii(const ENetHost_raii&) = delete;
			ENetHost_raii(ENetHost_raii&&) = delete;
			ENetHost_raii& operator=(const ENetHost_raii&) = delete;
			ENetHost_raii& operator=(ENetHost_raii&&) = delete;
		};
	}
}