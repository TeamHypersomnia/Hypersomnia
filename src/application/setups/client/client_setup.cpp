#include "augs/misc/pool/pool_io.hpp"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/client/client_setup.h"
#include "application/config_lua_table.h"

#include "application/network/client_adapter.hpp"
#include "application/network/net_message_translation.h"
#include "application/setups/client/demo_step.h"
#include "application/network/net_message_readwrite.h"
#include "augs/templates/thread_templates.h"

#include "game/cosmos/change_solvable_significant.h"

#include "augs/readwrite/memory_stream.h"

#include "augs/filesystem/file.h"

#include "augs/gui/text/printer.h"
#include "application/network/payload_easily_movable.h"
#include "augs/misc/readable_bytesize.h"
#include "game/cosmos/for_each_entity.h"
#include "game/cosmos/entity_type_traits.h"

#include "application/gui/config_nvp.h"

#include "application/setups/client/rcon_pane.h"
#include "application/gui/pretty_tabs.h"

#include "application/gui/client/rcon_gui.hpp"
#include "application/arena/arena_handle.hpp"
#include "application/setups/client/demo_paths.h"
#include "augs/misc/time_utils.h"
#include "application/network/net_serialize.h"
#include "augs/readwrite/byte_file.h"
#include "application/gui/client/demo_player_gui.hpp"

#include "application/setups/client/client_handle_payload.hpp"
#include "application/network/resolve_address.h"
#include "augs/network/netcode_sockets.h"
#include "application/network/resolve_address_result.h"
#include "augs/network/netcode_utils.h"

#include "application/setups/editor/packaged_official_content.h"
#include "augs/filesystem/directory.h"
#include "augs/readwrite/json_readwrite_errors.h"
#include "application/setups/client/https_file_downloader.h"
#include "application/setups/server/file_chunk_packet.h"
#include "application/setups/client/direct_file_download.hpp"

void client_demo_player::play_demo_from(const augs::path_type& p) {
	source_path = p;
	auto source = augs::open_binary_input_stream(source_path);

	augs::read_bytes(source, meta);
	augs::read_vector_until_eof(source, demo_steps);

	gui.open();
}

bool client_demo_player::control(const handle_input_before_game_input in) {
	using namespace augs::event;
	using namespace augs::event::keys;

	const auto& state = in.common_input_state;
	const bool has_alt{ state[key::LALT] };

	const auto ch = in.e.get_key_change();

	if (ch == key_change::PRESSED) {
		const auto key = in.e.get_key();

		if (in.e.was_any_key_pressed()) {
			switch (key) {
				case key::NUMPAD0: set_speed(1.0); return true;
				case key::NUMPAD1: set_speed(0.01); return true;
				case key::NUMPAD2: set_speed(0.05); return true;
				case key::NUMPAD3: set_speed(0.1); return true;
				case key::NUMPAD4: set_speed(0.5); return true;
				case key::NUMPAD5: set_speed(2.0); return true;
				case key::NUMPAD6: set_speed(4.0); return true;
				case key::NUMPAD7: set_speed(10.0); return true;
				case key::SPACE: paused = !paused; return true;
				case key::L: paused = false; return true;
				case key::ADD: seek_forward(1); return true;
				case key::SUBTRACT: seek_backward(1); return true;
				default: break;
			}

			if (has_alt) {
				switch (key) {
					case key::P: gui.show = !gui.show; return true;
					default: break;
				}
			}
		}

	}

	return false;
}

void client_setup::demo_replay_server_messages_from(const demo_step& step) {
	for (std::vector<std::byte>& serialized_bytes : step.serialized_messages) {
		auto replay_message = [this](auto& typed_msg) -> message_handler_result {
			using net_message_type = remove_cref<decltype(typed_msg)>;

			auto read_payload_into = [&](auto&&... args) {
				return typed_msg.read_payload(
					std::forward<decltype(args)>(args)...
				);
			};

			using P = payload_of_t<net_message_type>;

			return handle_payload<remove_cref<P>>(std::move(read_payload_into));
		};

		try {
			const auto result = ::replay_serialized_net_message(serialized_bytes, replay_message);

			if (result == message_handler_result::ABORT_AND_DISCONNECT) {
				disconnect();
				return;
			}
		}
		catch (const augs::stream_read_error& err) {
			set_demo_failed_reason(err.what());
			disconnect();
			break;
		}
	}
}

template <class T>
void client_setup::demo_record_server_message(T& message) {
	if (is_recording()) {
		auto bytes = ::net_message_to_bytes(message);
		get_currently_recorded_step().serialized_messages.emplace_back(std::move(bytes));
	}
}

void client_setup::play_demo_from(const augs::path_type& p) {
	demo_player.play_demo_from(p);
}

void client_setup::flush_demo_steps() {
	if (unflushed_demo_steps.empty()) {
		return;
	}

	when_last_flushed_demo = client_time;

	std::swap(demo_steps_being_flushed, unflushed_demo_steps);

	future_flushed_demo = launch_async(
		[&]() {
			auto out = augs::with_exceptions<std::ofstream>();
			out.open(recorded_demo_path, std::ios::out | std::ios::binary | std::ios::app);

			if (!was_demo_meta_written) {
				demo_file_meta meta;
				meta.server_name = displayed_connecting_server_name;
				meta.server_address = last_addr.address;
				meta.version = hypersomnia_version();
				augs::write_bytes(out, meta);

				const auto version_info_path = augs::path_type(recorded_demo_path).replace_extension(".version.txt");
				augs::save_as_text(version_info_path, meta.version.get_summary());

				was_demo_meta_written = true;
			}

			for (const auto& s : demo_steps_being_flushed) {
				augs::write_bytes(out, s);
			}

			out.flush();
			demo_steps_being_flushed.clear();
		}
	);
}

bool client_setup::is_replaying() const {
	return !demo_player.source_path.empty();
}

bool client_setup::is_paused() const {
	return demo_player.is_paused();
}

