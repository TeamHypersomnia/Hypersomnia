#pragma once

template <class T, class F>
message_handler_result server_setup::handle_payload(
	const client_id_type& client_id, 
	F&& read_payload
) {
	constexpr auto abort_v = message_handler_result::ABORT_AND_DISCONNECT;
	constexpr bool is_easy_v = payload_easily_movable_v<T>;

	std::conditional_t<is_easy_v, T, std::monostate> payload;

	if constexpr(is_easy_v) {
		if (!read_payload(payload)) {
			LOG("Failed to read payload from the client. Disconnecting.");
			return abort_v;
		}
	}

	using S = client_state_type;
	namespace N = net_messages;

	auto& c = clients[client_id];
	ensure(c.is_set());

	if constexpr (std::is_same_v<T, requested_client_settings>) {
		/* 
			A client might re-state its requested settings
			even outside of the PENDING_WELCOME state
		*/

		auto& new_nick = payload.chosen_nickname;

		if (!is_nickname_valid_characters(new_nick)) {
			const auto reason = typesafe_sprintf(
				"Your nickname has invalid characters.\nPlease choose a different one.",
				new_nick
			);

			kick(client_id, reason);
			return abort_v;
		}

		if (!nickname_len_in_range(new_nick.length())) {
			new_nick = "Player";
		}

		if (find_client_state(new_nick) != nullptr) {
			{
				auto candidate = std::string(new_nick);

				for (int i = 1; i < 100; ++i) {
					const auto new_index = std::to_string(i);

					if (candidate.length() + new_index.length() > max_nickname_length_v) {
						if (candidate.size() > 0) {
							candidate.pop_back();
						}
					}

					const auto candidate_nick = candidate + new_index;

					if (find_client_state(candidate_nick) == nullptr) {
						new_nick = candidate_nick;
						break;
					}
				}
			}

			if (!nickname_len_in_range(new_nick.length()) || find_client_state(new_nick) != nullptr) {
				const auto reason = typesafe_sprintf(
					"Nickname: '%x' was already taken.\nPlease choose a different one.",
					new_nick
				);

				kick(client_id, reason);
				return abort_v;
			}
		}

		c.settings = std::move(payload);

		if (c.state == S::PENDING_WELCOME) {
			LOG("Client %x requested nickname: %x", client_id, c.get_nickname());
			c.state = S::WELCOME_ARRIVED;
		}

		c.rebroadcast_public_settings = true;
		c.last_keyboard_activity_time = server_time;
	}
	else if constexpr (std::is_same_v<T, rcon_command_variant>) {
		const auto level = get_rcon_level(client_id);

		LOG("Detected rcon level: %x", static_cast<int>(level));

		if (level >= rcon_level_type::BASIC) {
			const auto result = std::visit(
				[&](const auto& typed_payload) {
					return handle_rcon_payload(level, typed_payload);
				},
				payload
			);

			if (result == abort_v) {
				return result;
			}

			c.last_keyboard_activity_time = server_time;
		}
		else {
			++c.unauthorized_rcon_commands;

			if (c.unauthorized_rcon_commands > vars.max_unauthorized_rcon_commands) {
				LOG("unauthorized_rcon_commands exceeded max_unauthorized_rcon_commands (%x). Kicking client.", vars.max_unauthorized_rcon_commands);

				return abort_v;
			}
		}
	}
	else if constexpr (std::is_same_v<T, client_requested_chat>) {
		if (c.state == S::IN_GAME) {
			if (const auto session_id = find_session_id(client_id)) {
				server_broadcasted_chat message;

				message.author = *session_id;
				message.message = std::string(payload.message);
				message.target = payload.target;

				broadcast(message);

				c.last_keyboard_activity_time = server_time;
			}
		}
	}
	else if constexpr (std::is_same_v<T, total_mode_player_entropy>) {
		if (c.state == S::RECEIVING_INITIAL_SNAPSHOT) {
			c.set_in_game(server_time);
		}

		if (c.state != S::IN_GAME) {
			/* 
				The client shall not send commands until it receives the first step entropy,
				at which point being IN_GAME is guaranteed.

				Actually, wouldn't the ack for the last fragment come along the first client commands?
			*/

			LOG("Client has sent its command too early (state: %x). Disconnecting.", c.state);

			return abort_v;
		}

		if (!payload.empty()) {
			c.last_keyboard_activity_time = server_time;
		}

		c.pending_entropies.emplace_back(std::move(payload));
		//LOG("Received %x th command from client. ", c.pending_entropies.size());
	}
	else if constexpr (std::is_same_v<T, special_client_request>) {
		switch (payload) {
			case special_client_request::RESET_AFK_TIMER:
				c.last_keyboard_activity_time = server_time;
				break;

			case special_client_request::WAIT_IM_DOWNLOADING_ARENA:
				set_client_is_downloading_files(client_id, c);

				c.last_valid_payload_time = server_time;
				c.last_keyboard_activity_time = server_time;
				break;

			case special_client_request::RESYNC_ARENA_AFTER_FILES_DOWNLOADED:
				LOG("Client is asking for a resync after download.");
				
				if (!c.is_downloading_files) {
					LOG("Client notified about downloads completion twice.");
					return abort_v;
				}

				{
					server_broadcasted_chat message;
					message.target = chat_target_type::FINISHED_DOWNLOADING;

					if (const auto session_id = find_session_id(client_id)) {
						message.author = *session_id;
					}

					{
						const auto except = client_id;
						broadcast(message, except);
					}

					message.recipient_effect = recipient_effect_type::RESUME_RECEIVING_SOLVABLES;

					server->send_payload(
						client_id,
						game_channel_type::RELIABLE_MESSAGES,
						message
					);
				}

				c.is_downloading_files = false;
				c.is_downloading_files_directly = false;

				/* Prevent kick after the inactivity period */

				c.last_valid_payload_time = server_time;
				c.last_keyboard_activity_time = server_time;

				/* 
					Resync entire solvable as if the client has just connected. 
				*/

				c.reset_solvable_stream();
				send_complete_solvable_state_to(client_id);
				reinference_necessary = true;

				break;

			case special_client_request::RESYNC_ARENA:
				if (server_time >= c.last_resync_counter_reset_at + vars.reset_resync_timer_once_every_secs) {
					c.resyncs_counter = 0;
					LOG("Resetting the resync counter.");
				}

				c.last_resync_counter_reset_at = server_time;
				++c.resyncs_counter;

				LOG("Client has asked for a resync no %x.", c.resyncs_counter);

				if (c.resyncs_counter > vars.max_client_resyncs) {
					LOG("Client is asking for a resync too often! Kicking.");
					return abort_v;
				}

				send_full_arena_snapshot_to(client_id);
				reinference_necessary = true;

				break;

			default: return abort_v;
		}
	}
	else if constexpr (std::is_same_v<T, arena_player_avatar_payload>) {
		{
			session_id_type dummy_id;
			arena_player_avatar_payload payload;

			if (read_payload(dummy_id, payload)) {
				if (payload.image_bytes.empty()) {
					/* We interpret it as a signal that there won't be any avatar. */
					if (!c.settings.suppress_webhooks) {
						push_connected_webhook(to_mode_player_id(client_id));
					}
				}
				else {
					try {
						const auto size = augs::image::get_size(payload.image_bytes);

						if (size.x > max_avatar_side_v || size.y > max_avatar_side_v) {
							const auto disconnect_reason = typesafe_sprintf("sending an avatar of size %xx%x", size.x, size.y);
							kick(client_id, disconnect_reason);
						}
						else {
							c.meta.avatar = std::move(payload);

							if (!c.settings.suppress_webhooks) {
								push_connected_webhook(to_mode_player_id(client_id));
							}

							if (const auto session_id_of_avatar = find_session_id(client_id)) {
								auto broadcast_avatar = [this, session_id_of_avatar, &client_with_updated_avatar = c](const auto recipient_client_id, auto&) {
									server->send_payload(
										recipient_client_id,
										game_channel_type::RELIABLE_MESSAGES,

										*session_id_of_avatar,
										client_with_updated_avatar.meta.avatar
									);
								};

								for_each_id_and_client(broadcast_avatar, only_connected_v);
								rebuild_player_meta_viewables = true;
							}
							else {
								kick(client_id, "sending an avatar too early");
							}
						}
					}
					catch (...) {
						kick(client_id, "sending an invalid avatar");
					}
				}
			}
			else {
				kick(client_id, "sending an invalid avatar");
			}
		}
	}
	else if constexpr (std::is_same_v<T, ::download_progress_message>) {

	}
	else if constexpr (std::is_same_v<T, ::request_arena_file_download>) {
		if (!vars.allow_direct_arena_file_downloads) {
			kick(client_id, "This server disabled downloading arenas.");
			return abort_v;
		}

		auto kick_file_not_found = [&]() {
			kick(client_id, "Requested file was not found on the server.");
			return abort_v;
		};

		/*
		   	TODO: Check if the previous file transfer is complete, for security.
			Should probably check if the channel is empty, and if it's not, just kick.
		*/

		if (const auto found_file_path = mapped_or_nullptr(arena_files_database, payload.requested_file_hash)) {
			file_download_payload sent_file_payload;

			try {
				if (found_file_path->extension() == ".json") {
					const auto str = augs::file_to_string_crlf_to_lf(*found_file_path);

					sent_file_payload.file_bytes.assign(
						reinterpret_cast<const std::byte*>(str.data()),
						reinterpret_cast<const std::byte*>(str.data() + str.size())
					);
				}
				else {
					sent_file_payload.file_bytes = augs::file_to_bytes(*found_file_path);
				}
			}
			catch (...) {
				return kick_file_not_found();
			}

			set_client_is_downloading_files(client_id, c);
			c.is_downloading_files_directly = true;
			c.when_last_sent_file_packet = get_current_time();

			server->send_payload(
				client_id, 
				game_channel_type::RELIABLE_MESSAGES, 

				sent_file_payload
			);
		}
		else {
			return kick_file_not_found();
		}
	}
	else {
		static_assert(always_false_v<T>, "Unhandled payload type.");
	}

	c.last_valid_payload_time = server_time;
	return message_handler_result::CONTINUE;
}

