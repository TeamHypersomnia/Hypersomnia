#include "augs/network/reliable_channel.h"
#include "augs/readwrite/byte_readwrite.h"

namespace augs {
	namespace network {
		template <class T>
		bool sequence_more_recent(T s1, T s2) {
			return
				((s1 > s2) &&
				(s1 - s2 <= std::numeric_limits<T>::max() / 2))
			||	((s2 > s1) &&
				(s2 - s1  > std::numeric_limits<T>::max() / 2))
			;
		}
		
		template <class T>
		T sequence_distance(const T _s1, const T _s2) {
			const auto s1 = _s1 > _s2 ? _s1 : _s2;
			const auto s2 = _s1 > _s2 ? _s2 : _s1;

			if (s1 - s2 <= std::numeric_limits<T>::max() / 2) {
				return s1 - s2;
			}
			else {
				return (std::numeric_limits<T>::max() + 1) - s1 + s2;
			}
		}

		bool reliable_sender::post_message(memory_stream& output) {
			if (last_message - first_message >= std::numeric_limits<unsigned short>::max()) {
				return false;
			}

			reliable_buf.push_back(output);
			++last_message;

			return true;
		}

		void reliable_sender::write_data(memory_stream& output) {
			/* reliable + maybe unreliable */
			memory_stream reliable_bs;

			for (auto& msg : reliable_buf) {
				//reliable_bs.name_property("reliable message");
				reliable_bs.write(msg);
			}

			if (reliable_bs.size() > 0) {
				//output.name_property("has_reliable");
				augs::write_bytes(output, bool(1));
				//output.name_property("sequence");
				augs::write_bytes(output, ++sequence);
				//output.name_property("most_recent_acked_sequence");
				augs::write_bytes(output, most_recent_acked_sequence);

				//output.name_property("first_message");
				augs::write_bytes(output, first_message);
				//output.name_property("last_message");
				
				//if (last_message - first_message > std::numeric_limits<unsigned char>::max()) {
				//	ensure(false);
				//	return false;
				//}

				unsigned short msg_count = last_message - first_message;
				augs::write_bytes(output, msg_count);

				sequence_to_reliable_range[sequence] = last_message;
			}
			else {
				//output.name_property("has_reliable");
				augs::write_bytes(output, bool(0));
			}

			//output.name_property("reliable_buffer");
			output.write(reliable_bs);
		}

		bool reliable_sender::read_ack(memory_stream& input) {
			unsigned short reliable_ack = 0u;

			//input.name_property("reliable_ack");
			
			augs::read_bytes(input, reliable_ack);
			
			if (sequence_more_recent(reliable_ack, most_recent_acked_sequence)) {
				auto last_message_of_acked_sequence = sequence_to_reliable_range.find(reliable_ack);

				if (last_message_of_acked_sequence != sequence_to_reliable_range.end()) {
					auto old_last = (*last_message_of_acked_sequence).second;
					reliable_buf.erase(reliable_buf.begin(), reliable_buf.begin() + (old_last-first_message));
					
					first_message = old_last;

					{
						unsigned short left = most_recent_acked_sequence;
						unsigned short right = reliable_ack;
						++right;

						while (left != right) {
							sequence_to_reliable_range.erase(left++);
						}
					}

					most_recent_acked_sequence = reliable_ack;
				}
			}

			return true;
		}
		
		reliable_receiver::result_data reliable_receiver::read_sequence(memory_stream& input) {
			unsigned short update_from_sequence = 0u;
			unsigned received_first_message = 0u;
			unsigned short received_message_count = 0u;

			unsigned short received_sequence = 0u;

			bool has_reliable = false;

			result_data res;

			//input.name_property("has_reliable");
			augs::read_bytes(input, has_reliable);

			/* reliable + maybe unreliable */
			if (has_reliable) {
				//input.name_property("sequence");
				augs::read_bytes(input, received_sequence);
				//input.name_property("most_recent_acked_sequence");
				augs::read_bytes(input, update_from_sequence);

				//input.name_property("first_message");
				augs::read_bytes(input, received_first_message);
				//input.name_property("last_message");
				augs::read_bytes(input, received_message_count);

				if (!sequence_more_recent(received_sequence, last_received_sequence)) {
					return res;
				}
				
				last_received_sequence = received_sequence;
				ack_requested = true;

				res.messages_to_skip = last_message - received_first_message;
				res.result_type = result_data::MESSAGES_RECEIVED;

				last_message = received_first_message + received_message_count;
			}

			return res;
		}

