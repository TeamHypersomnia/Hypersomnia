#pragma once

/*
	A tournament's match that finished - by its summary, by the round's timeout, or an open match closed - stays up
	empty instead of shutting down: the browsers keep showing its tournament with all its ports (and the first one,
	which a favorite tournament is), and the final stays until the host stops the tournament.
*/

bool server_setup::is_tournament_match() const {
	return instance_type == server_instance_type::TOURNAMENT && vars.tournament_stage >= 0;
}

void server_setup::enter_tournament_finished() {
	if (!is_tournament_match()) {
		schedule_shutdown();
		return;
	}

	if (tournament_finished) {
		return;
	}

	LOG("Tournament match finished - the server stays up empty, refusing everyone, until its round ends.");

	tournament_finished = true;

	/*
		An open match played to its end is closed too: its players leaving now aren't leaving the round - their places,
		the round's count of who came and the match's name stay.
	*/

	open_match_closed = true;

	request_immediate_heartbeat();
}

/*
	Whoever stayed (came during the summary, an admin, a lost message) and whoever comes: a plain disconnect, told why -
	the players of the match leave by themselves after its summary (client_setup.h), with the same text.
*/

void server_setup::disconnect_everyone_from_finished() {
	if (!tournament_finished) {
		return;
	}

	for_each_id_and_client([&](const auto client_id, auto& c) {
		if (c.when_kicked.has_value()) {
			return;
		}

		c.when_kicked = server_time;
		c.kick_no_linger = true;

		server_broadcasted_chat message;
		message.message = ::tournament_finished_text(vars.tournament_stage);
		message.target = chat_target_type::INFO;
		message.recipient_effect = recipient_effect_type::DISCONNECT;

		server->send_payload(
			client_id,
			game_channel_type::RELIABLE_MESSAGES,

			message
		);
	}, only_connected_v);
}
