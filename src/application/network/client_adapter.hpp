#pragma once
#include "application/network/network_adapters.hpp"
#include "application/network/client_adapter.h"

template <class H>
void client_adapter::advance(const net_time_t client_time, H&& handler) {
	if (has_connection_failed()) {
        return;
    }

    client.AdvanceTime(client_time);
    client.ReceivePackets();

	if (is_connected()) {
		for (int j = 0; j < connection_config.numChannels; j++) {
			auto receive_next = [&]() {
				return client.ReceiveMessage(j);
			};

			auto message = receive_next();

			while (message) {
				const auto result = process_message(*message, std::forward<H>(handler));
				client.ReleaseMessage(message);

				if (result == message_handler_result::ABORT_AND_DISCONNECT) {
					j = connection_config.numChannels;
					handler.disconnect();
					break;
				}

				message = receive_next();
			}
		}
	}
}

template <class H>
message_handler_result client_adapter::process_message(yojimbo::Message& m, H&& handler) {
	using namespace net_messages;

	const auto type = m.GetType();

	using I = net_messages::id_t;
	using Idx = I::index_type;

	if (static_cast<Idx>(type) >= I::max_index_v) {
		return message_handler_result::ABORT_AND_DISCONNECT;
	}

	I id;
	id.set_index(static_cast<Idx>(type));

	return id.dispatch(
		[&](auto* e) -> message_handler_result {
			using net_message_type = remove_cptr<decltype(e)>;

			constexpr bool forbidden_message_type = !net_message_type::server_to_client;

			if constexpr(forbidden_message_type) {
				handler.set_disconnect_reason("The server has sent an invalid message type.");
				return message_handler_result::ABORT_AND_DISCONNECT;
			}
			else {
				handler.demo_record_server_message(static_cast<net_message_type&>(m));

				auto read_payload_into = [&m](auto&&... args) {
					auto& typed_msg = static_cast<net_message_type&>(m);

					return typed_msg.read_payload(
						std::forward<decltype(args)>(args)...
					);
				};

				using P = payload_of_t<net_message_type>;

				return handler.template handle_payload<remove_cref<P>>(std::move(read_payload_into));
			}
		}
	);
}

template <class net_message_type>
auto client_adapter::create_message() {
	const auto idx = net_messages::id_t::of<net_message_type*>().get_index();
	const auto idx_int = static_cast<int>(idx);

	return static_cast<net_message_type*>(client.CreateMessage(idx_int));
}

template <class... Args>
bool client_adapter::send_payload(
	const game_channel_type& channel_id, 
	Args&&... args
) {
	using net_message_type = std::remove_pointer_t<find_matching_type_in_list<
		detail::strap_can_create<Args&&...>::template can_create,
		net_messages::all_t
	>>;

	constexpr bool is_block_message_v = std::is_base_of_v<yojimbo::BlockMessage, net_message_type>;

	if (auto new_message = create_message<net_message_type>()) {
		auto& m = *new_message;

		auto send_it = [&]() {
			const auto channel_id_int = static_cast<channel_id_type>(channel_id);
			client.SendMessage(channel_id_int, new_message);
		};

		if constexpr (is_block_message_v) {
			uint8_t* allocated_block = nullptr;
			std::size_t allocated_size = 0;

			auto allocate_block = [&](const std::size_t requested_size) {
				allocated_size = requested_size;
				allocated_block = (uint8_t*)YOJIMBO_ALLOCATE(yojimbo::GetDefaultAllocator(), requested_size);;

				return allocated_block;
			};

			const auto translation_result = m.write_payload(
				allocate_block,
				std::forward<Args>(args)...
			);

			if (translation_result && allocated_block != nullptr) {
				new_message->AttachBlock( yojimbo::GetDefaultAllocator(), allocated_block, allocated_size );
				send_it();
				return true;
			}

			YOJIMBO_FREE(yojimbo::GetDefaultAllocator(), new_message);
			return false;
		}
		else {
			const auto translation_result = m.write_payload(
				std::forward<Args>(args)...
			);

			if (translation_result) {
				send_it();
				return true;
			}
		}
	}

	return false;
}

