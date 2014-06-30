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
			if (!enable_partial_updates) {
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
			}
			
			/* if we have nothing to send */
			if (reliable_buf.empty() && unreliable_buf && unreliable_buf->GetNumberOfBitsUsed() <= 0)
				return false;

			/* reliable + maybe unreliable */
			if (!reliable_buf.empty()) {
				bitstream reliable_bs;
				
				for (auto& msg : reliable_buf)
					if (msg.output_bitstream) {
					reliable_bs.name_property("reliable message");
					reliable_bs.WriteBitstream(*msg.output_bitstream);
					}

				output.name_property("has_reliable");
				output.Write<bool>(reliable_bs.GetNumberOfBitsUsed() > 0);
				output.name_property("sequence");
				output.Write(++sequence);
				output.name_property("ack_sequence");
				output.Write(ack_sequence);

				output.name_property("reliable buffer");
				output.WriteBitstream(reliable_bs);

				sequence_to_reliable_range[sequence] = reliable_buf.size();
			}
			/* only unreliable */
			else {
				output.name_property("has_reliable");
				output.Write<bool>(0);
				output.name_property("unreliable_only_sequence");
				output.Write(++unreliable_only_sequence);
			}

			/* either way append unreliable buffer */
			if (unreliable_buf)  {
				output.name_property("unreliable buffer");
				output.WriteBitstream(*unreliable_buf);
			}
		
			return true;
		}

		bool reliable_sender::read_ack(bitstream& input) {
			unsigned short incoming_ack = 0u;
			
			input.name_property("incoming ack");
			if (input.Read(incoming_ack)) {
				if (sequence_more_recent(incoming_ack, ack_sequence)) {
					auto num_messages_to_erase = sequence_to_reliable_range.find(incoming_ack);

					if (num_messages_to_erase != sequence_to_reliable_range.end()) {
						reliable_buf.erase(reliable_buf.begin(), reliable_buf.begin() + (*num_messages_to_erase).second);

						if (!enable_partial_updates) {
							/* for now just clear the sequence history,
							we'll implement partial updates on the client later

							from now on the client won't acknowledge any other packets than those with ack_sequence equal to incoming_ack,
							so we can safely clear the sequence history.
							*/
							sequence_to_reliable_range.clear();
						} 
						else {
							/* iterate through ranges and decrease them by acknowledged sequence length */
							unsigned acknowledged_length = sequence_to_reliable_range[incoming_ack];

							sequence_to_reliable_range.erase(incoming_ack);

							for (unsigned i = incoming_ack; i != sequence+1; ++i) {
								auto iter = sequence_to_reliable_range.find(i);
								
								if (iter != sequence_to_reliable_range.end()) {
									(*iter).second -= acknowledged_length;

									if ((*iter).second == 0u)
										sequence_to_reliable_range.erase(iter);
								}
							}
						}
						
						ack_sequence = incoming_ack;
					}
				}

				return true;
			}

			return false;
		}
		
		int reliable_receiver::read_sequence(bitstream& input) {
			std::stringstream report;

			unsigned short update_to_sequence = 0u;
			unsigned short update_from_sequence = 0u;

			bool reliable_length = 0u;
			
			input.name_property("has_reliable");
			if (!input.Read<bool>(reliable_length)) return NOTHING_RECEIVED;

			/* reliable + maybe unreliable */
			if (reliable_length > 0) {
				input.name_property("sequence");
				if (!input.Read(update_to_sequence)) return NOTHING_RECEIVED;
				input.name_property("ack_sequence");
				if (!input.Read(update_from_sequence)) return NOTHING_RECEIVED;

				bool is_recent = sequence_more_recent(update_to_sequence, last_sequence);

				if (is_recent && update_from_sequence == last_sequence) {
					last_sequence = update_to_sequence;
					ack_requested = true;
					return RELIABLE_RECEIVED;
				}
				/*
				we can't only rely on the unreliable data of the moment
				*/
				else if (is_recent)
					ack_requested = true;
				
				return NOTHING_RECEIVED;
			}
			/* only unreliable */
			else {
				input.name_property("unreliable_only_sequence");
				if (!input.Read(update_to_sequence)) return NOTHING_RECEIVED;
				
				if (sequence_more_recent(update_to_sequence, last_unreliable_only_sequence)) {
					last_unreliable_only_sequence = update_to_sequence;

					return ONLY_UNRELIABLE_RECEIVED;
				}
				
				/* discard out of date packets */
				return NOTHING_RECEIVED;
			}
		}

		void reliable_receiver::write_ack(bitstream& output) {
			output.name_property("receiver channel ack");
			output.Write(last_sequence);
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

TEST(NetChannel, PartialUpdatesSimpleCase) {
	reliable_channel a, b;
	a.enable_starting_byte(135);
	b.enable_starting_byte(135);

	a.sender.enable_partial_updates = true;
	a.receiver.enable_partial_updates = true;
	a.sender.enable_partial_updates = true;
	b.receiver.enable_partial_updates = true;

	bitstream bs[15];
	reliable_sender::message msg[15];

	bitstream s_packets[15];
	bitstream r_packets[15];

	for (int i = 0; i < 15; ++i) {
		bs[i].Write(int(i));
		msg[i].output_bitstream = &bs[i];
	}

	for (int i = 0; i < 15; ++i) {
		a.sender.post_message(msg[i]);
		a.send(s_packets[i]);

		b.recv(s_packets[i]);
		b.send(r_packets[i]);

		a.recv(r_packets[i]);

		EXPECT_EQ(i, s_packets[i].ReadPOD<int>());
	}
}


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