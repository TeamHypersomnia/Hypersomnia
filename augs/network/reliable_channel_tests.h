
TEST(NetChannelWrapper, SingleTransmissionDeleteAllPending) {
	reliable_channel a, b;
	
	augs::stream msg[15];

	for (int i = 0; i < 15; ++i) {
		augs::write(msg[i], int(i));
		
	}

	/* post four messages */
	a.sender.post_message(msg[0]);
	a.sender.post_message(msg[1]);
	a.sender.post_message(msg[2]);
	a.sender.post_message(msg[3]);

	augs::stream sender_bs;
	augs::stream receiver_bs;

	EXPECT_EQ(false, a.receiver.ack_requested);
	a.build_next_packet(sender_bs);
	EXPECT_EQ(false, a.receiver.ack_requested);

	b.handle_incoming_packet(sender_bs);
	EXPECT_EQ(true, b.receiver.ack_requested);

	b.build_next_packet(receiver_bs);
	EXPECT_EQ(false, b.receiver.ack_requested);

	a.handle_incoming_packet(receiver_bs);

	a.sender.post_message(msg[4]);

	EXPECT_EQ(1, a.sender.reliable_buf.size());
	EXPECT_EQ(1, a.sender.sequence);
	EXPECT_EQ(1, a.sender.most_recent_acked_sequence);

	EXPECT_EQ(1, b.receiver.last_received_sequence);
}

TEST(NetChannelWrapper, PastAcknowledgementDeletesSeveralPending) {
	reliable_channel a, b;

	
	augs::stream msg[15];

	for (int i = 0; i < 15; ++i) {
		augs::write(msg[i], int(i));
		
	}

	augs::stream sender_packets[15];
	augs::stream receiver_packet;

	/* post four messages */
	a.sender.post_message(msg[0]);
	a.sender.post_message(msg[1]);
	a.sender.post_message(msg[2]);
	a.sender.post_message(msg[3]);

	a.build_next_packet(sender_packets[0]);

	a.sender.post_message(msg[4]);
	a.sender.post_message(msg[5]);

	a.build_next_packet(sender_packets[1]);

	a.sender.post_message(msg[6]);
	a.sender.post_message(msg[7]);
	a.sender.post_message(msg[8]);

	a.build_next_packet(sender_packets[2]);

	b.handle_incoming_packet(sender_packets[0]);
	b.build_next_packet(receiver_packet);

	a.handle_incoming_packet(receiver_packet);

	EXPECT_EQ(3, a.sender.sequence);
	EXPECT_EQ(5, a.sender.reliable_buf.size());
	EXPECT_EQ(1, a.sender.most_recent_acked_sequence);

	EXPECT_EQ(1, b.receiver.last_received_sequence);
}

TEST(NetChannelWrapper, FlagForDeletionAndAck) {
	reliable_channel a, b;

	
	augs::stream msg[15];

	for (int i = 0; i < 15; ++i) {
		augs::write(msg[i], int(i));
		
	}

	augs::stream sender_packets[15];
	augs::stream receiver_packet;

	/* post four messages */
	a.sender.post_message(msg[0]);
	a.sender.post_message(msg[1]);
	a.sender.post_message(msg[2]);
	a.sender.post_message(msg[3]);

	a.build_next_packet(sender_packets[0]);

	a.sender.post_message(msg[4]);
	a.sender.post_message(msg[5]);
	a.sender.post_message(msg[6]);

	a.build_next_packet(sender_packets[1]);

	//a.sender.reliable_buf[0].flag_for_deletion = true;
	//a.sender.reliable_buf[2].flag_for_deletion = true;
	//a.sender.reliable_buf[4].flag_for_deletion = true;
	//a.sender.reliable_buf[5].flag_for_deletion = true;
	//a.sender.reliable_buf[6].flag_for_deletion = true;

	a.sender.post_message(msg[7]);
	a.sender.post_message(msg[8]);

	a.build_next_packet(sender_packets[2]);
	a.build_next_packet(sender_packets[3]);
	a.build_next_packet(sender_packets[4]);
	a.build_next_packet(sender_packets[5]);

	b.handle_incoming_packet(sender_packets[0]);
	int table[4];
	augs::read(sender_packets[0], table[0]);
	augs::read(sender_packets[0], table[1]);
	augs::read(sender_packets[0], table[2]);
	augs::read(sender_packets[0], table[3]);

	//EXPECT_EQ(0, table[0]);
	//EXPECT_EQ(1, table[1]);
	//EXPECT_EQ(2, table[2]);
	//EXPECT_EQ(3, table[3]);


	b.build_next_packet(receiver_packet);

	a.handle_incoming_packet(receiver_packet);

	EXPECT_EQ(6, a.sender.sequence);
	EXPECT_EQ(1, a.sender.most_recent_acked_sequence);
	//EXPECT_EQ(2, a.sender.reliable_buf.size());


	//EXPECT_EQ(msg + 7, a.sender.reliable_buf[0].output_bitstream);
	//EXPECT_EQ(msg + 8, a.sender.reliable_buf[1].output_bitstream);

	EXPECT_EQ(1, b.receiver.last_received_sequence);
}