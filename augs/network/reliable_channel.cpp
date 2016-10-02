#include "augs/misc/templated_readwrite.h"
#include "reliable_channel.h"

namespace augs {
	namespace network {
		template <class T>
		bool sequence_more_recent(T s1, T s2)
		{
			return
				(s1 > s2) &&
				(s1 - s2 <= std::numeric_limits<T>::max() / 2)
				||
				(s2 > s1) &&
				(s2 - s1  > std::numeric_limits<T>::max() / 2);
		}
		
		bool reliable_sender::post_message(augs::stream& output) {
			if (last_message - first_message >= std::numeric_limits<unsigned short>::max()) {
				return false;
			}

			reliable_buf.push_back(output);
			++last_message;

			return true;
		}

		void reliable_sender::write_data(augs::stream& output) {
			/* reliable + maybe unreliable */
			augs::stream reliable_bs;

			for (auto& msg : reliable_buf) {
				reliable_bs;//name_property("reliable message");
				augs::write_object(reliable_bs, msg);
			}

			if (reliable_bs.size() > 0) {
				output;//name_property("has_reliable");
				augs::write_object(output, bool(1));
				output;//name_property("sequence");
				augs::write_object(output, ++sequence);
				output;//name_property("ack_sequence");
				augs::write_object(output, ack_sequence);

				output;//name_property("first_message");
				augs::write_object(output, first_message);
				output;//name_property("last_message");
				
				//if (last_message - first_message > std::numeric_limits<unsigned char>::max()) {
				//	ensure(false);
				//	return false;
				//}

				unsigned short msg_count = last_message - first_message;
				augs::write_object(output, msg_count);

				sequence_to_reliable_range[sequence] = last_message;
			}
			else {
				output;//name_property("has_reliable");
				augs::write_object(output, bool(0));
			}

			output;//name_property("reliable_buffer");
			augs::write_object(output, reliable_bs);
		}

		bool reliable_sender::read_ack(augs::stream& input) {
			unsigned short reliable_ack = 0u;

			input;//name_property("reliable_ack");
			if (!augs::read_object(input, reliable_ack)) return false;
			
			if (sequence_more_recent(reliable_ack, ack_sequence)) {
				auto last_message_of_acked_sequence = sequence_to_reliable_range.find(reliable_ack);

				if (last_message_of_acked_sequence != sequence_to_reliable_range.end()) {
					auto old_last = (*last_message_of_acked_sequence).second;
					reliable_buf.erase(reliable_buf.begin(), reliable_buf.begin() + (old_last-first_message));
					
					first_message = old_last;

					{
						unsigned short left = ack_sequence;
						unsigned short right = reliable_ack;
						++right;

						while (left != right)
							sequence_to_reliable_range.erase(left++);
					}

					ack_sequence = reliable_ack;
				}
			}

			return true;
		}
		
		reliable_receiver::result_data reliable_receiver::read_sequence(augs::stream& input) {
			std::stringstream report;

			unsigned short update_from_sequence = 0u;
			unsigned received_first_message = 0u;
			unsigned short received_message_count = 0u;

			bool has_reliable = false;
			bool request_ack_for_unreliable = false;

			result_data res;

			input;//name_property("has_reliable");
			if (!augs::read_object(input, has_reliable)) return res;

			/* reliable + maybe unreliable */
			if (has_reliable) {
				input;//name_property("sequence");
				if (!augs::read_object(input, received_sequence)) return res;
				input;//name_property("ack_sequence");
				if (!augs::read_object(input, update_from_sequence)) return res;

				input;//name_property("first_message");
				if (!augs::read_object(input, received_first_message)) return res;
				input;//name_property("last_message");
				if (!augs::read_object(input, received_message_count)) return res;

				if (!sequence_more_recent(received_sequence, last_sequence))
					return res;
				
				last_sequence = received_sequence;
				ack_requested = true;

				res.messages_to_skip = last_message - received_first_message;
				res.result_type = result_data::MESSAGES_RECEIVED;

				last_message = received_first_message + received_message_count;
			}

			return res;
		}

		void reliable_receiver::write_ack(augs::stream& output) {
			output;//name_property("reliable_ack");
			augs::write_object(output, last_sequence);
		}

		bool reliable_channel::timed_out(const float ms, const size_t max_pending_reliable_messages) const {
			return sender.reliable_buf.size() > max_pending_reliable_messages || last_received_packet.get<std::chrono::milliseconds>() >= ms;
		}

		reliable_receiver::result_data reliable_channel::handle_incoming_packet(augs::stream& in) {
			last_received_packet.reset();

			if (!sender.read_ack(in))
				return reliable_receiver::result_data();

			return receiver.read_sequence(in);
		}

		void reliable_channel::build_next_packet(augs::stream& out) {
			augs::stream output_bs;
			sender.write_data(output_bs);

			//if ( || receiver.ack_requested) {
				receiver.write_ack(out);
				receiver.ack_requested = false;

				if (output_bs.size() > 0) {
					out;//name_property("sender channel");
					augs::write_object(out, output_bs);
				}
			//}
		}
	}
}


#include <gtest\gtest.h>

using namespace augs;
using namespace network;

