#pragma once

template <class T>
constexpr bool is_block_message_v = std::is_base_of_v<only_block_message, T>;

template <class F>
decltype(auto) on_read_net_message(const std::vector<std::byte>& bytes, F&& callback) {
	using Id = type_in_list_id<server_message_variant>;

	auto ar = augs::cref_memory_stream(bytes);

	Id id;
	augs::read_bytes(ar, id);

	if (id.get_index() >= Id::max_index_v) {
		throw augs::stream_read_error("message type (%x) is out of range! It should be less than %x.", id.get_index(), Id::max_index_v);
	}

	auto& allocator = yojimbo::GetDefaultAllocator();

	return id.dispatch(
		[&](auto* e) -> decltype(auto) {
			using net_message_type = remove_cptr<decltype(e)>;

			constexpr bool forbidden_message_type = !net_message_type::server_to_client;

			if constexpr(forbidden_message_type) {
				throw augs::stream_read_error("this message type is a client_to_server kind.");
			}
			else {
				net_message_type msg;

				auto release_msg = augs::scope_guard([&msg]() {
					msg.Release();
				});

				if constexpr(is_block_message_v<net_message_type>) {
					const auto pos = ar.get_read_pos();
					const auto block_size = bytes.size() - pos;

					if (block_size > max_block_size_v) {
						throw augs::stream_read_error("block_size (%x) is too big!", block_size);
					}

					const auto block_bytes = reinterpret_cast<uint8_t*>(YOJIMBO_ALLOCATE(allocator, block_size));
					augs::detail::read_raw_bytes(ar, block_bytes, block_size);

					msg.AttachBlock(allocator, block_bytes, block_size);

					return callback(msg);
				}
				else {
					const auto pos = ar.get_read_pos();

					auto stream = yojimbo::ReadStream(allocator, reinterpret_cast<const uint8_t*>(bytes.data() + pos), bytes.size() - pos);
					msg.Serialize(stream);

					return callback(msg);
				}
			}
		}
	);
}

template <class net_message_type>
std::vector<std::byte> net_message_to_bytes(net_message_type& msg) {
	using Id = type_in_list_id<server_message_variant>;
	const auto id = Id::of<net_message_type*>();

	std::vector<std::byte> output;
	auto ar = augs::ref_memory_stream(output);

	augs::write_bytes(ar, id);

	auto& allocator = yojimbo::GetDefaultAllocator();

	if constexpr(is_block_message_v<net_message_type>) {
		const auto block_bytes = reinterpret_cast<const std::byte*>(msg.GetBlockData());
		const auto block_size = msg.GetBlockSize();

		augs::detail::write_raw_bytes(ar, block_bytes, block_size);
	}
	else {
		uint8_t buffer[max_packet_size_v];
		auto stream = yojimbo::WriteStream(allocator, buffer, sizeof(buffer));

		msg.Serialize(stream);
		stream.Flush();

		augs::detail::write_raw_bytes(ar, stream.GetData(), stream.GetBytesProcessed());
	}

	return output;
}
