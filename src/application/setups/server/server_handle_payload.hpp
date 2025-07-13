#pragma once
#include <cstdint>

template <class T, class F>
message_handler_result server_setup::handle_payload(
	const client_id_type& client_id, 
	F&& read_payload
) {
	constexpr auto abort_v = message_handler_result::ABORT_AND_DISCONNECT;
	constexpr auto continue_v = message_handler_result::CONTINUE;
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

	if (c.when_kicked.has_value()) {
		return continue_v;
	}

	ensure(c.is_set());

	if constexpr (std::is_same_v<T, auth_request_payload>) {
		if (c.auth_requested) {
			kick(client_id, "Client requested authentication twice.");

			return abort_v;
		}

		if (!vars.requires_authentication()) {
			return continue_v;
		}

		LOG("auth_request_payload from: %x. Took %x secs.", c.get_nickname(), server_time - c.when_connected);

		c.auth_requested = true;
		c.when_sent_auth_ticket = server_time;
		request_auth(to_mode_player_id(client_id), payload);
	}
	else if constexpr (std::is_same_v<T, requested_client_settings>) {
		/* 
			A client might re-state its requested settings
			even outside of the PENDING_WELCOME state
		*/

		const auto client_chosen_nickname = payload.chosen_nickname;
		auto& new_nick = payload.chosen_nickname;

		LOG("Received requested_client_settings from %x. Client state: %x", new_nick, c.state);

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

		const bool nickname_changed = 
			new_nick != c.settings.chosen_nickname
		;

		if (nickname_changed && find_client_state(new_nick) != nullptr) {
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

		if (!c.settings.chosen_nickname.empty()) {
			if (new_nick != c.settings.chosen_nickname) {
				LOG("Nickname changed: %x to %x", c.settings.chosen_nickname, new_nick);
			}
		}

		c.settings = std::move(payload);

		if (c.state == S::PENDING_WELCOME) {
			LOG("Client %x requested nickname: %x", client_id, c.get_nickname());
			c.state = S::WELCOME_ARRIVED;

#if !PLATFORM_WEB
			if (vars.authenticate_with_nicknames) {
				/*
					Authenticate by originally chosen nickname,
					instead of the apparent one (so after adding potential indices like Player1, Player2)
					in order to actually force people to chose unique nickname
				*/

				LOG(
					"authenticate_with_nicknames is set.\nAuthenticating client with nickname: %x",
					client_chosen_nickname
				);

				push_auth_job(
					to_mode_player_id(client_id),

					[client_chosen_nickname]() {
						return std::string("nick_") + std::string(client_chosen_nickname);
					}
				);
			}
#else
			(void)client_chosen_nickname;
#endif
		}

		c.rebroadcast_synced_meta = true;
		c.last_keyboard_activity_time = server_time;

#if 0
#if !IS_PRODUCTION_BUILD
		/* Test rankeds without real accounts */
		c.verified_has_no_ban = true;
		c.authenticated_id = new_nick;
#endif
#endif
	}
	else if constexpr (is_one_of_v<T, rcon_command_variant, server_vars>) {
		const auto level = get_rcon_level(client_id);

		LOG("Detected rcon level: %x", static_cast<int>(level));

#if IS_PRODUCTION_BUILD
		if (is_ranked_live_or_starting()) {
			LOG("Cannot execute RCON commands during a ranked match.");
			return continue_v;
		}
#endif

		if (level >= rcon_level_type::BASIC) {
			const auto result = [&]() {
				if constexpr(std::is_same_v<T, server_vars>) {
					return handle_rcon_payload(client_id, level, payload);
				}
				else {
					return std::visit(
						[&](const auto& typed_payload) {
							return handle_rcon_payload(client_id, level, typed_payload);
						},
						payload
					);
				}
			}();

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
			server_broadcasted_chat message;

			message.author = to_mode_player_id(client_id);
			message.message = std::string(payload.message);
			message.target = payload.target;

			broadcast(message);

			handle_client_chat_command(client_id, payload);

			c.last_keyboard_activity_time = server_time;
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

		if (c.web_client_paused == client_pause_state::PAUSED) {
			++c.entropies_since_pause;

			bool unpause = false;

			if (server_time - c.when_last_zeroed_entropy_counter > 3.0f) {
				c.when_last_zeroed_entropy_counter = server_time;
				c.entropies_since_pause = 0;
			}

			if (c.entropies_since_pause >= 30) {
				unpause = true;
				LOG("Web client is back (packets resumed at a reasonable rate).");
			}

			if (!payload.empty()) {
				unpause = true;
				LOG("Web client is back (keystrokes resumed).");
			}

			if (unpause) {
				LOG("Re-sending complete solvable state.");

				server->send_payload(
					client_id, 
					game_channel_type::RELIABLE_MESSAGES, 

					special_client_request::UNPAUSE_WEB_CLIENT
				);

				c.web_client_paused = client_pause_state::WAITING_UNPAUSE;
			}
		}

		if (!c.is_web_client_paused()) {
			c.pending_entropies.emplace_back(std::move(payload));
		}
		// LOG("Received %xth command from client. ", c.pending_entropies.size());
	}
	else if constexpr (std::is_same_v<T, special_client_request>) {
		auto wait_download = [&](const auto type) {
			set_client_is_downloading_files(client_id, c, type);

			c.reset_solvable_stream();
			c.last_valid_payload_time = server_time;
			c.last_keyboard_activity_time = server_time;
		};

		switch (payload) {
			case special_client_request::RESET_AFK_TIMER:
				c.last_keyboard_activity_time = server_time;
				break;

			case special_client_request::WAIT_IM_DOWNLOADING_ARENA_EXTERNALLY:
				wait_download(downloading_type::EXTERNALLY);
				break;

			case special_client_request::WAIT_IM_DOWNLOADING_ARENA_DIRECTLY:
				wait_download(downloading_type::DIRECTLY);
				break;

			case special_client_request::RESYNC_ARENA_AFTER_FILES_DOWNLOADED:
				LOG("Client is asking for a resync after download.");
				
				if (c.downloading_status == downloading_type::NONE && !c.lingering_after_arena_reloaded) {
					LOG("Client notified about downloads completion twice.");
					return abort_v;
				}

				{
					server_broadcasted_chat message;
					message.target = chat_target_type::FINISHED_DOWNLOADING;

					message.author = to_mode_player_id(client_id);

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

				c.downloading_status = downloading_type::NONE;
				c.now_downloading_file = std::nullopt;

				/* Prevent kick after the inactivity period */

				c.when_connected = server_time;
				c.last_valid_payload_time = server_time;
				c.last_keyboard_activity_time = server_time;

				/* 
					Resync entire solvable as if the client has just connected. 

					The client's solvable stream is guaranteed to be clean right now (unless they're malicious but it's their loss)

					It's because client makes sure to start sending entropies
					ONLY AFTER RECEIVING the first full state snapshot,
					as if they have just connected for the first time.
				*/

				send_complete_solvable_state_to(client_id);
				reinference_necessary = true;

				break;

			case special_client_request::RESYNC_ARENA:
				if (c.is_web_client_paused()) {
					break;
				}

			case special_client_request::UNPAUSE_WEB_CLIENT:
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

				if (payload == special_client_request::UNPAUSE_WEB_CLIENT) {
					send_complete_solvable_state_to(client_id);
					c.web_client_paused = client_pause_state::LIVE;
				}
				else {
					send_full_arena_snapshot_to(client_id);
				}

				break;

			default: return abort_v;
		}
	}
	else if constexpr (std::is_same_v<T, arena_player_avatar_payload>) {
		{
			mode_player_id dummy_id;
			arena_player_avatar_payload payload;

			if (read_payload(dummy_id, payload)) {
				if (payload.image_bytes.empty()) {
					/* We interpret it as a signal that there won't be any avatar. */
					if (c.should_post_webhooks()) {
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

							if (c.should_post_webhooks()) {
								push_connected_webhook(to_mode_player_id(client_id));
							}

							auto broadcast_avatar = [this, client_id, &client_with_updated_avatar = c](const auto recipient_client_id, auto&) {
								server->send_payload(
									recipient_client_id,
									game_channel_type::RELIABLE_MESSAGES,

									to_mode_player_id(client_id),
									client_with_updated_avatar.meta.avatar
								);
							};

							for_each_id_and_client(broadcast_avatar, only_connected_v);
							rebuild_player_meta_viewables = true;
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
		c.meta.stats.download_progress = payload.progress;
		LOG("Client %x download progress: %x", client_id, float(payload.progress) / 255);
	}
	else if constexpr (std::is_same_v<T, ::file_chunks_request_payload>) {
		if (!c.now_downloading_file.has_value()) {
			return continue_v;
		}

		const auto found_file = mapped_or_nullptr(arena_files_database, *c.now_downloading_file);

		if (found_file == nullptr) {
			return continue_v;
		}

		for (const auto chunk_index : payload.requests) {
			if (c.direct_file_chunks_left == 0) {
				break;
			}

			--c.direct_file_chunks_left;

			send_file_chunk(client_id, *found_file, chunk_index);
		}
	}
	else if constexpr (std::is_same_v<T, ::request_arena_file_download>) {
		if (!vars.allow_direct_arena_file_downloads) {
			kick(client_id, "This server disabled downloading arenas.");
			return continue_v;
		}

		auto kick_file_not_found = [&]() {
			kick(client_id, "Requested file was not found on the server.");
		};

		if (const auto found_file = mapped_or_nullptr(arena_files_database, payload.requested_file_hash)) {
			auto& file_bytes = found_file->cached_file;

			if (file_bytes.empty()) {
				try {
					file_bytes = augs::file_to_bytes(found_file->path);

					const auto requested = payload.requested_file_hash;

					if (const bool asking_current_arena = current_arena_hash == requested) {
						const auto actual = augs::secure_hash(augs::crlf_to_lf_string(file_bytes));

						if (const bool current_arena_is_out_of_date = requested != actual) {
							file_bytes.clear();

							broadcast_info("Files changed on the server. Reloading arena.");
							rechoose_arena();
							rebroadcast_server_public_vars();
						}
					}

					if (!file_bytes.empty()) {
						opened_arena_files.emplace(payload.requested_file_hash);
					}
				}
				catch (...) {
					kick_file_not_found();
					return continue_v;
				}
			}

			if (const bool canceled = file_bytes.empty()) {
				set_client_is_downloading_files(client_id, c, downloading_type::DIRECTLY);

				file_download_payload sent_file_payload;
				sent_file_payload.num_file_bytes = 0;

				server->send_payload(
					client_id, 
					game_channel_type::RELIABLE_MESSAGES, 

					sent_file_payload
				);

				c.lingering_after_arena_reloaded = true;
			}
			else {
				set_client_is_downloading_files(client_id, c, downloading_type::DIRECTLY);
				c.when_last_sent_file_packet = get_current_time();
				c.now_downloading_file = payload.requested_file_hash;
				c.direct_file_chunks_left = 0;

				file_download_payload sent_file_payload;
				sent_file_payload.num_file_bytes = file_bytes.size();

				server->send_payload(
					client_id, 
					game_channel_type::RELIABLE_MESSAGES, 

					sent_file_payload
				);

				const auto max_to_presend = uint16_t(calc_num_chunks_per_tick_per_downloader() * 2);
				const auto num_to_presend = std::min(max_to_presend, payload.num_chunks_to_presend);

				for (file_chunk_index_type chunk_index = 0; chunk_index < num_to_presend; ++chunk_index) {
					if (!send_file_chunk(client_id, *found_file, chunk_index)) {
						break;
					}
				}
			}
		}
		else {
			kick_file_not_found();
		}
	}
	else {
		static_assert(always_false_v<T>, "Unhandled payload type.");
	}

	c.last_valid_payload_time = server_time;
	return continue_v;
}

