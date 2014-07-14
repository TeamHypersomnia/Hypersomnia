#pragma once
#include "stdafx.h"
#include "reliable_channel.h"
#include "bitstream_wrapper.h"

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
		
		void reliable_sender::post_message(message& output) {
			reliable_buf.push_back(output);
		}

		bool reliable_sender::write_data(bitstream& output) {
			/* handle messages flagged for deletion by decremending reliable ranges in history */
			for (auto& iter : sequence_to_reliable_range) {
				auto new_value = iter.second;

				for (size_t i = 0; i < reliable_buf.size(); ++i) {
					if (reliable_buf[i].flag_for_deletion && i < iter.second)
						--new_value;
				}

				iter.second = new_value;
			}

			/* delete flagged messages from vector */
			reliable_buf.erase(std::remove_if(reliable_buf.begin(), reliable_buf.end(), [](const message& m){ return m.flag_for_deletion; }), reliable_buf.end());
			
			/* if we have nothing to send */
			if (reliable_buf.empty() && unreliable_buf && unreliable_buf->GetNumberOfBitsUsed() <= 0)
				return false;

			/* reliable + maybe unreliable */
			bitstream reliable_bs;

			for (auto& msg : reliable_buf) {
				if (msg.output_bitstream) {
					reliable_bs.name_property("reliable message");
					reliable_bs.WriteBitstream(*msg.output_bitstream);
				}
			}

			if (reliable_bs.GetNumberOfBitsUsed() > 0) {
				output.name_property("has_reliable");
				output.Write<bool>(1);
				output.name_property("sequence");
				output.Write(++sequence);
				output.name_property("ack_sequence");
				output.Write(ack_sequence);

				sequence_to_reliable_range[sequence] = reliable_buf.size();
			}
			else {
				output.name_property("has_reliable");
				output.Write<bool>(0);
			}

			/* only unreliable */
			if (unreliable_buf && unreliable_buf->GetNumberOfBitsUsed() > 0) {
				output.name_property("has_unreliable");
				output.Write<bool>(1);
				output.name_property("unreliable_sequence");
				output.Write(++unreliable_sequence);
				output.name_property("request_ack_for_unreliable");
				output.Write<bool>(request_ack_for_unreliable);

				request_ack_for_unreliable = false;
			}
			else {
				output.name_property("has_unreliable");
				output.Write<bool>(0);
			}

			output.name_property("reliable_buffer");
			output.WriteBitstream(reliable_bs);
			
			if (unreliable_buf) {
				output.name_property("unreliable_buffer");
				output.WriteBitstream(*unreliable_buf);
			}
		
			return true;
		}

		bool reliable_sender::read_ack(bitstream& input) {
			unsigned short reliable_ack = 0u;
			unsigned short unreliable_ack = 0u;

			input.name_property("reliable_ack");
			if (!input.Read(reliable_ack)) return false;
			input.name_property("unreliable_ack");
			if (!input.Read(unreliable_ack)) return false;
			
			if (sequence_more_recent(reliable_ack, ack_sequence)) {
				auto num_messages_to_erase = sequence_to_reliable_range.find(reliable_ack);

				if (num_messages_to_erase != sequence_to_reliable_range.end()) {
					reliable_buf.erase(reliable_buf.begin(), reliable_buf.begin() + (*num_messages_to_erase).second);

					/* for now just clear the sequence history,
					we'll implement partial updates on the client later

					from now on the client won't acknowledge any other packets than those with ack_sequence equal to incoming_ack,
					so we can safely clear the sequence history.
					*/
					sequence_to_reliable_range.clear();
					ack_sequence = reliable_ack;
				}
			}

			if (sequence_more_recent(unreliable_ack, unreliable_ack_sequence)) {
				unreliable_ack_sequence = unreliable_ack;
			}

			return true;
		}
		
		int reliable_receiver::read_sequence(bitstream& input) {
			std::stringstream report;

			unsigned short update_from_sequence = 0u;

			bool has_reliable = false;
			bool has_unreliable = false;
			auto result = MESSAGES_RECEIVED;

			bool request_ack_for_unreliable = false;
			
			input.name_property("has_reliable");
			if (!input.Read<bool>(has_reliable)) return NOTHING_RECEIVED;

			/* reliable + maybe unreliable */
			if (has_reliable) {
				input.name_property("sequence");
				if (!input.Read(received_sequence)) return NOTHING_RECEIVED;
				input.name_property("ack_sequence");
				if (!input.Read(update_from_sequence)) return NOTHING_RECEIVED;

				bool is_recent = sequence_more_recent(received_sequence, last_sequence);

				if (is_recent && update_from_sequence == last_sequence) {
					last_sequence = received_sequence;
					ack_requested = true;
					result = MESSAGES_RECEIVED;
				}
				else if (is_recent) {
					ack_requested = true;
					result = UNMATCHING_RELIABLE_RECEIVED;
				} 
				else {
					/* if even the reliable sequence is out of date, the unreliable sequence will be out of date too, so it can't be read anyway */
					return NOTHING_RECEIVED;
				}
			}

			input.name_property("has_unreliable");
			if (!input.Read<bool>(has_unreliable)) return NOTHING_RECEIVED;
			
			if (has_unreliable) {
				input.name_property("unreliable_sequence");
				if (!input.Read(received_unreliable_sequence)) return NOTHING_RECEIVED;
				input.name_property("request_ack_for_unreliable");
				if (!input.Read(request_ack_for_unreliable)) return NOTHING_RECEIVED;

				if (sequence_more_recent(received_unreliable_sequence, last_unreliable_sequence)) {
					last_unreliable_sequence = received_unreliable_sequence;

					if (request_ack_for_unreliable)
						ack_requested = true;

					return result;
				}
				
				/* discard out of date packets */
				return NOTHING_RECEIVED;
			}

			return result;
		}

		void reliable_receiver::write_ack(bitstream& output) {
			output.name_property("reliable_ack");
			output.Write(last_sequence);
			output.name_property("unreliable_ack");
			output.Write(last_unreliable_sequence);
		}


		void reliable_channel::enable_starting_byte(unsigned char c) {
			starting_byte = c;
			add_starting_byte = true;
		}

		void reliable_channel::disable_starting_byte() {
			add_starting_byte = false;
		}

		int reliable_channel::recv(bitstream& in) {
			if (add_starting_byte) {
				unsigned char byte;
				in.name_property("Starting byte");
				in.Read(byte);
			}

			sender.read_ack(in);
			return receiver.read_sequence(in);
		}


		void reliable_channel::send(bitstream& out) {
			bitstream output_bs;

			if (sender.write_data(output_bs) || receiver.ack_requested) {
				if (add_starting_byte) {
					out.name_property(starting_byte_name);
					out.Write(starting_byte);
				}

				receiver.write_ack(out);
				receiver.ack_requested = false;

				if (output_bs.GetNumberOfBitsUsed() > 0) {
					out.name_property("sender channel");
					out.WriteBitstream(output_bs);
				}
			}
		}
	}
}


