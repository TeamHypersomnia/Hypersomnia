#pragma once
#include "application/network/network_adapters.h"
#include "augs/enums/callback_result.h"
#include "augs/templates/traits/function_traits.h"
#include "augs/templates/filter_types.h"

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
void server_adapter::advance(const double server_time, H&& handler) {
    if (!server.IsRunning()) {
        return;
    }

    // update server and process messages
    server.AdvanceTime(server_time);
    server.ReceivePackets();

	process_connections_disconnections(std::forward<H>(handler));

    for (int i = 0; i < yojimbo::MaxClients; i++) {
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
						j = connection_config.numChannels;
						server.DisconnectClient(i);
						handler.unset_client(i);
						break;
					}

					message = receive_next();
                }
            }
        }
    }
}

template <class T>
using payload_of_t = last_argument_t<decltype(&T::read_payload)>;

template <class H>
message_handler_result server_adapter::process_message(const client_id_type& client_id, yojimbo::Message& m, H&& handler) {
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

			constexpr bool forbidden_message_type = !net_message_type::client_to_server;

			if constexpr(forbidden_message_type) {
				handler.log_malicious_client(client_id);
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

				return handler.template handle_client_message<remove_cref<P>>(client_id, std::move(read_payload_into));
			}
		}
	);
}

template <class net_message_type>
auto server_adapter::create_message(const client_id_type& client_id) {
	const auto idx = net_messages::id_t::of<net_message_type*>().get_index();
	const auto idx_int = static_cast<int>(idx);

	return (net_message_type*)server.CreateMessage(client_id, idx_int);
}

namespace detail {
	template <class... Args>
	struct strap_can_create {
		template <class M, class = void>
		struct write_payload : std::false_type {};

		template <class M>
		struct write_payload<
			M, 
			decltype(std::declval<M&>().write_payload(std::declval<Args>()...), void())
		> : std::true_type {};

		template <class M>
		struct can_create : write_payload<std::remove_pointer_t<M>> {};
	};
}

template <class... Args>
bool server_adapter::send_payload(
	const client_id_type& client_id, 
	const game_channel_type& channel_id, 
	Args&&... args
) {
	using net_message_type = std::remove_pointer_t<find_matching_type_in_list<
		detail::strap_can_create<Args&&...>::template can_create,
		net_messages::all_t
	>>;

	constexpr bool is_block_message_v = std::is_base_of_v<yojimbo::BlockMessage, net_message_type>;

	if (const auto new_message = create_message<net_message_type>(client_id)) {
		auto& m = *new_message;

		const auto translation_result = m.write_payload(
			std::forward<Args>(args)...
		);

		auto send_it = [&]() {
			const auto channel_id_int = static_cast<channel_id_type>(channel_id);
			server.SendMessage(client_id, channel_id_int, new_message);
		};

		if constexpr (is_block_message_v) {
			const auto serialized_bytes = translation_result;

			if (serialized_bytes != nullptr) {
				const auto result_size = serialized_bytes->size();
				const auto memory = get_specific().AllocateBlock(client_id, result_size);

				std::memcpy(memory, serialized_bytes->data(), result_size);
				get_specific().AttachBlockToMessage(client_id, new_message, memory, result_size);

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

