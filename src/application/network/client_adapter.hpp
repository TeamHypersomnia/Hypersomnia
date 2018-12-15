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
					disconnect();
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
				handler.log_malicious_server();
				return message_handler_result::ABORT_AND_DISCONNECT;
			}
			else {
				auto read_payload_into = [&m](auto&&... args) {
					auto& typed_msg = (net_message_type&)m;

					return typed_msg.read_payload(
						std::forward<decltype(args)>(args)...
					);
				};

				using P = payload_of_t<net_message_type>;

				return handler.template handle_server_message<remove_cref<P>>(std::move(read_payload_into));
			}
		}
	);
}

template <class net_message_type>
auto client_adapter::create_message() {
	const auto idx = net_messages::id_t::of<net_message_type*>().get_index();
	const auto idx_int = static_cast<int>(idx);

	return (net_message_type*)client.CreateMessage(idx_int);
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

	if (const auto new_message = create_message<net_message_type>()) {
		auto& m = *new_message;

		const auto translation_result = m.write_payload(
			std::forward<Args>(args)...
		);

		auto send_it = [&]() {
			const auto channel_id_int = static_cast<channel_id_type>(channel_id);
			client.SendMessage(channel_id_int, new_message);
		};

		if constexpr (is_block_message_v) {
			const auto serialized_bytes = translation_result;

			if (serialized_bytes != nullptr) {
				const auto result_size = serialized_bytes->size();
				const auto memory = get_specific().AllocateBlock(result_size);

				std::memcpy(memory, serialized_bytes->data(), result_size);
				get_specific().AttachBlockToMessage(new_message, memory, result_size);

				send_it();

				return true;
			}

			return false;
		}
		else {
			send_it();

			return translation_result;
		}
	}

	return false;
}

