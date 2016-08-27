#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <map>

#include "augs/misc/streams.h"

#include "network_types.h"

namespace augs {
	namespace network {
		struct reliable_sender {
			std::vector<augs::stream> reliable_buf;

			std::map<unsigned, unsigned> sequence_to_reliable_range;

			unsigned short sequence = 0u;
			unsigned short ack_sequence = 0u;

			unsigned first_message = 0u;
			unsigned last_message = 0u;

			void post_message(augs::stream&);
			bool write_data(augs::stream& output);
			bool read_ack(augs::stream& input);
		};

		struct reliable_receiver {
			bool ack_requested = false;

			unsigned last_message = 0u;

			unsigned short received_sequence = 0u;
			unsigned short last_sequence = 0u;

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

			std::string last_read_report;
		};


		struct reliable_channel {
			reliable_receiver receiver;
			reliable_sender sender;

			bool add_starting_byte = false;

			std::string starting_byte_name = "GAME_TRANSMISSION";
			unsigned char starting_byte;

			void enable_starting_byte(unsigned char);
			void disable_starting_byte();

			void build_next_packet(augs::stream& out);
			/* returns result enum */
			reliable_receiver::result_data handle_incoming_packet(augs::stream& in);
		};
	}
}