		void reliable_receiver::write_ack(memory_stream& output) {
			//output.name_property("reliable_ack");
			augs::write_bytes(output, last_received_sequence);
		}

		bool reliable_channel::timed_out(const std::size_t max_unacknowledged_sequences) const {
			return get_num_unacknowledged_sequences() > max_unacknowledged_sequences;
		}

		std::size_t reliable_channel::get_num_unacknowledged_sequences() const {
			//LOG("seq: %x last: %x dist: %x", sender.sequence, sender.most_recent_acked_sequence, sequence_distance(sender.sequence, sender.most_recent_acked_sequence));
			return sequence_distance(sender.sequence, sender.most_recent_acked_sequence);
		}

		std::size_t reliable_channel::get_num_pending_reliable_messages() const {
			return sender.reliable_buf.size();
		}

		std::size_t reliable_channel::get_num_pending_reliable_bytes() const {
			std::size_t output{ 0 };

			for (const auto& r : sender.reliable_buf) {
				output += r.size();
			}

			return output;
		}

		reliable_receiver::result_data reliable_channel::handle_incoming_packet(memory_stream& in) {
			last_received_packet.reset();

			if (!sender.read_ack(in))
				return reliable_receiver::result_data();

			return receiver.read_sequence(in);
		}

		void reliable_channel::build_next_packet(memory_stream& out) {
			memory_stream output_bs;
			sender.write_data(output_bs);

			//if ( || receiver.ack_requested) {
				receiver.write_ack(out);
				receiver.ack_requested = false;

				if (output_bs.size() > 0) {
					//out.name_property("sender channel");
					out.write(output_bs);
				}
			//}
		}
	}
}


#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>

using namespace augs;
using namespace network;

TEST_CASE("NetChannel SequenceArithmetic") {
	typedef unsigned short us;

	REQUIRE(1 == sequence_distance(us(65535), us(0)));
	REQUIRE(1 == sequence_distance(us(0), us(65535)));
	REQUIRE(3 == sequence_distance(us(65535), us(2)));
	REQUIRE(3 == sequence_distance(us(2), us(65535)));
	REQUIRE(1 == sequence_distance(us(2), us(1)));
	REQUIRE(1 == sequence_distance(us(1), us(2)));
	REQUIRE(0 == sequence_distance(us(0), us(0)));
	REQUIRE(0 == sequence_distance(us(65535), us(65535)));

	REQUIRE(sequence_more_recent(us(0), us(65535)));
	REQUIRE(sequence_more_recent(us(1), us(65535)));
	REQUIRE(sequence_more_recent(us(2), us(65535)));
	REQUIRE(sequence_more_recent(us(2), us(65534)));
	REQUIRE(!sequence_more_recent(us(65535), us(0)));
	REQUIRE(!sequence_more_recent(us(65535), us(0)));
	REQUIRE(!sequence_more_recent(us(65535), us(1)));
	REQUIRE(!sequence_more_recent(us(65534), us(2)));
}

TEST_CASE("NetChannel SingleTransmissionDeleteAllPending") {
	reliable_sender sender;
	reliable_receiver receiver;

	
	augs::memory_stream msg[15];

	for (int i = 0; i < 15; ++i) {
		augs::write_bytes(msg[i], int(i));
		
	}

	/* post four messages */
	sender.post_message(msg[0]);
	sender.post_message(msg[1]);
	sender.post_message(msg[2]);
	sender.post_message(msg[3]);

	augs::memory_stream sender_bs;
	augs::memory_stream receiver_bs;

	sender.write_data(sender_bs);

	receiver.read_sequence(sender_bs);
	
	receiver.write_ack(receiver_bs);

	sender.read_ack(receiver_bs);

	sender.post_message(msg[4]);

	REQUIRE(1 == sender.reliable_buf.size());
	REQUIRE(1 == sender.sequence);
	REQUIRE(1 == sender.most_recent_acked_sequence);

	REQUIRE(1 == receiver.last_received_sequence);
}

