#pragma once
#include "augs/log.h"
#include "application/network/network_adapters.hpp"
#include "application/network/server_adapter.h"

template <class H>
void server_adapter::process_connections_disconnections(H&& handler) {
	for (const auto& p : pending_events) {
		if (p.connected) {
			handler.init_client(p.client_id);
		}
		else {
			handler.unset_client(p.client_id);
		}
	}

	pending_events.clear();
}

template <class H>
void server_adapter::advance(const net_time_t server_time, H&& handler) {
    if (!server.IsRunning()) {
        return;
    }

    server.AdvanceTime(server_time);
    server.ReceivePackets();

	process_connections_disconnections(std::forward<H>(handler));

	for (int i = 0; i < static_cast<int>(max_incoming_connections_v); i++) {
        if (server.IsClientConnected(i)) {
            for (int j = 0; j < connection_config.numChannels; j++) {
				auto receive_next = [&]() {
					return server.ReceiveMessage(i, j);
				};

				auto message = receive_next();

				while (message) {
					const auto result = process_message(i, *message, std::forward<H>(handler));
                    server.ReleaseMessage(i, message);

					if (result == message_handler_result::ABORT_AND_DISCONNECT) {
						LOG("client %x: message_handler_result::ABORT_AND_DISCONNECT", i);

						j = connection_config.numChannels;
						handler.disconnect_and_unset(i);
						break;
					}

					message = receive_next();
                }
            }
        }
    }
}

template <class H>
message_handler_result server_adapter::process_message(const client_id_type& client_id, yojimbo::Message& m, H&& handler) {
	using namespace net_messages;

	const auto type = m.GetType();

	using I = net_messages::id_t;
	using Idx = I::index_type;

	if (static_cast<Idx>(type) >= I::max_index_v) {
		LOG("Invalid message index: %x. Max: %x", type, I::max_index_v);

		handler.log_malicious_client(client_id);
		return message_handler_result::ABORT_AND_DISCONNECT;
	}

	I id;
	id.set_index(static_cast<Idx>(type));

	return id.dispatch(
		[&](auto* e) -> message_handler_result {
			using net_message_type = remove_cptr<decltype(e)>;

			constexpr bool forbidden_message_type = !net_message_type::client_to_server;

			if constexpr(forbidden_message_type) {
				LOG("Client has sent forbidden message type: %x", type);

				handler.log_malicious_client(client_id);
				return message_handler_result::ABORT_AND_DISCONNECT;
			}
			else {
				auto read_payload_into = [&m](auto&&... args) {
					auto& typed_msg = static_cast<net_message_type&>(m);

					return typed_msg.read_payload(
						std::forward<decltype(args)>(args)...
					);
				};

				using P = payload_of_t<net_message_type>;

				return handler.template handle_payload<remove_cref<P>>(client_id, std::move(read_payload_into));
			}
		}
	);
}

template <class net_message_type>
auto server_adapter::create_message(const client_id_type& client_id) {
	const auto idx = net_messages::id_t::of<net_message_type*>().get_index();
	const auto idx_int = static_cast<int>(idx);

	return static_cast<net_message_type*>(server.CreateMessage(client_id, idx_int));
}

template <class... Args>
translated_payload_id server_adapter::translate_payload(
	const client_id_type& client_id, 
	Args&&... args
) {
	using net_message_type = std::remove_pointer_t<find_matching_type_in_list<
		detail::strap_can_create<Args&&...>::template can_create,
		net_messages::all_t
	>>;

	constexpr bool is_block_message_v = std::is_base_of_v<yojimbo::BlockMessage, net_message_type>;

	if (auto new_message = create_message<net_message_type>(client_id)) {
		auto& m = *new_message;

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
				return new_message;
			}

			YOJIMBO_FREE(yojimbo::GetDefaultAllocator(), new_message);
		}
		else {
			const auto translation_result = m.write_payload(
				std::forward<Args>(args)...
			);

			if (translation_result) {
				return new_message;
			}
		}
	}

	return nullptr;
}

template <class... Args>
bool server_adapter::send_payload(
	const client_id_type& client_id, 
	const game_channel_type& channel_id, 
	Args&&... args
) {
	return send(client_id, channel_id, translate_payload(client_id, std::forward<Args>(args)...));
}