#include <gtest\gtest.h>

using namespace augs;
using namespace network;

TEST(NetChannel, SingleTransmissionDeleteAllPending) {
	reliable_sender sender;
	reliable_receiver receiver;

	bitstream bs[15];
	reliable_sender::message msg[15];

	for (int i = 0; i < 15; ++i) {
		bs[i].Write(int(i));
		msg[i].output_bitstream = &bs[i];
	}

	/* post four messages */
	sender.post_message(msg[0]);
	sender.post_message(msg[1]);
	sender.post_message(msg[2]);
	sender.post_message(msg[3]);

	bitstream sender_bs;
	bitstream receiver_bs;

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

	bitstream bs[15];
	reliable_sender::message msg[15];

	for (int i = 0; i < 15; ++i) {
		bs[i].Write(int(i));
		msg[i].output_bitstream = &bs[i];
	}

	bitstream sender_packets[15];
	bitstream receiver_packet;

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

	bitstream bs[15];
	reliable_sender::message msg[15];

	for (int i = 0; i < 15; ++i) {
		bs[i].Write(int(i));
		msg[i].output_bitstream = &bs[i];
	}

	bitstream sender_packets[15];
	bitstream receiver_packet;

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
	
	sender.reliable_buf[0].flag_for_deletion = true;
	sender.reliable_buf[2].flag_for_deletion = true;
	sender.reliable_buf[4].flag_for_deletion = true;
	sender.reliable_buf[5].flag_for_deletion = true;
	sender.reliable_buf[6].flag_for_deletion = true;

	sender.post_message(msg[7]);
	sender.post_message(msg[8]);

	sender.write_data(sender_packets[2]);
	sender.write_data(sender_packets[3]);
	sender.write_data(sender_packets[4]);
	sender.write_data(sender_packets[5]);

	receiver.read_sequence(sender_packets[0]);
	int table[4];
	sender_packets[0].Read(table[0]);
	sender_packets[0].Read(table[1]);
	sender_packets[0].Read(table[2]);
	sender_packets[0].Read(table[3]);

	EXPECT_EQ(0, table[0]);
	EXPECT_EQ(1, table[1]);
	EXPECT_EQ(2, table[2]);
	EXPECT_EQ(3, table[3]);


	receiver.write_ack(receiver_packet);

	sender.read_ack(receiver_packet);

	EXPECT_EQ(6, sender.sequence);
	EXPECT_EQ(1, sender.ack_sequence);
	EXPECT_EQ(2, sender.reliable_buf.size());


	EXPECT_EQ(bs+7, sender.reliable_buf[0].output_bitstream);
	EXPECT_EQ(bs+8, sender.reliable_buf[1].output_bitstream);

	EXPECT_EQ(1, receiver.last_sequence);
}