TEST_CASE("NetChannel PastAcknowledgementDeletesSeveralPending") {
	reliable_sender sender;
	reliable_receiver receiver;
	
	augs::memory_stream msg[15];

	for (int i = 0; i < 15; ++i) {
		augs::write_bytes(msg[i], int(i));
		
	}

	augs::memory_stream sender_packets[15];
	augs::memory_stream receiver_packet;

	/* post four messages */
	sender.post_message(msg[0]);
	sender.post_message(msg[1]);
	sender.post_message(msg[2]);
	sender.post_message(msg[3]);

	sender.write_data(sender_packets[0]);

	sender.post_message(msg[4]);
	sender.post_message(msg[5]);

	sender.write_data(sender_packets[1]);

	sender.post_message(msg[6]);
	sender.post_message(msg[7]);
	sender.post_message(msg[8]);

	sender.write_data(sender_packets[2]);

	receiver.read_sequence(sender_packets[0]);
	receiver.write_ack(receiver_packet);

	sender.read_ack(receiver_packet);

	REQUIRE(3 == sender.sequence);
	REQUIRE(5 == sender.reliable_buf.size());
	REQUIRE(1 == sender.most_recent_acked_sequence);

	REQUIRE(1 == receiver.last_received_sequence);
}

TEST_CASE("NetChannel FlagForDeletionAndAck") {
	reliable_sender sender;
	reliable_receiver receiver;

	
	augs::memory_stream msg[15];

	for (int i = 0; i < 15; ++i) {
		augs::write_bytes(msg[i], int(i));
		
	}

	augs::memory_stream sender_packets[15];
	augs::memory_stream receiver_packet;

	/* post four messages */
	sender.post_message(msg[0]);
	sender.post_message(msg[1]);
	sender.post_message(msg[2]);
	sender.post_message(msg[3]);

	sender.write_data(sender_packets[0]);

	sender.post_message(msg[4]);
	sender.post_message(msg[5]);
	sender.post_message(msg[6]);

	sender.write_data(sender_packets[1]);
	
	//sender.reliable_buf[0].flag_for_deletion = true;
	//sender.reliable_buf[2].flag_for_deletion = true;
	//sender.reliable_buf[4].flag_for_deletion = true;
	//sender.reliable_buf[5].flag_for_deletion = true;
	//sender.reliable_buf[6].flag_for_deletion = true;

	sender.post_message(msg[7]);
	sender.post_message(msg[8]);

	sender.write_data(sender_packets[2]);
	sender.write_data(sender_packets[3]);
	sender.write_data(sender_packets[4]);
	sender.write_data(sender_packets[5]);

	receiver.read_sequence(sender_packets[0]);
	//int table[4];
	//sender_packets[0].Read(table[0]);
	//sender_packets[0].Read(table[1]);
	//sender_packets[0].Read(table[2]);
	//sender_packets[0].Read(table[3]);
	//
	//REQUIRE(0 == table[0]);
	//REQUIRE(1 == table[1]);
	//REQUIRE(2 == table[2]);
	//REQUIRE(3 == table[3]);


	receiver.write_ack(receiver_packet);

	sender.read_ack(receiver_packet);

	REQUIRE(6 == sender.sequence);
	REQUIRE(1 == sender.most_recent_acked_sequence);
	//REQUIRE(2 == sender.reliable_buf.size());


	//REQUIRE(msg+7 == sender.reliable_buf[0].output_bitstream);
	//REQUIRE(msg+8 == sender.reliable_buf[1].output_bitstream);

	REQUIRE(1 == receiver.last_received_sequence);
}