bool client_setup::demo_flushing_finished() const {
	return !future_flushed_demo.valid();
}

bool client_setup::is_recording() const {
	return !recorded_demo_path.empty();
}

demo_step& client_setup::get_currently_recorded_step() {
	return unflushed_demo_steps.back();
}

void client_setup::record_demo_to(const augs::path_type& p) {
	recorded_demo_path = p;
	when_last_flushed_demo = client_time;
}

template <class... Args>
bool client_setup::send_payload(Args&&... args) {
	if (is_replaying()) {
		return true;
	}

	return adapter->send_payload(std::forward<Args>(args)...);
}

void client_setup::handle_received(const file_chunk_packet& chunk) {
	ensure(direct_downloader.has_value());

	uint32_t data_received = 0;

	if (const auto complete_file = direct_downloader->advance(chunk, data_received); complete_file.has_value()) {
		direct_downloader = std::nullopt;
		last_requested_direct_file_hash = std::nullopt;

		advance_downloading_session(augs::make_ptr_read_stream(*complete_file));
	}

	if (data_received != 0) {
		direct_bandwidth.newDataReceived(data_received);
	}
}

bool client_setup::handle_auxiliary_command(std::byte* const bytes, const int n) {
	if (n != sizeof(file_chunk_packet)) {
		return false;
	}
	
	const file_chunk_packet& chunk = *reinterpret_cast<file_chunk_packet*>(bytes);

	if (!chunk.header_valid()) {
		return false;
	}

	if (!direct_downloader.has_value()) {
		if (last_requested_direct_file_hash == chunk.file_hash) {
			buffered_chunk_packets.push_back(chunk);
			return true;
		}

		return false;
	}

	handle_received(chunk);

	return true;
}

client_setup::client_setup(
	sol::state& lua,
	const packaged_official_content& official,
	const client_start_input& in,
	const client_vars& initial_vars,
	const nat_detection_settings& nat_detection,
	port_type preferred_binding_port
) : 
	lua(lua),
	official(official),
	last_addr(in.get_address_and_port()),
	displayed_connecting_server_name(in.displayed_connecting_server_name),
	vars(initial_vars),
	adapter(std::make_unique<client_adapter>(preferred_binding_port, [this](std::byte* bytes, std::size_t n) {
		return handle_auxiliary_command(bytes, n);
	})),
	client_time(get_current_time()),
	when_initiated_connection(get_current_time())
{
	(void)nat_detection;

	LOG("Initializing connection with %x", last_addr.address);

	const auto& input_demo_path = in.replay_demo;

	if (!input_demo_path.empty() && in.chosen_address_type == connect_address_type::REPLAY) {
		const auto error = typesafe_sprintf(
			"Failed to open demo file:\n%x", 
			input_demo_path
		);

		try {
			play_demo_from(input_demo_path);
		}
		catch (const augs::file_open_error& err) {
			set_disconnect_reason(error + "\n" + err.what(), true);
			disconnect();
		}
		catch (const augs::stream_read_error& err) {
			set_disconnect_reason(error + "\n" + err.what(), true);
			disconnect();
		}
	}
	else {
		if (!nickname_len_in_range(vars.nickname.length())) {
			const auto reason = typesafe_sprintf(
				"The nickname should be between %x and %x bytes.", 
				min_nickname_length_v,
				max_nickname_length_v
			);

			set_disconnect_reason(reason);
		}
		else if (!is_nickname_valid_characters(vars.nickname)) {
			const auto reason = typesafe_sprintf(
				"The nickname '%x' has invalid characters.", 
				vars.nickname
			);

			set_disconnect_reason(reason);
		}
		else {
			augs::network::enable_detailed_logs(true);

			const auto resolution = adapter->connect(last_addr);

			if (resolution.result == resolve_result_type::COULDNT_RESOLVE_HOST) {
				const auto reason = typesafe_sprintf(
					"Couldn't resolve host: %x", 
					resolution.host
				);

				set_disconnect_reason(reason);
			}
			else if (resolution.result == resolve_result_type::INVALID_ADDRESS) {
				const auto reason = typesafe_sprintf(
					"The address: \"%x\" is invalid!", last_addr.address
				);

				set_disconnect_reason(reason);
			}
			else {
				resolved_server_address = resolution.addr;

				ensure_eq(resolution.result, resolve_result_type::OK);

				const auto new_demo_fname = augs::date_time().get_readable_for_file() + ".dem";
				const auto new_demo_path = augs::path_type(DEMOS_DIR) / new_demo_fname;

				record_demo_to(new_demo_path);
			}
		}
	}
}

client_setup::~client_setup() {
	LOG("Client setup dtor");
	disconnect();

	augs::network::enable_detailed_logs(false);

	wait_for_demo_flush();
	flush_demo_steps();
	wait_for_demo_flush();
}

void client_setup::request_direct_file_download(const augs::secure_hash_type& hash) {
	request_arena_file_download request;
	request.requested_file_hash = hash;
	/* Send a burst for the first time */
	request.num_chunks_to_presend = calc_num_chunks_per_tick() * 2;
	num_skip_chunks = request.num_chunks_to_presend;
	buffered_chunk_packets.clear();

	last_requested_direct_file_hash = hash;

	send_payload(
		game_channel_type::RELIABLE_MESSAGES,
		request
	);
}

bool client_setup::setup_external_arena_download_session() {
	if (sv_public_vars.external_arena_files_provider.empty()) {
		LOG("No external link was provided for the arena files.");
		return false;
	}

	if (const auto parsed = parsed_url(sv_public_vars.external_arena_files_provider); parsed.valid()) {
		LOG("External arena files provider: %x", sv_public_vars.external_arena_files_provider);

		external_downloader = std::make_unique<https_file_downloader>(parsed);

		auto external_file_requester = [this](const augs::secure_hash_type&, const augs::path_type& path) {
			const auto location = typesafe_sprintf("%x/%x", last_download_request.arena_name, path.string());
			this->external_downloader->download_file(location);
		};

		downloading = arena_downloading_session(
			last_download_request.arena_name,
			last_download_request.project_hash,
			external_file_requester
		);

		return true;
	}
	else {
		LOG("Couldn't parse %x: ", sv_public_vars.external_arena_files_provider);
	}

	return false;
}