TEST(NetChannel, SingleTransmissionDeleteAllPending) {
	reliable_sender sender;
	reliable_receiver receiver;

	
	augs::stream msg[15];

	for (int i = 0; i < 15; ++i) {
		augs::write_object(msg[i], int(i));
		
	}

	/* post four messages */
	sender.post_message(msg[0]);
	sender.post_message(msg[1]);
	sender.post_message(msg[2]);
	sender.post_message(msg[3]);

	augs::stream sender_bs;
	augs::stream receiver_bs;

	sender.write_data(sender_bs);

	receiver.read_sequence(sender_bs);
	
	receiver.write_ack(receiver_bs);

	sender.read_ack(receiver_bs);

	sender.post_message(msg[4]);

	EXPECT_EQ(1, sender.reliable_buf.size());
	EXPECT_EQ(1, sender.sequence);
	EXPECT_EQ(1, sender.ack_sequence);

	EXPECT_EQ(1, receiver.last_sequence);
}

TEST(NetChannel, PastAcknowledgementDeletesSeveralPending) {
	reliable_sender sender;
	reliable_receiver receiver;

	
	augs::stream msg[15];

	for (int i = 0; i < 15; ++i) {
		augs::write_object(msg[i], int(i));
		
	}

	augs::stream sender_packets[15];
	augs::stream receiver_packet;

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

	EXPECT_EQ(3, sender.sequence);
	EXPECT_EQ(5, sender.reliable_buf.size());
	EXPECT_EQ(1, sender.ack_sequence);

	EXPECT_EQ(1, receiver.last_sequence);
}

TEST(NetChannel, FlagForDeletionAndAck) {
	reliable_sender sender;
	reliable_receiver receiver;

	
	augs::stream msg[15];

	for (int i = 0; i < 15; ++i) {
		augs::write_object(msg[i], int(i));
		
	}

	augs::stream sender_packets[15];
	augs::stream receiver_packet;

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
	//EXPECT_EQ(0, table[0]);
	//EXPECT_EQ(1, table[1]);
	//EXPECT_EQ(2, table[2]);
	//EXPECT_EQ(3, table[3]);


	receiver.write_ack(receiver_packet);

	sender.read_ack(receiver_packet);

	EXPECT_EQ(6, sender.sequence);
	EXPECT_EQ(1, sender.ack_sequence);
	//EXPECT_EQ(2, sender.reliable_buf.size());


	//EXPECT_EQ(msg+7, sender.reliable_buf[0].output_bitstream);
	//EXPECT_EQ(msg+8, sender.reliable_buf[1].output_bitstream);

	EXPECT_EQ(1, receiver.last_sequence);
}



TEST(NetChannel, SequenceNumberOverflowMultipleTries) {

	reliable_sender sender;
	reliable_receiver receiver;

	sender.sequence = std::numeric_limits<unsigned short>::max();
	sender.ack_sequence = std::numeric_limits<unsigned short>::max();

	receiver.last_sequence = std::numeric_limits<unsigned short>::max();

	for (int k = 0; k < 10; ++k) {
		
		augs::stream msg[15];

		for (int i = 0; i < 15; ++i) {
			augs::write_object(msg[i], int(i));
			
		}

		augs::stream sender_packets[15];
		augs::stream receiver_packet;

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
		//EXPECT_EQ(0, table[0]);
		//EXPECT_EQ(1, table[1]);
		//EXPECT_EQ(2, table[2]);
		//EXPECT_EQ(3, table[3]);

		receiver.write_ack(receiver_packet);

		sender.read_ack(receiver_packet);

		if (k == 0) {
			EXPECT_EQ(5, sender.sequence);
			EXPECT_EQ(0, sender.ack_sequence);
			EXPECT_EQ(0, receiver.last_sequence);
		}

		//EXPECT_EQ(2, sender.reliable_buf.size());

		//EXPECT_EQ(msg + 7, sender.reliable_buf[0].output_bitstream);
		//EXPECT_EQ(msg + 8, sender.reliable_buf[1].output_bitstream);

		sender.reliable_buf.clear();
	}
}


TEST(NetChannel, OutOfDatePackets) {
	reliable_sender sender;
	reliable_receiver receiver;

	sender.sequence = std::numeric_limits<unsigned short>::max();
	sender.ack_sequence = std::numeric_limits<unsigned short>::max();

	receiver.last_sequence = std::numeric_limits<unsigned short>::max();

	for (int k = 0; k < 10; ++k) {
		
		augs::stream msg[15];

		for (int i = 0; i < 15; ++i) {
			augs::write_object(msg[i], int(i));
			
		}

		augs::stream sender_packets[15];
		augs::stream receiver_packet;

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
		EXPECT_EQ(reliable_receiver::result_data::NOTHING_RECEIVED, receiver.read_sequence(sender_packets[0]).result_type);
		//int table[4];
		//sender_packets[1].Read(table[0]);
		//sender_packets[1].Read(table[1]);
		//sender_packets[1].Read(table[2]);
		//sender_packets[1].Read(table[3]);

		//EXPECT_EQ(0, table[0]);
		//EXPECT_EQ(1, table[1]);
		//EXPECT_EQ(2, table[2]);
		//EXPECT_EQ(3, table[3]);

		receiver.write_ack(receiver_packet);

		sender.read_ack(receiver_packet);

		if (k == 0) {
			EXPECT_EQ(5, sender.sequence);
			EXPECT_EQ(1, sender.ack_sequence);
			EXPECT_EQ(1, receiver.last_sequence);
		}

		//EXPECT_EQ(2, sender.reliable_buf.size());

		//EXPECT_EQ(msg + 7, sender.reliable_buf[0].output_bitstream);
		//EXPECT_EQ(msg + 8, sender.reliable_buf[1].output_bitstream);

		sender.reliable_buf.clear();
	}

}

#include "reliable_channel_tests.h"