TEST_CASE("NetChannel SequenceNumberOverflowMultipleTries") {
	reliable_sender sender;
	reliable_receiver receiver;

	sender.sequence = std::numeric_limits<unsigned short>::max();
	sender.most_recent_acked_sequence = std::numeric_limits<unsigned short>::max();

	receiver.last_received_sequence = std::numeric_limits<unsigned short>::max();

	for (int k = 0; k < 10; ++k) {
		
		augs::memory_stream msg[15];

		for (int i = 0; i < 15; ++i) {
			augs::write_bytes(msg[i], int(i));
			
		}

		augs::memory_stream sender_packets[15];
		augs::memory_stream receiver_packet;

		/* post four messages */
		sender.post_message(msg[0]);
		sender.post_message(msg[1]);
		sender.post_message(msg[2]);
		sender.post_message(msg[3]);

		sender.write_data(sender_packets[0]);

		sender.post_message(msg[4]);
		sender.post_message(msg[5]);
		sender.post_message(msg[6]);

		sender.write_data(sender_packets[1]);

		//sender.reliable_buf[0].flag_for_deletion = true;
		//sender.reliable_buf[2].flag_for_deletion = true;
		//sender.reliable_buf[4].flag_for_deletion = true;
		//sender.reliable_buf[5].flag_for_deletion = true;
		//sender.reliable_buf[6].flag_for_deletion = true;

		sender.post_message(msg[7]);
		sender.post_message(msg[8]);

		sender.write_data(sender_packets[2]);
		sender.write_data(sender_packets[3]);
		sender.write_data(sender_packets[4]);
		sender.write_data(sender_packets[5]);

		receiver.read_sequence(sender_packets[0]);
		//int table[4];
		//sender_packets[0].Read(table[0]);
		//sender_packets[0].Read(table[1]);
		//sender_packets[0].Read(table[2]);
		//sender_packets[0].Read(table[3]);
		//
		//REQUIRE(0 == table[0]);
		//REQUIRE(1 == table[1]);
		//REQUIRE(2 == table[2]);
		//REQUIRE(3 == table[3]);

		receiver.write_ack(receiver_packet);

		sender.read_ack(receiver_packet);

		if (k == 0) {
			REQUIRE(5 == sender.sequence);
			REQUIRE(0 == sender.most_recent_acked_sequence);
			REQUIRE(0 == receiver.last_received_sequence);
		}

		//REQUIRE(2 == sender.reliable_buf.size());

		//REQUIRE(msg + 7 == sender.reliable_buf[0].output_bitstream);
		//REQUIRE(msg + 8 == sender.reliable_buf[1].output_bitstream);

		sender.reliable_buf.clear();
	}
}


TEST_CASE("NetChannel OutOfDatePackets") {
	reliable_sender sender;
	reliable_receiver receiver;

	sender.sequence = std::numeric_limits<unsigned short>::max();
	sender.most_recent_acked_sequence = std::numeric_limits<unsigned short>::max();

	receiver.last_received_sequence = std::numeric_limits<unsigned short>::max();

	for (int k = 0; k < 10; ++k) {
		
		augs::memory_stream msg[15];

		for (int i = 0; i < 15; ++i) {
			augs::write_bytes(msg[i], int(i));
			
		}

		augs::memory_stream sender_packets[15];
		augs::memory_stream receiver_packet;

		/* post four messages */
		sender.post_message(msg[0]);
		sender.post_message(msg[1]);
		sender.post_message(msg[2]);
		sender.post_message(msg[3]);

		sender.write_data(sender_packets[0]);

		sender.post_message(msg[4]);
		sender.post_message(msg[5]);
		sender.post_message(msg[6]);

		sender.write_data(sender_packets[1]);

		//sender.reliable_buf[0].flag_for_deletion = true;
		//sender.reliable_buf[2].flag_for_deletion = true;
		//sender.reliable_buf[4].flag_for_deletion = true;
		//sender.reliable_buf[5].flag_for_deletion = true;
		//sender.reliable_buf[6].flag_for_deletion = true;

		sender.post_message(msg[7]);
		sender.post_message(msg[8]);

		sender.write_data(sender_packets[2]);
		sender.write_data(sender_packets[3]);
		sender.write_data(sender_packets[4]);
		sender.write_data(sender_packets[5]);

		receiver.read_sequence(sender_packets[1]);
		REQUIRE(reliable_receiver::result_data::NOTHING_RECEIVED == receiver.read_sequence(sender_packets[0]).result_type);
		//int table[4];
		//sender_packets[1].Read(table[0]);
		//sender_packets[1].Read(table[1]);
		//sender_packets[1].Read(table[2]);
		//sender_packets[1].Read(table[3]);

		//REQUIRE(0 == table[0]);
		//REQUIRE(1 == table[1]);
		//REQUIRE(2 == table[2]);
		//REQUIRE(3 == table[3]);

		receiver.write_ack(receiver_packet);

		sender.read_ack(receiver_packet);

		if (k == 0) {
			REQUIRE(5 == sender.sequence);
			REQUIRE(1 == sender.most_recent_acked_sequence);
			REQUIRE(1 == receiver.last_received_sequence);
		}

		//REQUIRE(2 == sender.reliable_buf.size());

		//REQUIRE(msg + 7 == sender.reliable_buf[0].output_bitstream);
		//REQUIRE(msg + 8 == sender.reliable_buf[1].output_bitstream);

		sender.reliable_buf.clear();
	}

}

#include "reliable_channel_tests.h"
#endif