void client_setup::setup_direct_arena_download_session() {
	LOG("Requesting direct arena download over UDP.");

	external_downloader = nullptr;

	auto direct_file_requester = [this](const augs::secure_hash_type& hash, const augs::path_type& path) {
		LOG("Requesting direct download over UDP: %x (hash: %x)", path, hash);
		this->request_direct_file_download(hash);
	};

	downloading = arena_downloading_session(
		last_download_request.arena_name,
		last_download_request.project_hash,
		direct_file_requester
	);
}

bool client_setup::start_downloading_session() {
	if (downloading->in_progress()) {
		send_payload(
			game_channel_type::RELIABLE_MESSAGES,
			is_trying_external_download() ? 
			special_client_request::WAIT_IM_DOWNLOADING_ARENA_EXTERNALLY : 
			special_client_request::WAIT_IM_DOWNLOADING_ARENA_DIRECTLY
		);

		/*
			As soon as pause_solvable_stream == true,
			entropies stop being unpacked altogether (in_game becomes false),
			they stop being sent,
			and no incoming entropies are handled from the server.

			The receiver in its clean state will be untouched until after these three are completed:
			- the download has completed and RESYNC_ARENA_AFTER_FILES_DOWNLOADED is sent,
			- the server responds with RESUME_RECEIVING_SOLVABLES, which makes the client go into RECEIVING_INITIAL_SNAPSHOT state.
			- and THEN the server sends the full arena snapshot.

			Sending entropies will resume as well due to state changing from RECEIVING_INITIAL_SNAPSHOT to IN_GAME.

			The server is instructed to clear the client's pending entropies/accepted entropy counter
			on receiving RESYNC_ARENA_AFTER_FILES_DOWNLOADED.
		*/

		LOG("Clear simulation receiver: starting download.");
		receiver.clear();

		pause_solvable_stream = true;

		return true;
	}
	else {
		return finalize_arena_download();
	}
}

bool client_setup::start_downloading_arena(const arena_download_input& request) {
	LOG("Start downloading arena: %x (hash: %x)", request.arena_name, request.project_hash);

	last_download_request = request;

	/* Re-show once download completes as we'll be moved to spectator. */
	arena_gui.choose_team.show = true;

	if (!setup_external_arena_download_session()) {
		setup_direct_arena_download_session();
	}

	return start_downloading_session();
}

void client_setup::advance_external_downloader() {
	ensure(external_downloader != nullptr);
	ensure(downloading != std::nullopt);

	auto fallback_to_direct_download = [&]() {
		setup_direct_arena_download_session();

		if (!start_downloading_session()) {
			LOG("Couldn't start downloading session after failing external. Disconnecting.");

			disconnect();
		}
	};

	if (const auto new_file = external_downloader->get_downloaded_file()) {
		if (advance_downloading_session(augs::make_ptr_read_stream(new_file->second)) == message_handler_result::ABORT_AND_DISCONNECT) {
			LOG("External downloading session failed: %x", last_disconnect_reason);
			last_disconnect_reason = {};

			fallback_to_direct_download();
		}
	}
	else {
		if (const bool external_download_failed = !external_downloader->is_running()) {
			LOG("External downloading session failed.");

			fallback_to_direct_download();
		}
	}
}

message_handler_result client_setup::advance_downloading_session(
	const augs::cptr_memory_stream next_received_file
) {
	constexpr auto abort_v = message_handler_result::ABORT_AND_DISCONNECT;
	constexpr auto continue_v = message_handler_result::CONTINUE;

	LOG("New file bytes arrived. Size: %x", next_received_file.size());

	if (downloading == std::nullopt) {
		set_disconnect_reason("The server sent a file despite no request. Disconnecting.");
		return abort_v;
	}

	downloading->advance_with(next_received_file);

	if (downloading->in_progress()) {
		return continue_v;
	}
	else {
		if (finalize_arena_download()) {
			LOG("Sending RESYNC_ARENA_AFTER_FILES_DOWNLOADED.");
			special_request(special_client_request::RESYNC_ARENA_AFTER_FILES_DOWNLOADED);

			return continue_v;
		}
		else {
			return abort_v;
		}
	}
}

bool client_setup::finalize_arena_download() {
	ensure(downloading.has_value());

	external_downloader = nullptr;

	if (downloading->has_error()) {
		set_disconnect_reason(downloading->get_error());
		downloading = std::nullopt;
		return false;
	}

	downloading = std::nullopt;

	return try_load_arena_according_to(sv_public_vars, false);
}

