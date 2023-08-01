#pragma once
#include "application/arena/choose_arena.h"

using initial_snapshot_payload = full_arena_snapshot_payload<false>;

void snap_interpolated_to_logical(cosmos&);

template <class T, class F>
message_handler_result client_setup::handle_payload(
	F&& read_payload
) {
	constexpr auto abort_v = message_handler_result::ABORT_AND_DISCONNECT;
	constexpr auto continue_v = message_handler_result::CONTINUE;
	constexpr bool is_easy_v = payload_easily_movable_v<T>;

	std::conditional_t<is_easy_v, T, std::monostate> payload;

	if constexpr(is_easy_v) {
		if (!read_payload(payload)) {
			return abort_v;
		}
	}

	if constexpr (std::is_same_v<T, server_public_vars>) {
		if (pause_solvable_stream) {
			/* 
				Ignore.
				We will resync everything once we're done anyway.
			*/

			return continue_v;
		}

		const bool are_initial_vars = state == client_state_type::PENDING_WELCOME;
		const auto& new_vars = payload;

		if (are_initial_vars) {
			LOG("Received initial public vars from the server");
			state = client_state_type::RECEIVING_INITIAL_SNAPSHOT;
		}
		else {
			LOG("Received corrected public vars from the server");
		}

		LOG("external_arena_files_provider: %x", new_vars.external_arena_files_provider);

		const auto& new_arena = new_vars.arena;
		const auto& new_mode = new_vars.game_mode;

		LOG_NVPS(new_arena);

		const bool reload_arena = 
			new_arena != sv_public_vars.arena
			|| new_mode != sv_public_vars.game_mode
			|| new_vars.required_arena_hash != sv_public_vars.required_arena_hash
		;

		sv_public_vars = new_vars;

		if (are_initial_vars || reload_arena) {
			if (!try_load_arena_according_to(new_vars, true)) {
				return abort_v;
			}
		}
	}
	else if constexpr (std::is_same_v<T, file_download_payload>) {
		if (is_replaying()) {
			return continue_v;
		}

		if (direct_downloader.has_value()) {
			set_disconnect_reason("The server sent a file payload despite an ongoing download.");
			return abort_v;
		}

		if (!last_requested_direct_file_hash.has_value()) {
			set_disconnect_reason("The server sent a file payload despite no download request.");
			return abort_v;
		}

		direct_downloader = direct_file_download(*last_requested_direct_file_hash, payload.num_file_bytes);

		for (const auto& buffered_chunk : buffered_chunk_packets) {
			if (direct_downloader.has_value()) {
				handle_received(buffered_chunk);
			}
			else {
				break;
			}
		}

		buffered_chunk_packets.clear();

		return continue_v;
	}
	else if constexpr (std::is_same_v<T, file_download_link_payload>) {
		if (is_replaying()) {
			return continue_v;
		}

		/* Unimplemented */

		return abort_v;
	}
	else if constexpr (std::is_same_v<T, server_vars>) {
		client_gui.rcon.on_arrived(payload);
	}
	else if constexpr (std::is_same_v<T, server_runtime_info>) {
		client_gui.rcon.on_arrived(payload);
	}
	else if constexpr (std::is_same_v<T, server_broadcasted_chat>) {
		const auto author_id = payload.author;

		std::string sender_player_nickname;
		auto sender_player_faction = faction_type::SPECTATOR;

		if (payload.recipient_effect == recipient_effect_type::RESUME_RECEIVING_SOLVABLES) {
			/* Has to set it as we have potentially no mode properly setup yet. */
			sender_player_nickname = vars.nickname;
		}

		get_arena_handle(client_arena_type::REFERENTIAL).on_mode(
			[&](const auto& typed_mode) {
				if (auto entry = typed_mode.find(author_id)) {
					sender_player_faction = entry->get_faction();
					sender_player_nickname = entry->get_nickname();
				}
			}
		);

		if (sender_player_nickname.empty()) {
			if (!author_id.is_set()) {
				sender_player_nickname = "Client";
			}
			else {
				sender_player_nickname = "Unknown player";
			}
		}

		{
			auto new_entry = chat_gui_entry::from(
				payload,
				get_current_time(),
				sender_player_nickname,
				sender_player_faction
			);

			LOG(new_entry.operator std::string());
			client_gui.chat.add_entry(std::move(new_entry));
		}

		if (payload.recipient_effect == recipient_effect_type::DISCONNECT) {
			if (payload.target == chat_target_type::SERVER_SHUTTING_DOWN) {
				const auto msg = std::string(payload.message);

				std::string reason_str;

				if (msg.size() > 0) {
					reason_str = "\n\n" + reason_str;
				}

				set_disconnect_reason(typesafe_sprintf(
					"The server is shutting down.%x", 
					reason_str
				), true);
			}
			else {
				std::string kicked_or_banned;

				if (payload.target == chat_target_type::KICK) {
					kicked_or_banned = "kicked";
				}
				else if (payload.target == chat_target_type::BAN) {
					kicked_or_banned = "banned";
				}

				set_disconnect_reason(typesafe_sprintf(
					"You were %x from the server.\nReason: %x", 
					kicked_or_banned,
					std::string(payload.message)
				), true);
			}

			LOG_NVPS(last_disconnect_reason);

			return abort_v;
		}
		else if (payload.recipient_effect == recipient_effect_type::RESUME_RECEIVING_SOLVABLES) {
			pause_solvable_stream = false;

			/*
				Stop sending entropies until the next full arena snapshot,
				as if we just have entered the game.
			*/

			state = client_state_type::RECEIVING_INITIAL_SNAPSHOT;
		}
	}
	else if constexpr (std::is_same_v<T, initial_snapshot_payload>) {
		if (pause_solvable_stream) {
			/* 
				Ignore.
				We will resync everything once we're done anyway.
			*/

			LOG("Ignoring initial_snapshot_payload during download.");
			return continue_v;
		}

		/*
			Note that changing arenas is deterministic,
			so if they're found both on the client and the server,
		   	an arena change will not trigger an initial_snapshot_payload.
		*/

		if (!now_resyncing && state < client_state_type::RECEIVING_INITIAL_SNAPSHOT) {
			set_disconnect_reason(typesafe_sprintf("The server has sent initial state early (state: %x). Disconnecting.", state));
			return abort_v;
		}

		const bool was_resyncing = now_resyncing;

		now_resyncing = false;

		uint32_t read_client_id;

		cosmic::change_solvable_significant(
			scene.world, 
			[&](cosmos_solvable_significant& signi) {
				read_payload(
					buffers,

					clean_round_state,

					initial_snapshot_payload {
						signi,
						current_mode_state,
						read_client_id,
						client_gui.rcon.level
					}
				);

				return changer_callback_result::REFRESH;
			}
		);

		client_player_id = static_cast<mode_player_id>(read_client_id);

		LOG("Received initial state from the server at step: %x.", scene.world.get_timestamp().step);
		LOG("Received client id: %x", client_player_id.value);

		state = client_state_type::IN_GAME;

		auto predicted = get_arena_handle(client_arena_type::PREDICTED);
		const auto referential = get_arena_handle(client_arena_type::REFERENTIAL);

		auto& predicted_cosmos = predicted.advanced_cosm;

		if (was_resyncing) {
			::save_interpolations(receiver.transfer_caches, std::as_const(predicted_cosmos));
		}

		predicted.transfer_all_solvables(referential);

		if (was_resyncing) {
			::restore_interpolations(receiver.transfer_caches, predicted_cosmos);
			receiver.schedule_reprediction = true;
		}

		receiver.clear_incoming();

		if (!was_resyncing) {
			snap_interpolated_to_logical(predicted_cosmos);
		}
	}
#if CONTEXTS_SEPARATE
	else if constexpr (std::is_same_v<T, prestep_client_context>) {
		if (state != client_state_type::IN_GAME) {
			set_disconnect_reason(typesafe_sprintf("The server has sent prestep context too early (state: %x). Disconnecting.", state));
			return abort_v;
		}

		receiver.acquire_next_server_entropy(payload);
	}
#endif
	else if constexpr (std::is_same_v<T, networked_server_step_entropy>) {
		if (pause_solvable_stream) {
			/* 
				Ignore.
				We will resync everything once we're done anyway.
			*/

			LOG("Ignoring networked_server_step_entropy during download.");
			return continue_v;
		}

		if (state != client_state_type::IN_GAME) {
			set_disconnect_reason(typesafe_sprintf("The server has sent entropy too early (state: %x). Disconnecting.", state));
			return abort_v;
		}

		receiver.acquire_next_server_entropy(
			payload.context,
			payload.meta, 
			payload.payload
		);

		const auto& max_commands = vars.max_buffered_server_commands;
		const auto num_commands = receiver.incoming_entropies.size();

		if (!is_replaying()) {
			if (num_commands > max_commands) {
				set_disconnect_reason(typesafe_sprintf(
					"Number of buffered server commands (%x) exceeded max_buffered_server_commands (%x).", 
					num_commands,
					max_commands
				));

				LOG_NVPS(last_disconnect_reason);

				return abort_v;
			}
		}

		//LOG("Received %x th entropy from the server", receiver.incoming_entropies.size());
		//LOG_NVPS(payload.num_entropies_accepted);
	}
	else if constexpr (std::is_same_v<T, public_settings_update>) {
		if (pause_solvable_stream) {
			/* 
				Ignore.
				We will resync everything once we're done anyway.
				Wouldn't hurt to apply it but why bother, just in case.
			*/

			return continue_v;
		}

		/* 
			We can assign it right away and it won't desync,
			because it only affects the incoming entropies and they are unpacked on the go
			whenever networked_server_step_entropy arrives.

			networked_server_step_entropy and public_settings_update are on the same channel.
		*/

		player_metas[payload.subject_id.value].public_settings = payload.new_settings;
	}
	else if constexpr (std::is_same_v<T, net_statistics_update>) {
		const auto& mode_player_stats = payload.stats;

		std::size_t player_i = 0;

		get_arena_handle(client_arena_type::REFERENTIAL).on_mode(
			[&](const auto& typed_mode) {
				typed_mode.for_each_player_id(
					[&](const mode_player_id& id) {
						if (player_i < mode_player_stats.size()) {
							auto& in_stats = mode_player_stats[player_i++];
							auto& out_stats = player_metas[id.value].stats;

							out_stats.ping = in_stats.ping;
							out_stats.download_progress = in_stats.download_progress;

							return callback_result::CONTINUE;
						}

						return callback_result::ABORT;
					}
				);
			}
		);

	}
	else if constexpr (std::is_same_v<T, arena_player_avatar_payload>) {
		session_id_type session_id;
		arena_player_avatar_payload new_avatar;

		const bool result = read_payload(
			session_id,
			new_avatar
		);

		if (!result) {
			return abort_v;
		}

		auto p = untimely_payload { session_id, std::move(new_avatar) };

		if (!push_or_handle(p)) {
			return abort_v;
		}
	}
	else {
		static_assert(always_false_v<T>, "Unhandled payload type.");
	}

	return continue_v;
}
