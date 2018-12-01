#if BUILD_NETWORKING

#endif

#include "network_server.h"
#include "augs/readwrite/byte_readwrite.h"

namespace augs {
	namespace network {
		bool server::listen(const server_listen_input& in) {
#if BUILD_NETWORKING
			(void)in;
			return false;
#else
			(void)in;
			LOG("Warning! Networking backend was not built.");
			return false;
#endif
		}

		bool server::post_redundant(packet&& payload, const endpoint_address& target) {
			return peer_map.at(target).redundancy.sender.post_message(std::move(payload));
		}

		bool server::has_endpoint(const endpoint_address& target) const {
			return peer_map.find(target) != peer_map.end();
		}

		bool server::has_timed_out(const endpoint_address& target, const float sequence_interval_ms, const float ms) const {
			return has_endpoint(target) && peer_map.at(target).redundancy.timed_out(static_cast<size_t>(ms / sequence_interval_ms));
		}

		bool server::send_pending_redundant() {
#if BUILD_NETWORKING
			bool result = true;

			for (auto& e : peer_map) {
				augs::memory_stream payload;
				
				e.second.redundancy.build_next_packet(payload);
			}

			return result;
#endif
			return false;
		}

		bool server::send_reliable(const packet& payload, const endpoint_address& target) {
#if BUILD_NETWORKING
			(void)payload;
			(void)target;
			return false;
#else
			(void)payload;
			(void)target;
#endif
			return false;
		}

		void server::disconnect(const endpoint_address& target) {
#if BUILD_NETWORKING
			(void)target;
#else
			(void)target;
#endif
		}

		void server::forceful_disconnect(const endpoint_address& target) {
#if BUILD_NETWORKING
			(void)target;
#else
			(void)target;
#endif
			peer_map.erase(target);
		}

		std::vector<message> server::collect_entropy() {
			std::vector<message> total;

#if BUILD_NETWORKING
#endif
			return total;
		}
	}
}