bool client_setup::try_load_arena_according_to(const server_public_vars& new_vars, bool allow_download) {
	const auto& new_arena = new_vars.arena;

	LOG("Trying to load arena: %x (game_mode: %x)", new_arena, new_vars.game_mode.empty() ? "default" : new_vars.game_mode.c_str());
	LOG("Required arena hash: %x", new_vars.required_arena_hash);

	try {
		const auto& referential_arena = get_arena_handle(client_arena_type::REFERENTIAL);

		current_arena_folder = augs::path_type();

		editor_project* keep_loaded_project = nullptr;

		const auto choice_result = ::choose_arena_client(
			{
				editor_project_readwrite::reading_settings(),
				lua,
				referential_arena,
				official,
				new_vars.arena,
				new_vars.game_mode,
				clean_round_state,
				new_vars.playtesting_context,
				keep_loaded_project,
				nullptr
			},

			new_vars.required_arena_hash
		);

		if (choice_result.arena_folder_path.has_value()) {
			current_arena_folder = *choice_result.arena_folder_path;

			const bool was_showing_choose_team = arena_gui.choose_team.show;

			arena_gui.reset();
			arena_gui.choose_team.show = !is_replaying() && (was_showing_choose_team || ::is_spectator(referential_arena, get_local_player_id()));

			client_gui.rcon.show = false;
		}
		else {
			if (choice_result.official_differs) {
				set_disconnect_reason(typesafe_sprintf(
					"Failed to load arena: \"%x\".\n"
					"The local arena file differs from the servers!\n"
					"This is an official arena, so your game might be out of date.",
					new_arena
				));
			}
			else if (choice_result.invalid_arena_name) {
				set_disconnect_reason(typesafe_sprintf(
					"Failed to load arena: \"%x\".\n"
					"The server sent a forbidden arena name!",
					new_arena
				));
			}
			else if (choice_result.not_found_any) {
				if (allow_download) {
					return start_downloading_arena({ new_arena, new_vars.required_arena_hash });
				}
				else {
					set_disconnect_reason(typesafe_sprintf(
						"Failed to load arena: \"%x\".\n"
						"Couldn't load despite having just downloaded it...",
						new_arena
					));
				}
			}
			else {
				set_disconnect_reason(typesafe_sprintf(
					"Failed to load arena: \"%x\".\n"
					"Unknown error.",
					new_arena
				));
			}

			return false;
		}
	}
	catch (const augs::file_open_error& err) {
		set_disconnect_reason(typesafe_sprintf(
			"Failed to load arena: \"%x\".\n"
			"The arena files might be corrupt, or they might be missing.\n"
			"Please check if \"%x\" folder resides within \"%x\" directory.\n"
			"\nDetails: \n%x",
			new_arena,
			new_arena,
			"arenas",
			err.what()
		));

		return false;
	}
	catch (const augs::json_deserialization_error& err) {
		set_disconnect_reason(typesafe_sprintf("Failed to load \"%x\":\n%x.", new_vars.arena, err.what()));

		return false;
	}
	catch (const std::exception& err) {
		set_disconnect_reason(typesafe_sprintf("Failed to load \"%x\":\n%x.", new_vars.arena, err.what()));

		return false;
	}

	/* Prepare the predicted cosmos. */
	predicted_cosmos = scene.world;
	predicted_mode = current_mode_state;

	return true;
}

net_time_t client_setup::get_current_time() {
	return yojimbo_time();
}

entity_id client_setup::get_controlled_character_id() const {
	if (!is_gameplay_on()) {
		return entity_id::dead();
	}

	if (is_replaying()) {
		return entity_id::dead();
	}

	return on_mode_with_input(
		[&](const auto& typed_mode, const auto& in) {
			(void)in;

			const auto local_id = get_local_player_id();
			const auto local_character = typed_mode.lookup(local_id);

			return local_character;
		}
	);
}

void client_setup::customize_for_viewing(config_lua_table& config) const {
#if !IS_PRODUCTION_BUILD
	config.window.name = "Arena client";
#endif

	if (is_gameplay_on()) {
		get_arena_handle(client_arena_type::REFERENTIAL).adjust(config.drawing);
	}

	if (is_replaying()) {
		config.arena_mode_gui.show_spectator_overlay = demo_player.gui.show_spectator_overlay;
		config.client.spectated_arena_type = demo_player.gui.shown_arena_type;

		if (is_paused()) {
			config.interpolation.enabled = false;
		}
	}
}

void client_setup::accept_game_gui_events(const game_gui_entropy_type& events) {
	control(events);
}

bool client_setup::is_spectating_referential() const {
	const bool should_spectator_be_drawn = get_arena_handle(client_arena_type::REFERENTIAL).on_mode(
		[this](const auto& typed_mode) {
			return arena_gui.spectator.should_be_drawn(typed_mode);
		}
	);
	
	return vars.spectated_arena_type == client_arena_type::REFERENTIAL && should_spectator_be_drawn;
}

client_arena_type client_setup::get_viewed_arena_type() const {
	if (is_spectating_referential()) {
		return client_arena_type::REFERENTIAL;
	}

#if USE_CLIENT_PREDICTION
	return client_arena_type::PREDICTED;
#else
	return client_arena_type::REFERENTIAL;
#endif

}

online_arena_handle<false> client_setup::get_arena_handle(std::optional<client_arena_type> c) {
	if (c == std::nullopt) {
		c = get_viewed_arena_type();
	}

	return get_arena_handle_impl<online_arena_handle<false>>(*this, *c);
}

online_arena_handle<true> client_setup::get_arena_handle(std::optional<client_arena_type> c) const {
	if (c == std::nullopt) {
		c = get_viewed_arena_type();
	}

	return get_arena_handle_impl<online_arena_handle<true>>(*this, *c);
}

double client_setup::get_inv_tickrate() const {
	if (!is_gameplay_on()) {
		return default_inv_tickrate;
	}

	return get_arena_handle(client_arena_type::REFERENTIAL).get_inv_tickrate();
}

double client_setup::get_audiovisual_speed() const {
	if (!is_gameplay_on()) {
		return 1.0;
	}

	auto mult = 1.0;

	if (is_replaying()) {
		mult = demo_player.get_speed();
	}

	return mult * get_arena_handle().get_audiovisual_speed();
}

bool client_setup::push_or_handle(untimely_payload& p) {
	const auto session_id = p.associated_id;

	if (logically_set(find_by(session_id))) {
		const auto typed_handler = [&](auto& typed_payload) {
			return handle_untimely(typed_payload, session_id);
		};

		return std::visit(typed_handler, p.payload);
	}

	const auto next_session_id = get_arena_handle(client_arena_type::REFERENTIAL).on_mode(
		[&](const auto& mode) {
			return mode.get_next_session_id();
		}
	);

	const bool can_still_appear = p.associated_id.value >= next_session_id.value;

	if (can_still_appear) {
		untimely_payloads.emplace_back(p);
	}

	return true;
}

