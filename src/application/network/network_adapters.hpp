#pragma once
#include "application/network/network_adapters.h"
#include "augs/enums/callback_result.h"

template <class F>
void server_adapter::process_connections_disconnections(F&& message_callback) {
	for (const auto& p : pending_events) {
		const auto result = message_callback(p.subject_id, p.type);

		ensure(result == callback_result::CONTINUE);
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

					if (result == callback_result::ABORT) {
						j = connection_config.numChannels;
						server.DisconnectClient(i);
						break;
					}

					message = receive_next();
                }
            }
        }
    }

    server.SendPackets();
}

template <class F>
callback_result server_adapter::process_message(const client_id_type id, yojimbo::Message& m, F&& message_callback) {
	using namespace net_messages;

    switch (m.GetType()) {
    case (int)GameMessageType::INITIAL_SOLVABLE:
		return message_callback(id, (initial_solvable&)m);
	case (int)GameMessageType::STEP_ENTROPY:
		return message_callback(id, (step_entropy&)m);

	case (int)GameMessageType::CLIENT_ENTROPY:
		return message_callback(id, (client_entropy&)m);

    default:
		return callback_result::ABORT;
    }
}
