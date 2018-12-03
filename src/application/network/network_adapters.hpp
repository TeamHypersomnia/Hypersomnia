#pragma once
#include "application/network/network_adapters.h"

template <class F>
void server_adapter::advance(const double server_time, F&& message_callback) {
    if (!server.IsRunning()) {
        return;
    }

    // update server and process messages
    server.AdvanceTime(server_time);
    server.ReceivePackets();

    for (int i = 0; i < yojimbo::MaxClients; i++) {
        if (server.IsClientConnected(i)) {
            for (int j = 0; j < connection_config.numChannels; j++) {
				while (yojimbo::Message* message = server.ReceiveMessage(i, j)) {
					process_message(i, *message, std::forward<F>(message_callback));
                    server.ReleaseMessage(i, message);
                    message = server.ReceiveMessage(i, j);
                }
            }
        }
    }

    server.SendPackets();
}

template <class F>
void server_adapter::process_message(const client_id_type id, yojimbo::Message& m, F&& message_callback) {
	using namespace net_messages;

    switch (m.GetType()) {
    case (int)GameMessageType::INITIAL_SOLVABLE:
		message_callback(id, (initial_solvable&)m);
        break;
	case (int)GameMessageType::STEP_ENTROPY:
		message_callback(id, (step_entropy&)m);
		break;

	case (int)GameMessageType::CLIENT_ENTROPY:
		message_callback(id, (client_entropy&)m);
		break;

    default:
        break;
    }
}