template <class U>
bool client_setup::handle_untimely(U& u, const session_id_type session_id) {
	if constexpr(std::is_same_v<U, arena_player_avatar_payload>) {
		auto& new_avatar = u;

		if (new_avatar.image_bytes.size() > 0) {
			try {
				const auto size = augs::image::get_size(new_avatar.image_bytes);

				if (size.x > max_avatar_side_v || size.y > max_avatar_side_v) {
					set_disconnect_reason(typesafe_sprintf("The server has tried to send an avatar of size %xx%x!", size.x, size.y));
					return false;
				}
			}
			catch (const augs::image_loading_error& err) {
				set_disconnect_reason("The server has tried to send an invalid avatar!\n%x", err.what());
				return false;
			}
		}

		const auto player_id = find_by(session_id);
		ensure(logically_set(player_id));

		player_metas[player_id.value].avatar = std::move(new_avatar);
		rebuild_player_meta_viewables = true;
	}
	else {
		static_assert(always_false_v<U>);
	}

	return true;
}

void client_setup::handle_incoming_payloads() {
	namespace N = net_messages;

	if (vars.network_simulator.value.loss_percent >= 100.f) {
		return;
	}

	auto& message_handler = *this;

	adapter->advance(client_time, message_handler);
}

void client_setup::advance_demo_recorder() {
	++recorded_demo_step;

	if (client_time - when_last_flushed_demo > vars.flush_demo_to_disk_once_every_secs) {
		if (::valid_and_is_ready(future_flushed_demo)) {
			future_flushed_demo.get();
		}

		if (demo_flushing_finished()) {
			flush_demo_steps();
		}
	}
}

void client_setup::send_pending_commands() {
	using C = client_state_type;

	const bool init_send = state == C::NETCODE_NEGOTIATING_CONNECTION;

	const bool can_already_resend_settings = client_time - when_sent_client_settings > 1.0;
	const bool resend_requested_settings = can_already_resend_settings && current_requested_settings != requested_settings;

	auto send_settings = [&]() {
		send_payload(
			game_channel_type::RELIABLE_MESSAGES,
			std::as_const(requested_settings)
		);

		if (init_send) {
			LOG("Sent initial client configuration to the server.");
			state = client_state_type::PENDING_WELCOME;
		}
		else {
			LOG("Sent repeated client configuration to the server.");
		}

		when_sent_client_settings = client_time;
		current_requested_settings = requested_settings;
	};

	if (init_send || resend_requested_settings) {
		send_settings();
	}

	const auto& avatar_path = vars.avatar_image_path;

	if (state == C::IN_GAME) {
		if (!has_sent_avatar) {
			if (avatar_path.empty()) {
				/* Send an empty payload to signal that there won't be any avatar. */
				arena_player_avatar_payload payload;

				const auto dummy_client_id = session_id_type::dead();

				send_payload(
					game_channel_type::RELIABLE_MESSAGES,

					dummy_client_id,
					payload
				);
			}
			else {
				arena_player_avatar_payload payload;

				try {
					payload.image_bytes = augs::file_to_bytes(avatar_path);
				}
				catch (...) {
					payload.image_bytes.clear();
				}

				if (payload.image_bytes.size() > 0 && payload.image_bytes.size() <= max_avatar_bytes_v) {
					const auto dummy_client_id = session_id_type::dead();

					send_payload(
						game_channel_type::RELIABLE_MESSAGES,

						dummy_client_id,
						payload
					);
				}
				else {
					const auto reason = typesafe_sprintf(
						"The avatar file (%x) exceeds the maximum size of %x.\nSupply a less entropic image file.", 
						readable_bytesize(payload.image_bytes.size()), 
						readable_bytesize(max_avatar_bytes_v)
					);

					set_disconnect_reason(reason);
					disconnect();
				}
			}

			has_sent_avatar = true;
		}
	}

	for (const auto& pending_request : pending_requests) {
		send_payload(
			game_channel_type::RELIABLE_MESSAGES,
			pending_request
		);
	}

	pending_requests.clear();
}

void client_setup::reset_afk_timer() {
	special_request(special_client_request::RESET_AFK_TIMER);
}

void client_setup::special_request(const special_client_request r) {
	pending_requests.push_back(r);
}

void client_setup::traverse_nat_if_required() {
	if (is_replaying()) {
		return;
	}

	if (!adapter->is_connecting()) {
		return;
	}
}

file_chunk_index_type client_setup::calc_num_chunks_per_tick() const {
	const auto inv_tickrate = default_inv_tickrate;

	const auto target_bandwidth = vars.max_direct_file_bandwidth * 1024 * 1024;
	const auto target_bandwidth_per_tick = target_bandwidth * inv_tickrate;

	const auto chunks_per_tick = std::max(
		uint32_t(1),
		uint32_t(target_bandwidth_per_tick / file_chunk_size_v)
	);

	return chunks_per_tick;
}

void client_setup::exchange_file_packets() {
	if (!is_connected()) {
		return;
	}

	send_keepalive_download_progress();

	const auto inv_tickrate = default_inv_tickrate;
	const double chunk_interval = inv_tickrate * 2;

	handle_incoming_payloads();

	if (direct_downloader.has_value()) {
		file_chunks_request_payload chunks;

		for (uint32_t i = 0; i < num_skip_chunks; ++i) {
			direct_downloader->request_next_chunk().last_sent = client_time;
		}

		num_skip_chunks = 0;

		for (uint32_t i = 0; i < calc_num_chunks_per_tick(); ++i) {
			auto& next_chunk = direct_downloader->request_next_chunk();

			if (next_chunk.last_sent + chunk_interval > client_time) {
				continue;
			}

			next_chunk.last_sent = client_time;

			// LOG_NVPS(next_chunk.index);

			chunks.requests.push_back(next_chunk.index);
		}

		send_payload(
			game_channel_type::VOLATILE_STATISTICS,
			chunks
		);
	}

	send_packets();

	client_time += inv_tickrate;
}

