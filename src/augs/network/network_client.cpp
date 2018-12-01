#if BUILD_NETWORKING

#endif

#include "augs/misc/readable_bytesize.h"
#include "augs/network/network_client.h"

namespace augs {
	namespace network {
		bool client::connect(
			const std::string& address,
			const unsigned timeout_ms
		) {
#if BUILD_NETWORKING
			(void)address;
			(void)timeout_ms;
			return false;
#else
			LOG("Warning! Networking backend was not built.");

			(void)address;
			(void)timeout_ms;
			return false;
#endif
		}

		bool client::is_connected() const {
			return false;
		}

		bool client::post_redundant(const packet& payload) {
#if BUILD_NETWORKING
			if (!is_connected()) {
				return false;
			}

			packet stream = payload;
			return redundancy.sender.post_message(stream);
#else
			(void)payload;
			return false;
#endif
		}

		bool client::send_pending_redundant() {
#if BUILD_NETWORKING
			if (!is_connected()) {
				return false;
			}

			augs::memory_stream payload;

			redundancy.build_next_packet(payload);

			return false;
#else
			return false;
#endif
		}

		bool client::send_reliable(const packet& payload) {
#if BUILD_NETWORKING
			if (!is_connected()) {
				return false;
			}

			(void)payload;
			return false;
#else
			(void)payload;
			return false;
#endif
		}

		bool client::has_timed_out(const float sequence_interval_ms, const float ms) const {
			return redundancy.timed_out(static_cast<size_t>(ms / sequence_interval_ms));
		}

		unsigned client::total_bytes_received() const {
#if BUILD_NETWORKING
			return 0;
#else
			return 0;
#endif
		}

		unsigned client::total_bytes_sent() const {
#if BUILD_NETWORKING
			return 0;
#else
			return 0;
#endif
		}

		unsigned client::total_packets_sent() const {
#if BUILD_NETWORKING
			return 0;
#else
			return 0;
#endif
		}

		unsigned client::total_packets_received() const {
#if BUILD_NETWORKING
			return 0;
#else
			return 0;
#endif
		}

		std::string client::format_transmission_details() const {
			return typesafe_sprintf("RTT:\nSent: %x (%x)\nReceived: %x (%x)\nLast sent: %x\nLast received: %x",
				total_packets_sent(), readable_bytesize(total_bytes_sent()), total_packets_received(), readable_bytesize(total_bytes_received()),
				readable_bytesize(static_cast<unsigned>(sent_size.get_average_units())),
				readable_bytesize(static_cast<unsigned>(recv_size.get_average_units()))
			);
		}

		void client::disconnect() {
#if BUILD_NETWORKING

#endif
		}

		void client::forceful_disconnect() {
#if BUILD_NETWORKING

#endif
		}

		std::vector<message> client::collect_entropy() {
			std::vector<message> total;
			
#if BUILD_NETWORKING
			if (!is_connected()) {
				return total;
			}
#endif
			return total;
		}
	}
}