
TEST_CASE("NetChannelWrapper SingleTransmissionDeleteAllPending") {
	reliable_channel a, b;
	
	augs::stream msg[15];

	for (int i = 0; i < 15; ++i) {
		augs::write_bytes(msg[i], int(i));
		
	}

	/* post four messages */
	a.sender.post_message(msg[0]);
	a.sender.post_message(msg[1]);
	a.sender.post_message(msg[2]);
	a.sender.post_message(msg[3]);

	augs::stream sender_bs;
	augs::stream receiver_bs;

	REQUIRE(false == a.receiver.ack_requested);
	a.build_next_packet(sender_bs);
	REQUIRE(false == a.receiver.ack_requested);

	b.handle_incoming_packet(sender_bs);
	REQUIRE(true == b.receiver.ack_requested);

	b.build_next_packet(receiver_bs);
	REQUIRE(false == b.receiver.ack_requested);

	a.handle_incoming_packet(receiver_bs);

	a.sender.post_message(msg[4]);

	REQUIRE(1 == a.sender.reliable_buf.size());
	REQUIRE(1 == a.sender.sequence);
	REQUIRE(1 == a.sender.most_recent_acked_sequence);

	REQUIRE(1 == b.receiver.last_received_sequence);
}

TEST_CASE("NetChannelWrapper PastAcknowledgementDeletesSeveralPending") {
	reliable_channel a, b;

	
	augs::stream msg[15];

	for (int i = 0; i < 15; ++i) {
		augs::write_bytes(msg[i], int(i));
		
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

	REQUIRE(3 == a.sender.sequence);
	REQUIRE(5 == a.sender.reliable_buf.size());
	REQUIRE(1 == a.sender.most_recent_acked_sequence);

	REQUIRE(1 == b.receiver.last_received_sequence);
}

TEST_CASE("NetChannelWrapper FlagForDeletionAndAck") {
	reliable_channel a, b;

	
	augs::stream msg[15];

	for (int i = 0; i < 15; ++i) {
		augs::write_bytes(msg[i], int(i));
		
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
	augs::read_bytes(sender_packets[0], table[0]);
	augs::read_bytes(sender_packets[0], table[1]);
	augs::read_bytes(sender_packets[0], table[2]);
	augs::read_bytes(sender_packets[0], table[3]);

	//REQUIRE(0 == table[0]);
	//REQUIRE(1 == table[1]);
	//REQUIRE(2 == table[2]);
	//REQUIRE(3 == table[3]);


	b.build_next_packet(receiver_packet);

	a.handle_incoming_packet(receiver_packet);

	REQUIRE(6 == a.sender.sequence);
	REQUIRE(1 == a.sender.most_recent_acked_sequence);
	//REQUIRE(2 == a.sender.reliable_buf.size());


	//REQUIRE(msg + 7 == a.sender.reliable_buf[0].output_bitstream);
	//REQUIRE(msg + 8 == a.sender.reliable_buf[1].output_bitstream);

	REQUIRE(1 == b.receiver.last_received_sequence);
}