void client_setup::send_download_progress() {
	ensure(downloading.has_value());

	::download_progress_message progress;
	progress.progress = get_total_download_percent_complete(true) * 255;

	send_payload(
		game_channel_type::RELIABLE_MESSAGES,
		progress
	);
}

bool client_setup::send_keepalive_download_progress() {
	const auto new_time = get_current_time();
	const bool can_already = new_time - when_sent_last_keepalive >= 0.5f;

	if (can_already) {
		LOG("Sending keepalive download progress.");

		when_sent_last_keepalive = new_time;
		send_download_progress();

		return true;
	}

	return false;
}

void client_setup::send_packets() {
	if (is_replaying()) {
		return;
	}

	if (vars.network_simulator.value.loss_percent >= 100.f) {
		return;
	}

	adapter->send_packets();
}

void client_setup::perform_chat_input_bar() {
	auto& chat = client_gui.chat;

	if (chat.perform_input_bar(vars.client_chat) && is_connected()) {
		::client_requested_chat message;

		message.target = chat.target;
		message.message = chat.current_message;

		send_payload(
			game_channel_type::RELIABLE_MESSAGES,
			message
		);

		chat.current_message.clear();
	}
}

void client_setup::snap_interpolation_of_viewed() {
	auto snap_for = [&](const auto arena_type) {
		snap_interpolated_to_logical(get_arena_handle(arena_type).get_cosmos());
	};

	snap_for(client_arena_type::PREDICTED);
	snap_for(client_arena_type::REFERENTIAL);
}

#if DEBUG_SOLVABLES
#include "augs/readwrite/lua_file.h"
#endif

void client_setup::perform_demo_player_imgui(augs::window& window) {
	demo_player.gui.perform(window, demo_player);
	
	auto& pending_snap = demo_player.gui.pending_interpolation_snap;

	if (pending_snap) {
		snap_interpolation_of_viewed();
		pending_snap = false;
	}

	if (demo_player.gui.pending_dump) {
		demo_player.gui.pending_dump = false;

#if DEBUG_DESYNCS
		LOG_BYTE_SERIALIZE = true;
		augs::save_as_bytes(clean_round_state, "/tmp/crs.solv");
		augs::save_as_lua_table(lua, clean_round_state, "/tmp/crs.lua");
		LOG_BYTE_SERIALIZE = false;
#endif
	}
}

auto client_setup::get_current_file_download_progress() const {
	if (external_downloader != nullptr) {
		return yojimbo::BlockProgress {
			uint32_t(external_downloader->get_downloaded_bytes()),
			uint32_t(external_downloader->get_total_bytes())
		};
	}

	if (direct_downloader.has_value()) {
		return yojimbo::BlockProgress {
			uint32_t(direct_downloader->get_downloaded_bytes()),
			uint32_t(direct_downloader->get_total_bytes())
		};
	}

	return yojimbo::BlockProgress { 0, 0 };
}

float client_setup::get_current_file_percent_complete() const {
	auto this_progress = get_current_file_download_progress();

	if (this_progress.blockSize == 0) {
		return 0.0f;
	}

	return this_progress.blockSize == 0 ? 0.0f : float(this_progress.downloadedBytes) / this_progress.blockSize;
}

float client_setup::get_total_download_percent_complete(const bool smooth) const {
	return downloading->get_total_percent_complete(smooth ? get_current_file_percent_complete() : 0.0f);
}

