#pragma once
#include <vector>
#include <unordered_map>
#include <memory>

namespace augs {
	namespace network {
		struct bitstream;

		struct reliable_sender {
			struct message {
				luabind::object script;
				bitstream* output_bitstream = nullptr;
			};

			std::vector<message> reliable_buf;
			bitstream unreliable_buf;

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
			bool write_data(bitstream& output);
			bool read_ack(bitstream& input);
		};

		struct reliable_receiver {
			bool message_indexing = false;
			bool ack_requested = false;

			unsigned last_message;
			unsigned first_message;

			unsigned short received_sequence = 0u;
			unsigned short received_unreliable_sequence = 0u;

			unsigned short last_sequence = 0u;
			unsigned short last_unreliable_sequence = 0u;

			enum result {
				NOTHING_RECEIVED = -1,
				MESSAGES_RECEIVED,
				UNMATCHING_RELIABLE_RECEIVED
			};

			/* returns result enum or how many messages to skip if message_indexing == true */
			int read_sequence(bitstream& input);
			void write_ack(bitstream& input);

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

			void send(bitstream& out);
			/* returns result enum */
			int recv(bitstream& in);
		};
	}
}