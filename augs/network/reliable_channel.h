#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <map>

#include "augs/misc/streams.h"

namespace augs {
	namespace network {
		struct reliable_sender {
			struct message {
				augs::stream* output_bitstream = nullptr;
			};

			std::vector<message> reliable_buf;
			augs::stream unreliable_buf;
			augs::stream custom_header;

			std::map<unsigned, unsigned> sequence_to_reliable_range;

			unsigned short unreliable_sequence = 0u;
			unsigned short unreliable_ack_sequence = 0u;
			unsigned short sequence = 0u;
			unsigned short ack_sequence = 0u;

			bool request_ack_for_unreliable = false;

			bool message_indexing = false;
			unsigned first_message = 0u;
			unsigned last_message = 0u;

			void post_message(message&);
			bool write_data(augs::stream& output);
			bool read_ack(augs::stream& input);
		};

		struct reliable_receiver {
			bool message_indexing = false;
			bool ack_requested = false;

			bool has_reliable = false;
			bool has_unreliable = false;

			unsigned last_message = 0u;
			unsigned first_message = 0u;

			unsigned short received_sequence = 0u;
			unsigned short received_unreliable_sequence = 0u;

			unsigned short last_sequence = 0u;
			unsigned short last_unreliable_sequence = 0u;

			enum result {
				NOTHING_RECEIVED = -1,
				MESSAGES_RECEIVED = 0,
				UNMATCHING_RELIABLE_RECEIVED = 1
			};

			/* returns result enum or how many messages to skip if message_indexing == true */
			int read_sequence(augs::stream& input);
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

			void send(augs::stream& out);
			/* returns result enum */
			int recv(augs::stream& in);
		};
	}
}