custom_imgui_result client_setup::perform_custom_imgui(
	const perform_custom_imgui_input in
) {
	using C = client_state_type;
	using namespace augs::imgui;

	arena_gui.resyncing_notifier = now_resyncing;

	const bool gameplay_on = is_gameplay_on();

	auto& rcon_gui = client_gui.rcon;

	if (!is_connected()) {
		rcon_gui.show = false;
	}

	if (!arena_gui.scoreboard.show && rcon_gui.show) {
		auto on_new_payload = [&](const auto& new_payload) {
			rcon_command_variant payload;
			payload = new_payload;

			send_payload(
				game_channel_type::RELIABLE_MESSAGES,
				payload
			);
		};

		const bool is_remote_server = true;

		perform_rcon_gui(
			rcon_gui,
			is_remote_server,
			on_new_payload
		);
	}

	perform_chat_input_bar();

	if (is_replaying()) {
		perform_demo_player_imgui(in.window);
	}

	if (gameplay_on) {
		augs::network::enable_detailed_logs(false);

		arena_gui_base::perform_custom_imgui(in);
	}
	else {
		auto print_reason_if_any = [&]() {
			if (last_disconnect_reason.empty()) {
				return;
			}

			if (print_only_disconnect_reason) {
				text(last_disconnect_reason);
			}
			else {
				text("Reason:\n\n%x", last_disconnect_reason);
			}
		};

		ImGuiWindowFlags window_flags = 
			ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoResize 
			| ImGuiWindowFlags_NoScrollbar 
			| ImGuiWindowFlags_NoScrollWithMouse
			| ImGuiWindowFlags_NoMove 
			| ImGuiWindowFlags_NoSavedSettings
		;

		const bool is_downloading = is_connected() && downloading.has_value();

		center_next_window(ImGuiCond_Always);

		if (is_downloading) {
			const auto line_height = 28;
			const auto num_lines = 13;

			const auto window_size = ImVec2(800, line_height * num_lines);

			ImGui::SetNextWindowSize(window_size);
		}
		else {
			window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		}

		const auto window_name = "Connection status";
		auto window = scoped_window(window_name, nullptr, window_flags);

#if 0
		if (is_replaying() && demo_replay_failed_reason.size() > 0) {
			text("Error during demo replay:");
			text(demo_replay_failed_reason);

			text("\n");
			ImGui::Separator();

			if (ImGui::Button("Go back")) {
				return custom_imgui_result::GO_TO_MAIN_MENU;
			}
		}
		else
#endif
		if (is_connected()) {
			augs::network::enable_detailed_logs(false);

			if (downloading.has_value()) {
				text_color("Downloading:", green);
				ImGui::SameLine();
				text_color(downloading->get_arena_name(), yellow);
				ImGui::Separator();

				const bool externally = external_downloader != nullptr;

				const auto bytes_per_second = 
					externally ?
					external_downloader->get_bandwidth() :
					direct_bandwidth.getAverageSpeed()
				;

				const auto this_progress = get_current_file_download_progress();
				const auto this_percent_complete = get_current_file_percent_complete();

				if (externally) {
					text_color("External mirror: ", cyan);
					ImGui::SameLine();
					text(sv_public_vars.external_arena_files_provider);
					ImGui::Separator();
				}
				else {
					text_color("Downloading directly over UDP.", yellow);
					ImGui::Separator();
				}

				if (downloading->now_downloading_external_resources()) {
					const auto downloaded_index = downloading->get_downloaded_file_index();
					const auto num_all = downloading->num_all_downloaded_files();

					/* Looks more pro without the easing per-file after all */

					text(typesafe_sprintf(
						"File: %x of %x",
						downloaded_index + 1,
						num_all
					));

					ImGui::ProgressBar(get_total_download_percent_complete(true), ImVec2(-1.0f,0.0f));

					text("\n");
				}

				{

					text(downloading->get_displayed_file_path());

					ImGui::ProgressBar(this_percent_complete, ImVec2(-1.0f, 0.0f));

					const auto readable_speed = readable_bytesize(bytes_per_second , "%2f");

					text(typesafe_sprintf(
						"%x / %x",
						readable_bytesize(this_progress.downloadedBytes, "%2f"),
						readable_bytesize(this_progress.blockSize, "%2f")
					));

					text_disabled(typesafe_sprintf("(Speed: %x/s)", readable_speed));
				}

				text("\n");
				ImGui::Separator();
			}
			else {
				text_color(typesafe_sprintf("Connected to %x.", get_displayed_connecting_server_name()), green);

				if (state == C::NETCODE_NEGOTIATING_CONNECTION) {
					text("Initializing connection...");
				}
				else if (state == C::PENDING_WELCOME) {
					text("Sending the client configuration.");
				}
				else if (state == C::RECEIVING_INITIAL_SNAPSHOT) {
					text("Receiving the initial state:");
				}
				else if (state == C::RECEIVING_INITIAL_SNAPSHOT_CORRECTION) {
					text("Receiving the initial state correction:");
				}
				else if (pause_solvable_stream) {
					text("Download complete. Rejoining the game.");
				}
				else {
					text("Unknown error.");
				}

				text("\n");
				ImGui::Separator();
			}

			if (ImGui::Button("Abort")) {
				disconnect();
			}
		}
		else if (state == C::NETCODE_NEGOTIATING_CONNECTION && adapter->is_connecting()) {
			text("Connecting to %x\nTime: %2f seconds", get_displayed_connecting_server_name(), get_current_time() - when_initiated_connection);

			text("\n");
			ImGui::Separator();

			if (ImGui::Button("Abort")) {
				disconnect();
				return custom_imgui_result::GO_TO_MAIN_MENU;
			}
		}
		else if (
			const bool failed_after_connected = adapter->is_connecting() && state > C::NETCODE_NEGOTIATING_CONNECTION; 
			failed_after_connected || adapter->has_connection_failed()
		) {
			if (state == C::IN_GAME) {
				text("Lost connection to the server.");
			}
			else if (state == C::NETCODE_NEGOTIATING_CONNECTION) {
				text("Failed to establish connection with %x", get_displayed_connecting_server_name());
			}
			else {
				text("Failed to join %x", get_displayed_connecting_server_name());
			}

			print_reason_if_any();

			text("\n");
			ImGui::Separator();

			if (ImGui::Button("Retry")) {
				return custom_imgui_result::RETRY;
			}

			ImGui::SameLine();

			if (ImGui::Button("Go back")) {
				return custom_imgui_result::GO_TO_MAIN_MENU;
			}
		}
		else if (adapter->is_disconnected()) {
			if (!print_only_disconnect_reason) {
				text("Disconnected from the server.");
			}

			print_reason_if_any();

			text("\n");
			ImGui::Separator();

			if (ImGui::Button("Go back")) {
				return custom_imgui_result::GO_TO_MAIN_MENU;
			}
		}
	}

	return custom_imgui_result::NONE;
}

void client_setup::apply(const config_lua_table& cfg) {
	vars = cfg.client;

	if (is_replaying()) {
		return;
	}

	auto& r = requested_settings;
	r.chosen_nickname = vars.nickname;
	r.suppress_webhooks = vars.suppress_webhooks;
	r.rcon_password = vars.rcon_password;
	r.net = vars.net;
	r.public_settings.character_input = cfg.input.character;

	adapter->set(vars.network_simulator);
}

bool client_setup::is_connected() const {
	if (is_replaying()) {
		return true;
	}

	return adapter->is_connected();
}

void client_setup::send_to_server(
	total_client_entropy& new_local_entropy
) {
	send_payload(
		game_channel_type::RELIABLE_MESSAGES,
		new_local_entropy
	);
}

void client_setup::disconnect() {
	if (is_replaying()) {
		demo_player.source_path.clear();
		return;
	}

	adapter->disconnect();
	downloading = std::nullopt;
}

bool client_setup::is_gameplay_on() const {
	if (pause_solvable_stream) {
		return false;
	}

	return is_connected() && state == client_state_type::IN_GAME;
}

setup_escape_result client_setup::escape() {
	if (!is_gameplay_on()) {
		return setup_escape_result::GO_TO_MAIN_MENU;
	}

	if (client_gui.rcon.escape()) {
		return setup_escape_result::JUST_FETCH;
	}

	if (arena_gui.escape()) {
		return setup_escape_result::JUST_FETCH;
	}

	if (is_replaying() && !is_paused()) {
		demo_player.pause();
		return setup_escape_result::JUST_FETCH;
	}

	return setup_escape_result::LAUNCH_INGAME_MENU;
}

