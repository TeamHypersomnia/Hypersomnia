#pragma once
#include "application/network/network_adapters.h"
#include "augs/enums/callback_result.h"
#include "augs/templates/traits/function_traits.h"

template <class F>
void server_adapter::process_connections_disconnections(F&& message_callback) {
	for (const auto& p : pending_events) {
		const auto result = message_callback(p.subject_id, p.type);

		ensure(result == message_handler_result::CONTINUE);
		(void)result;
	}

	pending_events.clear();
}


template <class F>
void server_adapter::advance(const double server_time, F&& message_callback) {
    if (!server.IsRunning()) {
        return;
    }

    // update server and process messages
    server.AdvanceTime(server_time);
    server.ReceivePackets();

	process_connections_disconnections(std::forward<F>(message_callback));

    for (int i = 0; i < yojimbo::MaxClients; i++) {
        if (server.IsClientConnected(i)) {
            for (int j = 0; j < connection_config.numChannels; j++) {
				auto receive_next = [&]() {
					return server.ReceiveMessage(i, j);
				};

				auto message = receive_next();

				while (message) {
					const auto result = process_message(i, *message, std::forward<F>(message_callback));
                    server.ReleaseMessage(i, message);

					if (result == message_handler_result::ABORT_AND_DISCONNECT) {
						j = connection_config.numChannels;
						server.DisconnectClient(i);
						break;
					}

					message = receive_next();
                }
            }
        }
    }
}

template <class F>
message_handler_result server_adapter::process_message(const client_id_type& client_id, yojimbo::Message& m, F&& message_callback) {
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
			using E = remove_cptr<decltype(e)>;

			return message_callback(client_id, (E&)m);
		}
	);
}

template <class T>
void server_adapter::send_message(const client_id_type& client_id, const game_channel_type& channel_id, T&& message_setter) {
	using message_type = remove_cref<argument_t<remove_cref<T>, 0>>;
	const auto idx = net_messages::id_t::of<message_type*>().get_index();

	const auto idx_int = static_cast<int>(idx);
	const auto channel_id_int = static_cast<channel_id_type>(channel_id);

	if (const auto new_message = (message_type*)server.CreateMessage(client_id, idx_int)) {
		auto& m = *new_message;
		message_setter(m);
		server.SendMessage(client_id, channel_id_int, std::addressof(m));
	}
}