TEST(NetChannel, SequenceNumberOverflowMultipleTries) {

	reliable_sender sender;
	reliable_receiver receiver;

	sender.sequence = std::numeric_limits<unsigned short>::max();
	sender.ack_sequence = std::numeric_limits<unsigned short>::max();

	receiver.last_sequence = std::numeric_limits<unsigned short>::max();

	for (int k = 0; k < 10; ++k) {
		bitstream bs[15];
		reliable_sender::message msg[15];

		for (int i = 0; i < 15; ++i) {
			bs[i].Write(int(i));
			msg[i].output_bitstream = &bs[i];
		}

		bitstream sender_packets[15];
		bitstream receiver_packet;

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

		sender.reliable_buf[0].flag_for_deletion = true;
		sender.reliable_buf[2].flag_for_deletion = true;
		sender.reliable_buf[4].flag_for_deletion = true;
		sender.reliable_buf[5].flag_for_deletion = true;
		sender.reliable_buf[6].flag_for_deletion = true;

		sender.post_message(msg[7]);
		sender.post_message(msg[8]);

		sender.write_data(sender_packets[2]);
		sender.write_data(sender_packets[3]);
		sender.write_data(sender_packets[4]);
		sender.write_data(sender_packets[5]);

		receiver.read_sequence(sender_packets[0]);
		int table[4];
		sender_packets[0].Read(table[0]);
		sender_packets[0].Read(table[1]);
		sender_packets[0].Read(table[2]);
		sender_packets[0].Read(table[3]);

		EXPECT_EQ(0, table[0]);
		EXPECT_EQ(1, table[1]);
		EXPECT_EQ(2, table[2]);
		EXPECT_EQ(3, table[3]);

		receiver.write_ack(receiver_packet);

		sender.read_ack(receiver_packet);

		if (k == 0) {
			EXPECT_EQ(5, sender.sequence);
			EXPECT_EQ(0, sender.ack_sequence);
			EXPECT_EQ(0, receiver.last_sequence);
		}

		EXPECT_EQ(2, sender.reliable_buf.size());

		EXPECT_EQ(bs + 7, sender.reliable_buf[0].output_bitstream);
		EXPECT_EQ(bs + 8, sender.reliable_buf[1].output_bitstream);

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
		bitstream bs[15];
		reliable_sender::message msg[15];

		for (int i = 0; i < 15; ++i) {
			bs[i].Write(int(i));
			msg[i].output_bitstream = &bs[i];
		}

		bitstream sender_packets[15];
		bitstream receiver_packet;

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

		sender.reliable_buf[0].flag_for_deletion = true;
		sender.reliable_buf[2].flag_for_deletion = true;
		sender.reliable_buf[4].flag_for_deletion = true;
		sender.reliable_buf[5].flag_for_deletion = true;
		sender.reliable_buf[6].flag_for_deletion = true;

		sender.post_message(msg[7]);
		sender.post_message(msg[8]);

		sender.write_data(sender_packets[2]);
		sender.write_data(sender_packets[3]);
		sender.write_data(sender_packets[4]);
		sender.write_data(sender_packets[5]);

		receiver.read_sequence(sender_packets[1]);
		EXPECT_EQ(receiver.NOTHING_RECEIVED, receiver.read_sequence(sender_packets[0]));
		int table[4];
		sender_packets[1].Read(table[0]);
		sender_packets[1].Read(table[1]);
		sender_packets[1].Read(table[2]);
		sender_packets[1].Read(table[3]);

		EXPECT_EQ(0, table[0]);
		EXPECT_EQ(1, table[1]);
		EXPECT_EQ(2, table[2]);
		EXPECT_EQ(3, table[3]);

		receiver.write_ack(receiver_packet);

		sender.read_ack(receiver_packet);

		if (k == 0) {
			EXPECT_EQ(5, sender.sequence);
			EXPECT_EQ(1, sender.ack_sequence);
			EXPECT_EQ(1, receiver.last_sequence);
		}

		EXPECT_EQ(2, sender.reliable_buf.size());

		EXPECT_EQ(bs + 7, sender.reliable_buf[0].output_bitstream);
		EXPECT_EQ(bs + 8, sender.reliable_buf[1].output_bitstream);

		sender.reliable_buf.clear();
	}

}

#include "reliable_channel_tests.h"