const cosmos& client_setup::get_viewed_cosmos() const {
	return get_arena_handle(get_viewed_arena_type()).get_cosmos();
}

void client_setup::update_stats(network_info& stats) const {
	stats = adapter->get_network_info();
}

augs::path_type client_setup::get_unofficial_content_dir() const {
	return current_arena_folder;
}

bool client_setup::handle_input_before_game(
	const handle_input_before_game_input in
) {
	if (arena_gui_base::handle_input_before_game(in)) {
		return true;
	}

	if (client_gui.control(in)) {
		return true;
	}

	if (is_replaying()) {
		if (demo_player.control(in)) {
			return true;
		}

		const auto& state = in.common_input_state;
		const auto& e = in.e;

		if (e.was_any_key_pressed()) {
			using namespace augs::event::keys;

			const auto k = e.data.key.key;

			auto forward = [&](const auto& secs) {
				demo_player.seek_forward(secs / get_inv_tickrate());
			};

			auto backward = [&](const auto& secs) {
				demo_player.seek_backward(secs / get_inv_tickrate());
			};


			const bool has_shift{ state[key::LSHIFT] || state[key::RSHIFT] };

			switch (k) {
				case key::RIGHT: forward(has_shift ? 1 : 5); return true;
				case key::LEFT: backward(has_shift ? 1 : 5); return true;
				case key::UP: forward(has_shift ? 5 : 10); return true;
				case key::DOWN: backward(has_shift ? 5 : 10); return true;
				default: break;
			}
		}

	}

	return false;
}

void client_setup::draw_custom_gui(const draw_setup_gui_input& in) const {
	using namespace augs::gui::text;

	client_gui.chat.draw_recent_messages(
		in.get_drawer(),
		vars.client_chat,
		in.config.faction_view,
		in.gui_fonts.gui,
		get_current_time()
	);

	arena_gui_base::draw_custom_gui(in);
}

std::optional<arena_player_metas> client_setup::get_new_player_metas() {
	if (rebuild_player_meta_viewables) {
		rebuild_player_meta_viewables = false;
		return player_metas;
	}

	return std::nullopt;
}

mode_player_id client_setup::get_local_player_id() const {
	return client_player_id;
}

mode_player_id client_setup::find_by(const session_id_type id) const {
	return get_arena_handle(client_arena_type::REFERENTIAL).on_mode(
		[&](const auto& mode) {
			return mode.lookup(id);
		}
	);
}

std::optional<session_id_type> client_setup::find_session_id(const mode_player_id id) const {
	return get_arena_handle(client_arena_type::REFERENTIAL).on_mode(
		[&](const auto& mode) -> std::optional<session_id_type> {
			if (const auto entry = mode.find(id)) {
				return entry->session.id;
			}

			return std::nullopt;
		}
	);
}

std::optional<session_id_type> client_setup::find_local_session_id() const {
	return find_session_id(get_local_player_id());
}

template <class I, class O, class K>
auto find_in_indirectors(const I& indirectors, O& objects, const K key) -> maybe_const_ptr_t<std::is_const_v<O>, typename O::value_type> {
	if (key.indirection_index >= indirectors.size()) {
		return nullptr;
	}

	const auto& indirector = indirectors[key.indirection_index];
	using size_type = decltype(indirector.real_index);

	const bool versions_match = indirector.version == key.version && indirector.real_index != static_cast<size_type>(-1);

	if (!versions_match) {
		return nullptr;
	}

	return &objects[indirector.real_index];
}

void save_interpolations(
	interpolation_transfer_caches& caches,
	const cosmos& source
) {
	source.get_solvable().significant.entity_pools.for_each_container(
		[&](const auto& p) {
			using P = remove_cref<decltype(p)>;
			using V = typename P::mapped_type;
			using E = entity_type_of<V>;

			if constexpr(has_all_of_v<E, invariants::interpolation>) {
				auto& c = caches.get_for<E>();
				c.interpolations = p.template get_corresponding_array<components::interpolation>();
				c.indirectors = p.get_indirectors();
			}
		}
	);
}

void restore_interpolations(
	const interpolation_transfer_caches& caches,
	cosmos& target
) {
	target.for_each_having<invariants::interpolation>(
		[&](const auto& typed_adjusted) {
			using E = entity_type_of<decltype(typed_adjusted)>;

			const auto id = typed_adjusted.get_id();
			const auto& c = caches.get_for<E>();

			if (const auto entry = ::find_in_indirectors(c.indirectors, c.interpolations, id.raw)) {
				get_corresponding<components::interpolation>(typed_adjusted) = *entry;
			}
		}
	);
}

void client_setup::handle_new_session(const add_player_input& in) {
	const auto new_player = in.id;
	const auto new_session_id = find_session_id(new_player);

	ensure(new_session_id.has_value());

	auto& meta = player_metas[new_player.value];
	meta.clear_session_channeled_data();
	meta.session_id = *new_session_id;

	auto untimely_handler = [&](auto& untimely) {
		if (logically_set(find_by(untimely.associated_id))) {
			const auto typed_handler = [&](auto& typed_payload) {
				return handle_untimely(typed_payload, untimely.associated_id);
			};

			const auto result = std::visit(typed_handler, untimely.payload);

			if (!result) {
				disconnect();
			}

			return true;
		}

		return false;
	};

	erase_if(untimely_payloads, untimely_handler);
	rebuild_player_meta_viewables = true;
}

bool client_setup::requires_cursor() const {
	return arena_gui_base::requires_cursor() || client_gui.requires_cursor() || demo_player.gui.requires_cursor();
}

void client_setup::ensure_handler() {
	wait_for_demo_flush();
	flush_demo_steps();
	wait_for_demo_flush();
}

void client_setup::wait_for_demo_flush() {
	if (future_flushed_demo.valid()) {
		future_flushed_demo.wait();
	}
}
