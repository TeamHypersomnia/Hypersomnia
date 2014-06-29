
TEST(NetChannelWrapperWrapper, SingleTransmissionDeleteAllPending) {
	reliable_channel a, b;
	a.enable_starting_byte(135);
	b.enable_starting_byte(135);

	bitstream bs[15];
	reliable_sender::message msg[15];

	for (int i = 0; i < 15; ++i) {
		bs[i].Write(int(i));
		msg[i].output_bitstream = &bs[i];
	}

	/* post four messages */
	a.sender.post_message(msg[0]);
	a.sender.post_message(msg[1]);
	a.sender.post_message(msg[2]);
	a.sender.post_message(msg[3]);

	bitstream sender_bs;
	bitstream receiver_bs;

	EXPECT_EQ(false, a.ack_requested);
	a.send(sender_bs);
	EXPECT_EQ(false, a.ack_requested);

	b.recv(sender_bs);
	EXPECT_EQ(true, b.ack_requested);

	b.send(receiver_bs);
	EXPECT_EQ(false, b.ack_requested);

	a.recv(receiver_bs);

	a.sender.post_message(msg[4]);

	EXPECT_EQ(1, a.sender.reliable_buf.size());
	EXPECT_EQ(1, a.sender.sequence);
	EXPECT_EQ(1, a.sender.ack_sequence);

	EXPECT_EQ(1, b.receiver.last_sequence);
}

TEST(NetChannelWrapper, PastAcknowledgementDeletesSeveralPending) {
	reliable_channel a, b;

	bitstream bs[15];
	reliable_sender::message msg[15];

	for (int i = 0; i < 15; ++i) {
		bs[i].Write(int(i));
		msg[i].output_bitstream = &bs[i];
	}

	bitstream sender_packets[15];
	bitstream receiver_packet;

	/* post four messages */
	a.sender.post_message(msg[0]);
	a.sender.post_message(msg[1]);
	a.sender.post_message(msg[2]);
	a.sender.post_message(msg[3]);

	a.send(sender_packets[0]);

	a.sender.post_message(msg[4]);
	a.sender.post_message(msg[5]);

	a.send(sender_packets[1]);

	a.sender.post_message(msg[6]);
	a.sender.post_message(msg[7]);
	a.sender.post_message(msg[8]);

	a.send(sender_packets[2]);

	b.recv(sender_packets[0]);
	b.send(receiver_packet);

	a.recv(receiver_packet);

	EXPECT_EQ(3, a.sender.sequence);
	EXPECT_EQ(5, a.sender.reliable_buf.size());
	EXPECT_EQ(1, a.sender.ack_sequence);

	EXPECT_EQ(1, b.receiver.last_sequence);
}

TEST(NetChannelWrapper, FlagForDeletionAndAck) {
	reliable_channel a, b;

	bitstream bs[15];
	reliable_sender::message msg[15];

	for (int i = 0; i < 15; ++i) {
		bs[i].Write(int(i));
		msg[i].output_bitstream = &bs[i];
	}

	bitstream sender_packets[15];
	bitstream receiver_packet;

	/* post four messages */
	a.sender.post_message(msg[0]);
	a.sender.post_message(msg[1]);
	a.sender.post_message(msg[2]);
	a.sender.post_message(msg[3]);

	a.send(sender_packets[0]);

	a.sender.post_message(msg[4]);
	a.sender.post_message(msg[5]);
	a.sender.post_message(msg[6]);

	a.send(sender_packets[1]);

	a.sender.reliable_buf[0].flag_for_deletion = true;
	a.sender.reliable_buf[2].flag_for_deletion = true;
	a.sender.reliable_buf[4].flag_for_deletion = true;
	a.sender.reliable_buf[5].flag_for_deletion = true;
	a.sender.reliable_buf[6].flag_for_deletion = true;

	a.sender.post_message(msg[7]);
	a.sender.post_message(msg[8]);

	a.send(sender_packets[2]);
	a.send(sender_packets[3]);
	a.send(sender_packets[4]);
	a.send(sender_packets[5]);

	b.recv(sender_packets[0]);
	int table[4];
	sender_packets[0].Read(table[0]);
	sender_packets[0].Read(table[1]);
	sender_packets[0].Read(table[2]);
	sender_packets[0].Read(table[3]);

	EXPECT_EQ(0, table[0]);
	EXPECT_EQ(1, table[1]);
	EXPECT_EQ(2, table[2]);
	EXPECT_EQ(3, table[3]);


	b.send(receiver_packet);

	a.recv(receiver_packet);

	EXPECT_EQ(6, a.sender.sequence);
	EXPECT_EQ(1, a.sender.ack_sequence);
	EXPECT_EQ(2, a.sender.reliable_buf.size());


	EXPECT_EQ(bs + 7, a.sender.reliable_buf[0].output_bitstream);
	EXPECT_EQ(bs + 8, a.sender.reliable_buf[1].output_bitstream);

	EXPECT_EQ(1, b.receiver.last_sequence);
}