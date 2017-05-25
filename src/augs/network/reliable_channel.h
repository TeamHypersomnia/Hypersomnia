#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <map>

#include "augs/misc/streams.h"
#include "augs/misc/timer.h"

#include "network_types.h"

namespace augs {
	namespace network {
		struct reliable_sender {
			std::vector<augs::stream> reliable_buf;

			std::map<unsigned, unsigned> sequence_to_reliable_range;

			unsigned short sequence = 0u;
			unsigned short most_recent_acked_sequence = 0u;

			unsigned first_message = 0u;
			unsigned last_message = 0u;

			bool post_message(augs::stream&);
			void write_data(augs::stream& output);
			bool read_ack(augs::stream& input);
		};

		struct reliable_receiver {
			unsigned last_message = 0u;
			unsigned short last_received_sequence = 0u;
			bool ack_requested = false;

			struct result_data {
				enum type {
					NOTHING_RECEIVED,
					MESSAGES_RECEIVED
				} result_type = NOTHING_RECEIVED;

				unsigned messages_to_skip = 0;
			};

			/* returns how many messages to skip if message_indexing == true */
			reliable_receiver::result_data read_sequence(augs::stream& input);
			void write_ack(augs::stream& input);
		};


		struct reliable_channel {
			reliable_receiver receiver;
			reliable_sender sender;

			augs::timer last_received_packet;

			bool timed_out(const size_t max_unacknowledged_sequences = 120) const;

			unsigned get_unacknowledged_sequences_num() const;
			unsigned get_pending_reliable_messages_num() const;
			unsigned get_pending_reliable_bytes_num() const;

			void build_next_packet(augs::stream& out);
			/* returns result enum */
			reliable_receiver::result_data handle_incoming_packet(augs::stream& in);
		};
	}
}