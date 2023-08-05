#include "augs/misc/pool/pool_io.hpp"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/compress.h"
#include "augs/string/parse_url.h"

#include "application/setups/server/server_setup.h"
#include "application/config_lua_table.h"

#include "application/network/server_adapter.hpp"

#include "augs/filesystem/file.h"
#include "application/arena/arena_paths.h"

#include "application/network/net_message_translation.h"
#include "application/network/net_serialize.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_file.h"

#include "application/arena/arena_handle.h"
#include "application/arena/choose_arena.h"
#include "application/gui/client/chat_gui.h"
#include "application/network/payload_easily_movable.h"
#include "application/gui/client/rcon_gui.hpp"
#include "application/arena/arena_handle.hpp"
#include "application/masterserver/server_heartbeat.h"
#include "application/network/resolve_address.h"
#include "augs/templates/thread_templates.h"
#include "application/masterserver/masterserver.h"
#include "augs/network/netcode_utils.h"
#include "application/masterserver/masterserver_requests.h"
#include "application/nat/stun_session.h"
#include "augs/templates/bit_cast.h"
#include "application/masterserver/gameserver_command_readwrite.h"
#include "application/detail_file_paths.h"
#include "3rdparty/include_httplib.h"
#include "application/setups/server/webhooks.h"
#include "game/messages/hud_message.h"
#include "application/setups/editor/resources/resource_traits.h"
#include "augs/readwrite/json_readwrite_errors.h"

#include "application/setups/server/server_json_events.h"
#include "augs/readwrite/json_readwrite.h"
#include "application/setups/editor/editor_paths.h"
#include "game/modes/arena_mode.hpp"
#include "game/messages/mode_notification.h"

const auto only_connected_v = server_setup::for_each_flags {
	server_setup::for_each_flag::ONLY_CONNECTED
};

const auto connected_and_integrated_v = server_setup::for_each_flags { 
	server_setup::for_each_flag::WITH_INTEGRATED,
	server_setup::for_each_flag::ONLY_CONNECTED
};

#include "application/setups/server/server_handle_payload.hpp"

server_setup::server_setup(
	sol::state& lua,
	const packaged_official_content& official,
	const augs::server_listen_input& in,
	const server_vars& initial_vars,
	const server_private_vars& private_initial_vars,
	const client_vars& integrated_client_vars,
	const std::optional<augs::dedicated_server_input> dedicated,
	const server_nat_traversal_input& nat_traversal_input,
	bool suppress_community_server_webhook_this_run
) : 
	integrated_client_vars(integrated_client_vars),
	lua(lua),
	official(official),
	last_loaded_project(std::make_unique<editor_project>()),
	last_start(in),
	dedicated(dedicated),
	server(
		std::make_unique<server_adapter>(
			in,
			dedicated == std::nullopt,
			[this](auto&&... args) {
				if (respond_to_ping_requests(std::forward<decltype(args)>(args)...)) {
					return true;
				}

				return nat_traversal.handle_auxiliary_command(std::forward<decltype(args)>(args)...); 
			}
		)
	),
	server_time(yojimbo_time()),
	nat_traversal(nat_traversal_input, resolved_server_list_addr),
	suppress_community_server_webhook_this_run(suppress_community_server_webhook_this_run)
{
	{
		auto initial_vars_modified = initial_vars;
		auto source_server_name = std::string(initial_vars_modified.server_name);
		str_ops(source_server_name).replace_all("${MY_NICKNAME}", std::string(integrated_client_vars.nickname));
		initial_vars_modified.server_name = source_server_name;

		apply(initial_vars_modified, true);
	}

	apply(private_initial_vars);

	if (private_initial_vars.master_rcon_password.empty()) {
		LOG("WARNING! The master rcon password is empty! This means that only the localhost can access the master rcon.");
	}

	if (private_initial_vars.rcon_password.empty()) {
		LOG("WARNING! The rcon password is empty! This means that only the localhost can access the rcon.");
	}

	if (dedicated == std::nullopt) {
		integrated_client.init(server_time);
		integrated_client.state = client_state_type::IN_GAME;
		integrated_client.settings.chosen_nickname = integrated_client_vars.nickname;

		if (!integrated_client_vars.avatar_image_path.empty()) {
			auto& image_bytes = integrated_client.meta.avatar.image_bytes;

			try {
				image_bytes = augs::file_to_bytes(integrated_client_vars.avatar_image_path);

				const auto size = augs::image::get_size(image_bytes);

				if (size.x > max_avatar_side_v || size.y > max_avatar_side_v) {
					image_bytes.clear();
				}
			}
			catch (...) {
				image_bytes.clear();
			}
		}

		push_connected_webhook(to_mode_player_id(get_integrated_client_id()));

		rebuild_player_meta_viewables = true;
	}

	const bool conditions_fulfilled = [&]() {
		if (dedicated == std::nullopt) {
			if (!nickname_len_in_range(integrated_client_vars.nickname.length())) {
				failure_reason = typesafe_sprintf(
					"The nickname should have between %x and %x bytes.", 
					min_nickname_length_v,
					max_nickname_length_v
				);

				return false;
			}
		}

		return true;

	}();

	if (!conditions_fulfilled) {
		shutdown();
	}

	resolve_server_list();
	refresh_runtime_info_for_rcon();
}

void server_setup::reset_afk_timer() {
	integrated_client.last_keyboard_activity_time = server_time;
}

void server_setup::send_goodbye_to_masterserver() {
	if (has_sent_any_heartbeats() && resolved_server_list_addr.has_value()) {
		const auto destination_address = resolved_server_list_addr.value();

		const auto goodbye = masterserver_request(masterserver_in::goodbye {});
		auto goodbye_bytes = augs::to_bytes(goodbye);

		for (int i = 0; i < 5; ++i) {
			server->send_udp_packet(destination_address, goodbye_bytes.data(), goodbye_bytes.size());
		}
	}
}

void server_setup::shutdown() {
	send_goodbye_to_masterserver();

	if (server->is_running()) {
		LOG("Shutting down the server.");
		server->stop();
	}
}

/* To avoid incomplete type error */
server_setup::~server_setup() {
	if (is_running()) {
		broadcast_shutdown_message();
		shutdown();
	}
}

void server_heartbeat::validate() {
	if (server_name.empty()) {
		server_name = "Hypersomnia Server";
	}

	if (current_arena.empty()) {
		current_arena = "NONE";
	}
}

bool server_heartbeat::is_valid() const {
	if (!is_nickname_valid_characters(server_name)) {
		return false;
	}

	return max_online >= 2 && !server_name.empty() && !current_arena.empty() && !game_mode.empty();
}

template <class F>
void server_setup::push_webhook_job(F&& f, mode_player_id id) {
	auto ptr = std::make_unique<std::future<std::string>>(
		std::async(std::launch::async, std::forward<F>(f))
	);

	pending_jobs.emplace_back(webhook_job{ id, std::move(ptr) });
}

void server_setup::log_match_start_json(const messages::team_match_start_message& msg) {
	server_json_events::match_start start;

	auto write_faction = [&](auto& to, const auto& from) {
		for (const auto& player : from) {
			to.players.push_back({ player.nickname });
		}
	};

	write_faction(start.team_1, msg.team_1);
	write_faction(start.team_2, msg.team_2);

	LOG("SERVER_EVENT match_start: %x", augs::to_json_string_nopretty(start));
}

void server_setup::log_match_end_json(const messages::match_summary_message& summary) {
	server_json_events::match_end end;

	auto write_faction = [&](auto& to, const auto from_score, const auto& from) {
		to.score = from_score;

		for (const auto& player : from) {
			to.players.push_back({
				player.score,
				player.nickname
			});
		}
	};

	write_faction(end.team_1, summary.first_team_score, summary.first_faction);
	write_faction(end.team_2, summary.second_team_score, summary.second_faction);

	LOG("SERVER_EVENT match_end: %x", augs::to_json_string_nopretty(end));
}

void server_setup::default_server_post_solve(const const_logic_step step) {
	{
		const auto& match_starts = step.get_queue<messages::team_match_start_message>();

		for (const auto& start : match_starts) {
			log_match_start_json(start);
		}
	}

	{
		const auto& duels = step.get_queue<messages::duel_of_honor_message>();

		for (const auto& duel : duels) {
			push_duel_of_honor_webhook(duel.first_player, duel.second_player);
		}
	}

	{
		const auto& duel_interrupts = step.get_queue<messages::duel_interrupted_message>();

		for (const auto& duel_interrupt : duel_interrupts) {
			push_duel_interrupted_webhook(duel_interrupt);
		}
	}

	{
		const auto& summaries = step.get_queue<messages::match_summary_message>();

		for (const auto& summary : summaries) {
			request_immediate_heartbeat();

			push_match_summary_webhook(summary);
			log_match_end_json(summary);
		}
	}

	if (vars.shutdown_after_first_match) {
		const auto& ends = step.get_queue<messages::match_summary_ended>();

		for (const auto& ended : ends) {
			request_immediate_heartbeat();

			if (ended.is_final) {
				schedule_shutdown();
			}
		}
	}
}

std::string server_setup::get_next_duel_pic_link() {
	auto pattern = vars.webhooks.duel_of_honor_pic_link_pattern;
	auto duel_pic_i = duel_pic_counter % vars.webhooks.num_duel_pics;
	++duel_pic_counter;

	return typesafe_sprintf(pattern, duel_pic_i + 1);
}

void server_setup::push_duel_interrupted_webhook(const messages::duel_interrupted_message& interrupt_info) {
	finalize_webhook_jobs();

	if (auto discord_webhook_url = parsed_url(private_vars.discord_webhook_url); discord_webhook_url.valid()) {
		auto server_name = get_server_name();

		LOG("pushing duel interrupted webhook.");

		auto fled = vars.webhooks.fled_pic_link;
		auto reconsidered = vars.webhooks.reconsidered_pic_link;

		push_webhook_job(
			[discord_webhook_url, server_name, fled, reconsidered, interrupt_info]() -> std::string {
				const auto ca_path = CA_CERT_PATH;
				http_client_type http_client(discord_webhook_url.host);

#if BUILD_OPENSSL
				http_client.set_ca_cert_path(ca_path.c_str());
				http_client.enable_server_certificate_verification(true);
#endif
				http_client.set_follow_location(true);
				http_client.set_read_timeout(5);
				http_client.set_write_timeout(5);

				auto items = discord_webhooks::form_duel_interrupted(
					server_name,
					fled,
					reconsidered,
					interrupt_info
				);

				http_client.Post(discord_webhook_url.location.c_str(), items);

				return "";
			}
		);
	}
}

void server_setup::push_match_summary_webhook(const messages::match_summary_message& summary) {
	finalize_webhook_jobs();

	if (auto discord_webhook_url = parsed_url(private_vars.discord_webhook_url); discord_webhook_url.valid()) {
		auto server_name = get_server_name();

		const auto mvp_state = find_client_state(summary.mvp_player_id);

		if (mvp_state == nullptr) {
			return;
		}

		auto mvp_nickname = mvp_state->get_nickname();
		auto mvp_player_avatar_url = mvp_state->uploaded_avatar_url;
		const auto this_duel_index = (duel_pic_counter - 1) % vars.webhooks.num_duel_pics;
		auto duel_victory_pic_link = typesafe_sprintf(vars.webhooks.duel_victory_pic_link_pattern, this_duel_index);

		LOG("pushing match summary webhook.");

		push_webhook_job(
			[discord_webhook_url, server_name, mvp_nickname, mvp_player_avatar_url, duel_victory_pic_link, summary]() -> std::string {
				const auto ca_path = CA_CERT_PATH;
				http_client_type http_client(discord_webhook_url.host);

#if BUILD_OPENSSL
				http_client.set_ca_cert_path(ca_path.c_str());
				http_client.enable_server_certificate_verification(true);
#endif
				http_client.set_follow_location(true);
				http_client.set_read_timeout(5);
				http_client.set_write_timeout(5);

				auto items = discord_webhooks::form_match_summary(
					server_name,
					mvp_nickname,
					mvp_player_avatar_url,
					duel_victory_pic_link,
					summary
				);

				http_client.Post(discord_webhook_url.location.c_str(), items);

				return "";
			}
		);
	}
}

void server_setup::push_duel_of_honor_webhook(const std::string& first, const std::string& second) {
	if (auto discord_webhook_url = parsed_url(private_vars.discord_webhook_url); discord_webhook_url.valid()) {
		auto server_name = get_server_name();

		LOG("pushing duel webhook with %x versus %x", first, second);

		push_webhook_job(
			[first, second, discord_webhook_url, server_name, duel_pic_link = get_next_duel_pic_link()]() -> std::string {
				const auto ca_path = CA_CERT_PATH;
				http_client_type http_client(discord_webhook_url.host);

#if BUILD_OPENSSL
				http_client.set_ca_cert_path(ca_path.c_str());
				http_client.enable_server_certificate_verification(true);
#endif
				http_client.set_follow_location(true);
				http_client.set_read_timeout(5);
				http_client.set_write_timeout(5);

				auto items = discord_webhooks::form_duel_of_honor(
					server_name,
					first,
					second,
					duel_pic_link
				);

				http_client.Post(discord_webhook_url.location.c_str(), items);

				return "";
			}
		);
	}
}

void server_setup::push_connected_webhook(const mode_player_id id) {
	auto client_state = find_client_state(id);

	if (client_state == nullptr) {
		return;
	}

	if (client_state->pushed_connected_webhook) {
		return;
	}

	client_state->pushed_connected_webhook = true;

	auto connected_player_nickname = client_state->get_nickname();

	auto discord_webhook_url = parsed_url(private_vars.discord_webhook_url);
	auto telegram_webhook_url = parsed_url(private_vars.telegram_webhook_url);

	if (discord_webhook_url.valid() || telegram_webhook_url.valid()) {
		auto server_name = get_server_name();
		auto avatar = client_state->meta.avatar.image_bytes;
		auto all_nicknames = get_all_nicknames();
		auto current_arena_name = get_current_arena_name();
		auto priv_vars = private_vars;

		push_webhook_job(
			[priv_vars, telegram_webhook_url, discord_webhook_url, server_name, avatar, connected_player_nickname, all_nicknames, current_arena_name]() -> std::string {
				if (telegram_webhook_url.valid()) {
					auto telegram_channel_id = priv_vars.telegram_channel_id;

					const auto ca_path = CA_CERT_PATH;
					http_client_type http_client(telegram_webhook_url.host);

#if BUILD_OPENSSL
					http_client.set_ca_cert_path(ca_path.c_str());
					http_client.enable_server_certificate_verification(true);
#endif
					http_client.set_follow_location(true);
					http_client.set_read_timeout(5);
					http_client.set_write_timeout(5);

					auto items = telegram_webhooks::form_player_connected(
						telegram_channel_id,
						connected_player_nickname
					);

					const auto location = telegram_webhook_url.location + "/sendMessage";
					auto response = http_client.Post(location.c_str(), items);

					if (response) {
						LOG("Received TG response.");
					}
					else {
						LOG("Response from TG was null");
					}
				}

				if (discord_webhook_url.valid()) {
					const auto ca_path = CA_CERT_PATH;
					http_client_type http_client(discord_webhook_url.host);

#if BUILD_OPENSSL
					http_client.set_ca_cert_path(ca_path.c_str());
					http_client.enable_server_certificate_verification(true);
#endif
					http_client.set_follow_location(true);
					http_client.set_read_timeout(5);
					http_client.set_write_timeout(5);

					auto items = discord_webhooks::form_player_connected(
						avatar,
						server_name,
						connected_player_nickname,
						all_nicknames,
						current_arena_name
					);

					auto response = http_client.Post(discord_webhook_url.location.c_str(), items);

					LOG("PUSH RESPONSE:");

					if (response) {
						LOG_NVPS(response->body);

						return discord_webhooks::find_attachment_url(response->body);
					}
					else {
						LOG("Response was null");
					}
				}

				return "";
			}, 
			id
		);
	}
}

void server_setup::finalize_webhook_jobs() {
	auto finalize = [&](auto& webhook_job) {
		if (is_ready(*webhook_job.job)) {
			if (auto client = find_client_state(webhook_job.player_id)) {
				client->uploaded_avatar_url = webhook_job.job->get();
			}

			return true;
		}

		return false;
	};

	erase_if(pending_jobs, finalize);
}

bool server_setup::respond_to_ping_requests(
	const netcode_address_t& from,
	const std::byte* packet_buffer,
	const std::size_t packet_bytes
) {
	auto handle = [&](const auto& typed_request) {
		using T = remove_cref<decltype(typed_request)>;

		if constexpr(std::is_same_v<T, gameserver_ping_request>) {
			const auto sequence = typed_request.sequence;
			LOG("Received ping request from: %x (sequence: %x / %f)", ::ToString(from), sequence, augs::bit_cast<double>(sequence));

			auto response = gameserver_ping_response();
			response.sequence = sequence;

			auto bytes = augs::to_bytes(response);
			server->send_udp_packet(from, bytes.data(), bytes.size());

			return true;
		}

		return false;
	};

	try {
		const auto request = read_gameserver_command(packet_buffer, packet_bytes);
		return std::visit(handle, request);
	}
	catch (const augs::stream_read_error&) {

	}

	return false;
}

uint32_t server_setup::get_num_slots() const {
	return last_start.slots;
}

uint32_t server_setup::get_num_connected() const {
	const auto& arena = get_arena_handle();

	return arena.on_mode_with_input(
		[&](const auto& mode, const auto&) {
			return mode.get_num_players();
		}
	);
}

uint32_t server_setup::get_num_active_players() const {
	const auto& arena = get_arena_handle();

	return arena.on_mode_with_input(
		[&](const auto& mode, const auto&) {
			return mode.get_num_active_players();
		}
	);
}

void server_setup::send_heartbeat_to_server_list() {
	ensure(resolved_server_list_addr.has_value());

	const auto& arena = get_arena_handle();

	server_heartbeat heartbeat;

	heartbeat.nat = nat_traversal.last_detected_nat;

	if (!vars.allow_nat_traversal) {
		heartbeat.nat.type = nat_type::PUBLIC_INTERNET;
	}

	heartbeat.server_name = get_server_name();
	heartbeat.current_arena = get_current_arena_name();
	heartbeat.game_mode = arena.on_mode_with_input(
		[&](const auto& mode, const auto& input) {
			return mode.get_name(input);
		}
	);

	heartbeat.suppress_new_community_server_webhook = 
		suppress_community_server_webhook_this_run ||
		vars.suppress_new_community_server_webhook
	;

	heartbeat.max_online = get_num_slots();
	heartbeat.internal_network_address = internal_address;

	heartbeat.num_online = get_num_connected();
	heartbeat.num_fighting = get_num_active_players();

	arena.on_mode_with_input(
		[&heartbeat](const auto& mode, const auto& input) {
			heartbeat.max_fighting = mode.get_max_num_active_players(input);
		}
	);

	heartbeat.server_version = hypersomnia_version().get_version_string();
	heartbeat.is_editor_playtesting_server = vars.playtesting_context.has_value();

	arena.on_mode_with_input(
		[&heartbeat](const auto& mode, const auto&) {
			auto fill = [&](const auto faction, auto& target) {
				thread_local std::vector<server_heartbeat_player_info> players;
				players.clear();

				mode.for_each_player_in(
					faction,
					[&](const auto, const auto& player) {
						auto converted = [](auto i) {
							return static_cast<uint8_t>(std::clamp(i, 0, 255));
						};

						players.push_back({ player.get_nickname(), converted(player.stats.calc_score()), converted(player.stats.deaths) });
					}
				);

				sort_range(players);
				target.assign(players.begin(), players.begin() + std::min(players.size(), target.max_size()));
			};

			fill(faction_type::RESISTANCE, heartbeat.players_resistance);
			fill(faction_type::METROPOLIS, heartbeat.players_metropolis);
			fill(faction_type::SPECTATOR, heartbeat.players_spectating);

			heartbeat.score_resistance = mode.get_faction_score(faction_type::RESISTANCE);
			heartbeat.score_metropolis = mode.get_faction_score(faction_type::METROPOLIS);
		}
	);

	heartbeat.validate();
	heartbeat_buffer.clear();

	{
		auto ss = augs::ref_memory_stream(heartbeat_buffer);
		augs::write_bytes(ss, masterserver_request(std::move(heartbeat)));
	}

	const auto heartbeat_bytes = heartbeat_buffer.size();

	if (heartbeat_bytes > 0) {
		const auto destination_address = resolved_server_list_addr.value();
		LOG("Sending heartbeat through UDP to %x (%x). Bytes: %x", ToString(to_yojimbo_addr(destination_address)), destination_address.type, heartbeat_bytes);

		server->send_udp_packet(destination_address, heartbeat_buffer.data(), heartbeat_bytes);
	}
}

void server_setup::resolve_server_list() {
	const auto& in = vars.notified_server_list;

	LOG("Requesting resolution of server_list address at %x", in.address);
	future_resolved_server_list_addr = async_resolve_address(in);
}

bool server_setup::server_list_enabled() const {
	return server->is_running() && vars.notified_server_list.address.size() > 0;
}

void server_setup::resolve_heartbeat_host_if_its_time() {
	if (!server_list_enabled()) {
		return;
	}

	auto& when_last = when_last_resolved_server_list_addr;

	if (valid_and_is_ready(future_resolved_server_list_addr)) {
		const auto result = future_resolved_server_list_addr.get();

		LOG(result.report());

		if (result.result == resolve_result_type::OK) {
			if (result.addr != resolved_server_list_addr) {
				request_immediate_heartbeat();
			}

			resolved_server_list_addr = result.addr; 
		}
	}

	if (future_resolved_server_list_addr.valid()) {
		when_last = server_time;
		return;
	}

	const auto since_last = server_time - when_last;
	const auto resolve_every = vars.resolve_server_list_address_once_every_secs;

	if (resolved_server_list_addr == std::nullopt || since_last >= resolve_every) {
		resolve_server_list();
		when_last = server_time;
	}
}

void server_setup::resolve_internal_address_if_its_time() {
	if (!server_list_enabled()) {
		return;
	}

	if (valid_and_is_ready(future_internal_address)) {
		auto new_address = future_internal_address.get();

		if (new_address.has_value()) {
			new_address->port = last_start.port;

			if (new_address != internal_address) {
				request_immediate_heartbeat();
			}

			internal_address = new_address;

			LOG("Resolved internal network address to be: %x", ::ToString(*new_address));
		}

		return;
	}

	auto& when_last = when_last_resolved_internal_address;

	const auto since_last = server_time - when_last;
	const auto resolve_every = vars.resolve_internal_address_once_every_secs;

	if (internal_address == std::nullopt || since_last >= resolve_every) {
		LOG("Requesting resolution of internal network address.");
		future_internal_address = async_get_internal_network_address();
		when_last = server_time;
	}
}

void server_setup::request_immediate_heartbeat() {
	when_last_sent_heartbeat_to_server_list = 0;
	LOG("Requesting immediate heartbeat to the server list.");
}

bool server_setup::has_sent_any_heartbeats() const {
	return when_last_sent_heartbeat_to_server_list != 0;
}

void server_setup::send_heartbeat_to_server_list_if_its_time() {
	if (!server_list_enabled()) {
		return;
	}

	auto& when_last = when_last_sent_heartbeat_to_server_list;

	if (resolved_server_list_addr == std::nullopt) {
		return;
	}

	const auto since_last = server_time - when_last;
	const auto send_every = vars.send_heartbeat_to_server_list_once_every_secs;

	const bool send_for_the_first_time = !has_sent_any_heartbeats();

	if (send_for_the_first_time || since_last >= send_every) {
		const auto times = when_last == 0 ? 4 : 1;

		for (int i = 0; i < times; ++i) {
			send_heartbeat_to_server_list();
		}

		when_last = server_time;
	}
}

template <class T, class F>
void server_setup::for_each_id_and_client_impl(T& self, F&& callback, const server_setup::for_each_flags flags) {
	for (auto& c : self.clients) {
		const auto client_id = static_cast<client_id_type>(index_in(self.clients, c));

		if (flags[for_each_flag::ONLY_CONNECTED]) {
			if (!c.is_set()) {
				continue;
			}
		}

		callback(client_id, c);
	}

	if (self.is_integrated()) {
		if (flags[for_each_flag::WITH_INTEGRATED]) {
			callback(self.get_integrated_client_id(), self.integrated_client);
		}
	}
}

template <class F>
void server_setup::for_each_id_and_client(F&& callback, const server_setup::for_each_flags flags) {
	for_each_id_and_client_impl(*this, std::forward<F>(callback), flags);
}

template <class F>
void server_setup::for_each_id_and_client(F&& callback, const server_setup::for_each_flags flags) const {
	for_each_id_and_client_impl(*this, std::forward<F>(callback), flags);
}

mode_player_id server_setup::get_integrated_player_id() const {
	return mode_player_id::machine_admin();
}

client_id_type server_setup::get_integrated_client_id() const {
	return static_cast<client_id_type>(get_integrated_player_id().value);
}

mode_player_id server_setup::to_mode_player_id(const client_id_type& id) {
	mode_player_id out;
	out.value = static_cast<mode_player_id::id_value_type>(id);

	return out;
}

std::optional<session_id_type> server_setup::find_session_id(const client_id_type& id) {
	return get_arena_handle().on_mode(
		[&](const auto& mode) -> std::optional<session_id_type> {
			if (const auto entry = mode.find(to_mode_player_id(id))) {
				return entry->get_session_id();
			}

			return std::nullopt;
		}
	);
}

online_arena_handle<false> server_setup::get_arena_handle() {
	return get_arena_handle_impl<online_arena_handle<false>>(*this);
}

online_arena_handle<true> server_setup::get_arena_handle() const {
	return get_arena_handle_impl<online_arena_handle<true>>(*this);
}

entity_id server_setup::get_controlled_character_id() const {
	return on_mode_with_input(
		[&](const auto& typed_mode, const auto& in) {
			(void)in;

			const auto local_id = get_local_player_id();
			const auto local_character = typed_mode.lookup(local_id);

			return local_character;
		}
	);
}

void server_setup::log_malicious_client(const client_id_type id) {
	LOG("Malicious client detected. Details:\n%x", describe_client(id));

#if !IS_PRODUCTION_BUILD
	ensure(false && "Client has sent some invalid data.");
#endif
}

std::string server_setup::find_client_nickname(const client_id_type& id) const {
	const auto& c = clients[id];

	if (!c.is_set()) {
		return {};
	}

	std::string nickname = "";

	get_arena_handle().on_mode(
		[&](const auto& mode) {
			if (const auto entry = mode.find(to_mode_player_id(id))) {
				nickname = ": " + entry->get_nickname();
			}
		}
	);

	if (nickname.empty()) {
		nickname = c.get_nickname();
	}

	return nickname;
}

std::string server_setup::describe_client(const client_id_type id) const {
	std::string nickname = find_client_nickname(id);

	return typesafe_sprintf(
		"Id: %x\nNickname%x",
		id,
		nickname.empty() ? std::string(" unknown") : std::string(": " + nickname)
	);
}

net_time_t server_setup::get_current_time() {
	return yojimbo_time();
}

void server_setup::customize_for_viewing(config_lua_table& config) const {
#if !IS_PRODUCTION_BUILD
	config.window.name = "Arena server";
#endif

	if (is_gameplay_on()) {
		get_arena_handle().adjust(config.drawing);
	}
}

void server_setup::apply(const server_private_vars& private_new_vars) {
	const auto old_vars = private_vars;
	private_vars = private_new_vars;
}

server_public_vars server_setup::make_public_vars() const {
	server_public_vars pub;

	pub.arena = vars.arena;
	pub.game_mode = vars.game_mode;

	pub.required_arena_hash = current_arena_hash;
	pub.playtesting_context = vars.playtesting_context;

	const bool is_user_project = ::begins_with(current_arena_folder.string(), EDITOR_PROJECTS_DIR.string());

	if (is_user_project) {
		pub.external_arena_files_provider.clear();
	}
	else {
		pub.external_arena_files_provider = vars.external_arena_files_provider;
	}

	return pub;
}

void server_setup::apply(const server_vars& new_vars, const bool first_time) {
	if (!first_time) {
		if (new_vars == vars) {
			return;
		}
	}

	const auto previous_arena = vars.arena;

	const bool reload_arena = first_time || vars.arena != new_vars.arena || vars.game_mode != new_vars.game_mode;
	const bool reload_net_sim = first_time || vars.network_simulator != new_vars.network_simulator;

	vars = new_vars;

	if (reload_arena) {
		try {
			rechoose_arena();
		}
		catch (const augs::json_deserialization_error& err) {
			LOG("Failed to load \"%x\":\n%x. Keepign the current arena.", vars.arena, err.what());

			vars.arena = previous_arena;
			rechoose_arena();
		}
		catch (const augs::file_open_error& err) {
			LOG("Arena named \"%x\" was not found on the server! Keeping the current arena.", new_vars.arena);

			vars.arena = previous_arena;
			rechoose_arena();
		}
		catch (...) {
			LOG("Failed to load \"%x\":\nUnknown error. Keeping the current arena.", vars.arena);

			vars.arena = previous_arena;
			rechoose_arena();
		}
	}

	{
		auto& rcon_gui = integrated_client_gui.rcon;
		rcon_gui.on_arrived(vars);
	}

	if (reload_net_sim) {
		server->set(vars.network_simulator);
	}

	auto broadcast_new_vars_to_rcons = [&](const auto recipient_id, auto&) {
		const auto rcon_level = get_rcon_level(recipient_id);

		if (rcon_level >= rcon_level_type::BASIC) {
			server->send_payload(
				recipient_id,
				game_channel_type::RELIABLE_MESSAGES,

				vars
			);
		}
	};

	for_each_id_and_client(broadcast_new_vars_to_rcons, only_connected_v);

	const auto new_public_vars = make_public_vars();

	if (first_time || last_broadcast_public_vars != new_public_vars) {
		last_broadcast_public_vars = new_public_vars;

		auto broadcast_new_vars = [&](const auto recipient_id, const auto& c) {
			if (c.should_pause_solvable_stream()) {
				return;
			}

			server->send_payload(
				recipient_id,
				game_channel_type::RELIABLE_MESSAGES,

				new_public_vars
			);
		};

		for_each_id_and_client(broadcast_new_vars, only_connected_v);
	}
}

void server_setup::apply(const config_lua_table& cfg) {
	/* 
		If we just applied the changes from game settings,
		RCON would not work on the integrated server and it would be confusing.
	*/

	/* apply(cfg.server); */

	integrated_client_vars = cfg.client;

	auto& current_requested_settings = integrated_client.settings.public_settings;

	auto requested_settings = current_requested_settings;
	requested_settings.character_input = cfg.input.character;

	const bool can_already_resend_settings = server_time - when_last_sent_admin_public_settings > 1.0;
	const bool resend_requested_settings = can_already_resend_settings && current_requested_settings != requested_settings;

	if (resend_requested_settings) {
		current_requested_settings = requested_settings;
		integrated_client.rebroadcast_public_settings = true;
	}
}

void register_external_resources_of(
	const editor_project& project,
	const augs::path_type& arena_folder_path,
	arena_files_database_type& database
) {
	project.resources.pools.for_each_container(
		[&]<typename P>(const P& pool) {
			using R = typename P::mapped_type;

			if constexpr(is_pathed_resource_v<R>) {
				for (auto& resource : pool) {
					const auto& file = resource.external_file;
					database[augs::to_secure_hash_byte_format(file.file_hash)] = { arena_folder_path / file.path_in_project, {} };
				}
			}
		}
	);
}

void server_setup::rechoose_arena() {
	LOG("Choosing arena: %x", vars.arena);

	const auto& arena = get_arena_handle();

	{
		const auto result = ::choose_arena_server({
			editor_project_readwrite::reading_settings(),
			lua,
			arena,
			official,
			vars.arena,
			vars.game_mode,
			clean_round_state,
			vars.playtesting_context,
			std::addressof(*last_loaded_project),
			nullptr
		});

		current_arena_folder = result.arena_folder_path;
		const auto paths = editor_project_paths(current_arena_folder);

		current_arena_hash = result.loaded_arena_hash;
			
		LOG("Chosen arena hash: %x", current_arena_hash);

		::register_external_resources_of(
			*last_loaded_project,
			current_arena_folder,
			arena_files_database
		);

		arena_files_database[current_arena_hash] = { paths.project_json, {} };
	}

	arena_gui.reset();
	arena_gui.choose_team.show = ::is_spectator(arena, get_local_player_id());

	integrated_client_gui.rcon.show = false;

	if (should_have_admin_character()) {
		const auto admin_id = get_integrated_player_id();

		if (!player_added_to_mode(admin_id)) {
			mode_entropy_general cmd;

			const auto& playtesting_context = vars.playtesting_context;
			auto admin_faction = faction_type::SPECTATOR;

			if (playtesting_context) {
				admin_faction = playtesting_context->first_player_faction;
				arena_gui.choose_team.show = false;
			}

			cmd.added_player = add_player_input {
				get_integrated_player_id(),
				integrated_client_vars.nickname,
				admin_faction
			};

			local_collected.clear();
			local_collected.control(cmd);
		}
	}

	request_immediate_heartbeat();
}

void server_setup::accept_game_gui_events(const game_gui_entropy_type& events) {
	control(events);
}

void server_setup::init_client(const client_id_type& id) {
	auto& new_client = clients[id];
	new_client.init(server_time);

	LOG("Client %x connected.", id);
}

void server_setup::unset_client(const client_id_type& id) {
	LOG("Client disconnected. Details:\n%x", describe_client(id));
	clients[id].unset();
}

void server_setup::disconnect_and_unset(const client_id_type& id) {
	server->disconnect_client(id);
	unset_client(id);
}

void server_setup::perform_automoves_to_spectators() {
	for (const auto& moved_mode_id : moved_to_spectators) {
		const auto choice = mode_commands::team_choice { faction_type::SPECTATOR };

		total_mode_player_entropy choice_entropy;
		choice_entropy.mode = choice;

		accept_entropy_of_client(moved_mode_id, choice_entropy);
	}

	moved_to_spectators.clear();
}

void server_setup::accept_entropy_of_client(
	const mode_player_id mode_id,
	const total_client_entropy& entropy
) {
	if (!entropy.empty()) {
		step_collected += { mode_id, entropy };
	}
}

void server_setup::send_full_arena_snapshot_to(const client_id_type client_id) {
	const auto sent_client_id = static_cast<uint32_t>(client_id);

	server->send_payload(
		client_id, 
		game_channel_type::RELIABLE_MESSAGES, 

		buffers,

		clean_round_state,
		scene.world.get_common_significant().flavours,

		full_arena_snapshot_payload<true> {
			scene.world.get_solvable().significant,
			current_mode_state,
			sent_client_id,
			get_rcon_level(client_id)
		}
	);
}

void server_setup::send_complete_solvable_state_to(const client_id_type client_id) {
	/*
		Three things: server public vars, client public settings, and the full state snapshot.
	*/

	server->send_payload(
		client_id, 
		game_channel_type::RELIABLE_MESSAGES, 

		last_broadcast_public_vars
	);

	send_full_arena_snapshot_to(client_id);

	auto download_existing_public_settings  = [this, recipient_client_id = client_id](const auto client_id_of_settings, auto& cc) {
		const auto downloaded_settings = make_public_settings_update_from(cc, client_id_of_settings);

		server->send_payload(
			recipient_client_id,
			game_channel_type::RELIABLE_MESSAGES,

			downloaded_settings
		);
	};

	for_each_id_and_client(download_existing_public_settings, connected_and_integrated_v);
}

void server_setup::advance_clients_state() {
	/* Do it only once per tick */
	bool added_someone_already = false;
	bool removed_someone_already = false;

	const auto inv_simulation_delta_ms = 1.0 / (get_inv_tickrate() * 1000.0);

	auto in_steps = [inv_simulation_delta_ms](const auto ms) {
		return static_cast<uint32_t>(ms * inv_simulation_delta_ms);
	};

	auto automove_to_spectators_if_afk = [&](const client_id_type client_id, auto& c) {
		if (c.should_move_to_spectators_due_to_afk(vars, server_time)) {
			const auto mode_id = to_mode_player_id(client_id);

			const auto moved_player_faction = get_arena_handle().on_mode(
				[&](const auto& typed_mode) {
					if (const auto data = typed_mode.find(mode_id)) {
						return data->get_faction();
					}

					return faction_type::SPECTATOR;
				}
			);

			if (moved_player_faction != faction_type::SPECTATOR) {
				moved_to_spectators.push_back(mode_id);
			}
		}
	};

	auto process_client = [&](const client_id_type client_id, auto& c) {
		using S = client_state_type;
		const auto mode_id = to_mode_player_id(client_id);

		if (c.is_set()) {
			{
				const auto num_commands = c.pending_entropies.size();
				const auto max_commands = vars.max_buffered_client_commands;

				if (num_commands > max_commands) {
					const auto reason = typesafe_sprintf("number of pending commands (%x) exceeded the maximum of %x.", num_commands, max_commands);
					kick(client_id, reason);
				}
			}

			if (!removed_someone_already) {
				if (c.when_kicked.has_value()) {
					const auto linger_secs = std::clamp(vars.max_kick_ban_linger_secs, 0.f, 15.f);

					if (server_time - *c.when_kicked > linger_secs) {
						LOG("Disconnecting kicked client %x.", client_id);
						disconnect_and_unset(client_id);
					}
				}
			}
		}

		if (!c.is_set()) {
			if (!removed_someone_already) {
				if (player_added_to_mode(mode_id)) {
					ensure(!removed_someone_already);

					mode_entropy_general cmd;
					cmd.removed_player = mode_id;

					local_collected.control(cmd);
					removed_someone_already = true;
				}
			}

			return;
		}

		auto contribute_to_step_entropy = [&]() {
#if 0
			c.pending_entropies.set_lower_limit(
				c.settings.net.requested_jitter_buffer_ms * inv_simulation_delta_ms;
			);
#endif

			const auto jitter_vars = c.settings.net.jitter;
			const auto jitter_squash_steps = std::max(jitter_vars.buffer_at_least_steps, in_steps(jitter_vars.buffer_at_least_ms));

			auto& inputs = c.pending_entropies;

			if (const auto num_pending = inputs.size(); num_pending > 0) {
				const bool should_squash = num_pending >= jitter_squash_steps;

				total_client_entropy entropy;

				c.num_entropies_accepted = [&]() {
					if (should_squash) {
						const auto num_squashed = static_cast<uint8_t>(
							std::min(
								num_pending,
								static_cast<std::size_t>(c.settings.net.jitter.max_commands_to_squash_at_once)
							)
						);

						for (std::size_t i = 0; i < num_squashed; ++i) {
							entropy += inputs[i];
						}

						if (num_squashed == num_pending) {
							inputs.clear();
						}
						else {
							erase_first_n(inputs, num_squashed);
						}

						return static_cast<uint8_t>(num_squashed);
					}

					entropy = inputs[0];
					erase_first_n(inputs, 1);

					return static_cast<uint8_t>(1);
				}();

				accept_entropy_of_client(mode_id, entropy);
			}
		};

		auto add_client_to_mode = [&]() {
			auto final_nickname = c.get_nickname(); 
			
			if (final_nickname.empty()) {
				return false;
			}

			mode_entropy_general cmd;

			cmd.added_player = add_player_input {
				mode_id,
				std::move(final_nickname),
				faction_type::SPECTATOR
			};

			local_collected.control(cmd);
			added_someone_already = true;

			return true;
		};

		auto send_state_for_the_first_time = [&]() {
			LOG("Sending initial payload for %x at step: %x", client_id, scene.world.get_total_steps_passed());

			send_complete_solvable_state_to(client_id);

			{
				auto download_existing_avatar = [this, recipient_client_id = client_id](const auto client_id_of_avatar, auto& cc) {
					const auto session_id_of_avatar = find_session_id(client_id_of_avatar);

					if (session_id_of_avatar) {
						server->send_payload(
							recipient_client_id,
							game_channel_type::RELIABLE_MESSAGES,

							*session_id_of_avatar,
							cc.meta.avatar
						);
					}
				};

				for_each_id_and_client(download_existing_avatar, connected_and_integrated_v);
			}

			const auto rcon_level = get_rcon_level(client_id);

			if (rcon_level >= rcon_level_type::BASIC) {
				server->send_payload(
					client_id, 
					game_channel_type::RELIABLE_MESSAGES, 

					vars
				);

				server->send_payload(
					client_id, 
					game_channel_type::RELIABLE_MESSAGES, 

					runtime_info
				);
			}
		};

		if (!added_someone_already) {
			if (c.state > client_state_type::PENDING_WELCOME) {
				if (!player_added_to_mode(mode_id)) {
					if (add_client_to_mode()) {
						if (c.state == S::WELCOME_ARRIVED) {
							send_state_for_the_first_time();

							c.state = S::RECEIVING_INITIAL_SNAPSHOT;
						}
					}
					else {
						LOG("Couldn't add client to the game mode. Disconnecting.");
						disconnect_and_unset(client_id);
						return;
					}
				}
			}
		}

		if (c.state == client_state_type::IN_GAME) {
			if (c.should_kick_due_to_afk(vars, server_time)) {
				kick(client_id, "AFK!");
			}

			automove_to_spectators_if_afk(client_id, c);
		}

		if (c.state > client_state_type::NETCODE_NEGOTIATING_CONNECTION) {
			if (c.should_kick_due_to_inactivity(vars, server_time)) {
				kick(client_id, "No messages arrived for too long!");
			}
		}

#if 0
		else if (c.state == S::RECEIVING_INITIAL_SNAPSHOT_CORRECTION) {
			if (!server->has_messages_to_send(client_id, game_channel_type::RELIABLE_MESSAGES)) {
				c.set_in_game(server_time);
			}

		}
#endif
		if (c.state == S::IN_GAME) {
			contribute_to_step_entropy();
		}
	};

	/* 
		Default means it will be iterated over all clients, even disconnected ones, 
		to the exception of the integrated client.
	*/

	for_each_id_and_client(process_client);

	{
		automove_to_spectators_if_afk(get_integrated_client_id(), integrated_client);
	}
}

void server_setup::broadcast_shutdown_message() {
	server_broadcasted_chat message;
	message.target = chat_target_type::SERVER_SHUTTING_DOWN;
	message.recipient_effect = recipient_effect_type::DISCONNECT;

	broadcast(message);

	handle_client_messages();

	for (int i = 0; i < 10; ++i) {
		server->send_packets();
	}
}

void server_setup::schedule_shutdown() {
	if (shutdown_scheduled) {
		return;
	}

	shutdown_scheduled = true;
	broadcast_shutdown_message();
}

template <class P>
message_handler_result server_setup::handle_rcon_payload(
	const rcon_level_type level,
	const P& typed_payload
) {
	using namespace rcon_commands;
	constexpr auto abort_v = message_handler_result::ABORT_AND_DISCONNECT;
	constexpr auto continue_v = message_handler_result::CONTINUE;

	if constexpr(std::is_same_v<P, match_command>) {
		local_collected.mode_general.special_command = typed_payload;

		return continue_v;
	}
	else if constexpr(std::is_same_v<P, special>) {
		if (level == rcon_level_type::BASIC) {
			if (typed_payload != special::REQUEST_RUNTIME_INFO) {
				/* 
					Basic RCON can only ask REQUEST_RUNTIME_INFO.
					The other tasks are administrative.
				*/

				LOG("Unauthorized RCON usage.");
				return abort_v;
			}
		}

		switch (typed_payload) {
			case special::SHUTDOWN: {
				LOG("Shutting down due to rcon's request.");
				schedule_shutdown();

				return continue_v;
			}

			case special::RESTART: {
				LOG("Restarting the server due to rcon's request.");
				schedule_shutdown();

				request_restart_after_shutdown = true;

				return continue_v;
			}

			case special::REQUEST_RUNTIME_INFO: 
				refresh_runtime_info_for_rcon();

				return continue_v;

			default:
				LOG("Unsupported rcon command.");
				return continue_v;
		}
	}
	else if constexpr(std::is_same_v<P, server_vars>) {
		LOG("New server vars from the client.");

		apply(typed_payload);

		return continue_v;
	}
	else if constexpr(std::is_same_v<P, std::monostate>) {
		return abort_v;
	}
	else {
		static_assert(always_false_v<P>, "Unhandled rcon command type!");
		return abort_v;
	}
}

void server_setup::handle_client_messages() {
	auto& message_handler = *this;
	server->advance(server_time, message_handler);
}

::public_settings_update server_setup::make_public_settings_update_from(
	const server_client_state& c,
	const client_id_type& id
) const {
	::public_settings_update update;

	update.subject_id = to_mode_player_id(id);
	update.new_settings = c.settings.public_settings;
	
	return update;
}

void server_setup::rebroadcast_public_settings() {
	auto rebroadcast_if_needed = [&](const auto client_id, auto& c) {
		if (!c.rebroadcast_public_settings) {
			return;
		}

		c.rebroadcast_public_settings = false;

		if (to_mode_player_id(client_id) == get_local_player_id()) {
			when_last_sent_admin_public_settings = server_time;
		}

		const auto broadcasted_update = make_public_settings_update_from(c, client_id);

		auto update_for_client = [this, &broadcasted_update](const auto recipient_client_id, const auto& c) {
			if (c.should_pause_solvable_stream()) {
				return;
			}

			server->send_payload(
				recipient_client_id, 
				game_channel_type::RELIABLE_MESSAGES,

				broadcasted_update
			);
		};

		for_each_id_and_client(update_for_client, only_connected_v);
	};

	for_each_id_and_client(rebroadcast_if_needed, connected_and_integrated_v);
}

void server_setup::broadcast_net_statistics() {
	const auto& interval = vars.send_net_statistics_update_once_every_secs;

	if (interval > 0 && server_time - when_last_sent_net_statistics > std::max(interval, 0.5f)) {
		net_statistics_update update;

		auto clamped = [](const auto value) {
			return static_cast<uint8_t>(std::clamp(value, 1, 255));
		};

		auto reread_stats = [&](const auto client_id, auto& c) {
			const auto clamped_ping = [&]() -> uint8_t {
				if (to_mode_player_id(client_id) == get_local_player_id()) {
					return 0;
				}

				const auto info = server->get_network_info(client_id);
				const auto rounded_ping = static_cast<int>(std::round(info.rtt_ms));
				return clamped(rounded_ping);
			}();

			c.meta.stats.ping = clamped_ping;

			if (c.downloading_status == downloading_type::NONE) {
				/* Set to 100% */
				c.meta.stats.download_progress = 255;
			}

			integrated_player_metas[client_id].stats = c.meta.stats;
		};

		auto fill_update = [&](const auto, const auto& c) {
			const auto ping = static_cast<uint8_t>(c.meta.stats.ping);
			const auto progress = c.meta.stats.download_progress;

			const auto entry = net_statistics_entry {
				ping,
				progress
			};

			update.stats.push_back(entry);
		};

		auto send_stats = [&](const auto client_id, const auto& c) {
			if (c.should_pause_solvable_stream()) {
				/* No point sending ping values either */
				return;
			}

			if (c.state != client_state_type::IN_GAME) {
				return;
			}

			server->send_payload(
				client_id,
				game_channel_type::VOLATILE_STATISTICS,

				update
			);
		};

		for_each_id_and_client(reread_stats, connected_and_integrated_v);
		for_each_id_and_client(fill_update, connected_and_integrated_v);

		for_each_id_and_client(send_stats, only_connected_v);

		when_last_sent_net_statistics = server_time;
	}
}

void server_setup::send_server_step_entropies(const compact_server_step_entropy& total_input) {
	networked_server_step_entropy total;
	total.payload = total_input;
	total.meta.reinference_necessary = reinference_necessary;
	total.meta.state_hash = [&]() -> decltype(total.meta.state_hash) {
		auto& ticks_remaining = ticks_until_sending_hash;

		if (ticks_remaining == 0) {
			ticks_remaining = vars.state_hash_once_every_tick;
			--ticks_remaining;

			const auto calculated_hash = get_arena_handle().get_cosmos().calculate_solvable_signi_hash<uint32_t>();
			return calculated_hash;
		}

		return std::nullopt;
	}();

	auto send_total_entropy = [&](const auto client_id, auto& c) {
		if (c.should_pause_solvable_stream()) {
			return;
		}

		const bool its_time_already = 
			c.state >= client_state_type::RECEIVING_INITIAL_SNAPSHOT
		;

		if (!its_time_already) {
			return;
		}

		{
			{
				prestep_client_context context;
				context.num_entropies_accepted = c.num_entropies_accepted;

#if CONTEXTS_SEPARATE
				server->send_payload(
					client_id, 
					game_channel_type::RELIABLE_MESSAGES,

					context
				);
#else
				total.context = context;
#endif
			}

			/* Reset the counter */
			c.num_entropies_accepted = 0;
		}

		/* TODO PERFORMANCE: only serialize the message once and multicast the same buffer to all clients! */
		server->send_payload(
			client_id,
			game_channel_type::RELIABLE_MESSAGES,

			total
		);
	};

	for_each_id_and_client(send_total_entropy, only_connected_v);
}

void server_setup::reinfer_if_necessary_for(const compact_server_step_entropy& entropy) {
	if (reinference_necessary || logically_set(entropy.general.added_player)) {
		LOG("Server: Added player or reinference_necessary. Will reinfer to sync.");
		cosmic::reinfer_solvable(get_arena_handle().get_cosmos());
		reinference_necessary = false;
	}
}

void server_setup::clean_unused_cached_files() {
	if (opened_arena_files.empty()) {
		return;
	}

	cached_currently_downloaded_files.clear();

	auto gather_downloaded = [&](const auto, auto& c) {
		if (c.now_downloading_file.has_value()) {
			cached_currently_downloaded_files.emplace(*c.now_downloading_file);
		}
	};

	for_each_id_and_client(gather_downloaded, connected_and_integrated_v);

	erase_if(
		opened_arena_files,
		[&](const auto& opened) {
			if (!found_in(cached_currently_downloaded_files, opened)) {
				arena_files_database[opened].free_opened_file();

				return true;
			}

			return false;
		}
	);
}

bool server_setup::send_file_chunk(const client_id_type client_id, const arena_files_database_entry& entry, const file_chunk_index_type chunk_index) {
	const auto& c = clients[client_id];

	const auto& bytes = entry.cached_file;
	const auto bytes_n = bytes.size();

	auto num_all_chunks = bytes_n / file_chunk_size_v;

	if (bytes_n % file_chunk_size_v != 0) {
		++num_all_chunks;
	}

	if (bytes_n == 0) {
		num_all_chunks = 1;
	}

	file_chunk_packet packet;
	packet.index = chunk_index;
	packet.file_hash = *c.now_downloading_file;

	const auto last_chunk_index = num_all_chunks - 1;

	if (chunk_index <= last_chunk_index) {
		const bool is_last_chunk = chunk_index == last_chunk_index;

		const auto bytes_start = 							std::size_t(chunk_index) * file_chunk_size_v;
		const auto bytes_end   = is_last_chunk ? bytes_n : (std::size_t(bytes_start) + file_chunk_size_v);

		ensure(bytes_end <= bytes_n);

		const auto bytes_copied = bytes_end - bytes_start;

		if (bytes_copied != 0) {
			std::memcpy(packet.chunk_bytes.data(), bytes.data() + bytes_start, bytes_copied);
		}

		if (auto s = find_underlying_socket()) {
			auto address = to_netcode_addr(server->get_client_address(client_id));

			const auto num_packet_bytes = sizeof(packet);
			// LOG("sending chunk %x to %x (%x bytes). CMD: %x", chunk_index, ::ToString(address), num_packet_bytes, packet.command);
			auto socket = *s;
			netcode_socket_send_packet(&socket, &address, reinterpret_cast<void*>(&packet), num_packet_bytes);

			return true;
		}
	}

	return false;
}

file_chunk_index_type server_setup::calc_num_chunks_per_tick_per_downloader() const {
	const auto target_bandwidth = vars.max_direct_file_bandwidth * 1024 * 1024;
	const auto target_bandwidth_per_tick = target_bandwidth * get_inv_tickrate();

	const auto chunks_per_tick = target_bandwidth_per_tick / file_chunk_size_v;

	int num_downloaders = 0;

	for_each_id_and_client([&num_downloaders](const auto, auto& c){
		if (c.downloading_status == downloading_type::DIRECTLY) {
			++num_downloaders;
		}
	}, connected_and_integrated_v);

	if (num_downloaders == 0) {
		return 0;
	}

	const auto chunks_per_tick_per_downloader = std::max(uint32_t(1), uint32_t(chunks_per_tick / num_downloaders));

	return file_chunk_index_type(chunks_per_tick_per_downloader);
}

void server_setup::refresh_available_direct_download_bandwidths() {
	const auto num_chunks = calc_num_chunks_per_tick_per_downloader();

	auto refresh_chunks = [&](const auto, auto& c) {
		if (c.downloading_status == downloading_type::DIRECTLY) {
			c.direct_file_chunks_left = num_chunks;
		}
		else {
			c.direct_file_chunks_left = 0;
		}
	};

	for_each_id_and_client(refresh_chunks, connected_and_integrated_v);
}

void server_setup::send_packets_if_its_time() {
	auto& ticks_remaining = ticks_until_sending_packets;

	if (ticks_remaining == 0) {
		server->send_packets();

		ticks_remaining = vars.send_packets_once_every_tick;
		--ticks_remaining;
	}
}

double server_setup::get_inv_tickrate() const {
	return get_arena_handle().get_inv_tickrate();
}

double server_setup::get_audiovisual_speed() const {
	return get_arena_handle().get_audiovisual_speed();
}

custom_imgui_result server_setup::perform_custom_imgui(const perform_custom_imgui_input in) {
	if (!server->is_running()) {
		using namespace augs::imgui;

		center_next_window(vec2::square(0.3f), ImGuiCond_FirstUseEver);

		const auto window_name = "Connection status";
		auto window = scoped_window(window_name, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

		if (failure_reason.size() > 0) {
			text(failure_reason);
		}
		else {
			const auto addr = yojimbo::Address(last_start.ip.c_str(), last_start.port);

			{
				char buffer[256];
				addr.ToString(buffer, sizeof(buffer));
				text("Failed to host the server at address: %x", buffer);
			}
		}

		text("\n");
		ImGui::Separator();

		if (ImGui::Button("Go back")) {
			return quit_playtesting_or(custom_imgui_result::GO_TO_MAIN_MENU);
		}
	}
	else {
		if (is_integrated()) {
			auto& chat = integrated_client_gui.chat;

			if (chat.perform_input_bar(integrated_client_vars.client_chat)) {
				server_broadcasted_chat message;

				const auto session_id = find_session_id(get_integrated_client_id());
				ensure(session_id.has_value());

				message.author = *session_id;
				message.message = std::string(chat.current_message);
				message.target = chat.target;

				broadcast(message);

				chat.current_message.clear();
			}

			auto& rcon_gui = integrated_client_gui.rcon;

			if (!arena_gui.scoreboard.show && rcon_gui.show) {
				auto on_new_payload = [&](const auto& new_payload) {
					handle_rcon_payload(rcon_level_type::MASTER, new_payload);
				};

				const bool has_maintenance = false;

				rcon_gui.level = rcon_level_type::MASTER;

				::perform_rcon_gui(
					rcon_gui,
					has_maintenance,
					on_new_payload
				);
			}
		}
	}

	return quit_playtesting_or(arena_gui_base::perform_custom_imgui(in));
}

setup_escape_result server_setup::escape() {
	if (!is_gameplay_on()) {
		return quit_playtesting_or(setup_escape_result::GO_TO_MAIN_MENU);
	}

	if (integrated_client_gui.rcon.escape()) {
		return setup_escape_result::JUST_FETCH;
	}

	return quit_playtesting_or(arena_gui_base::escape());
}

bool server_setup::is_gameplay_on() const {
	return is_running();
}

bool server_setup::is_running() const {
	return server->is_running();
}

bool server_setup::should_have_admin_character() const {
	return is_integrated();
}

void server_setup::sleep_until_next_tick() {
	const auto sleep_dt = server_time - get_current_time();

	if (sleep_dt > 0.0) {
		const auto mult = std::clamp(vars.sleep_mult, 0.f, 0.9f);

		if (mult > 0.f) {
			yojimbo_sleep(static_cast<float>(sleep_dt) * mult);
		}
	}
}

void server_setup::update_stats(server_network_info& info) const {
	info = server->get_server_network_info();
}

server_client_state* server_setup::find_client_state(const mode_player_id id) {
	if (id.is_set()) {
		return std::addressof(get_client_state(id));
	}

	return nullptr;
}

const server_client_state* server_setup::find_client_state(const std::string& nickname) const {
	const server_client_state* result = nullptr;

	auto check_nickname = [&](const auto, const auto& c) {
		if (c.get_nickname() == nickname) {
			result = std::addressof(c);

			return callback_result::ABORT;
		}

		return callback_result::CONTINUE;
	};

	for_each_id_and_client(check_nickname, connected_and_integrated_v);

	return result;
}

const server_client_state* server_setup::find_client_state(const mode_player_id id) const {
	if (id.is_set()) {
		return std::addressof(get_client_state(id));
	}

	return nullptr;
}

server_client_state& server_setup::get_client_state(const mode_player_id id) {
	if (id == get_integrated_player_id()) {
		return integrated_client;
	}

	return clients[id.value];
}

const server_client_state& server_setup::get_client_state(const mode_player_id id) const {
	if (id == get_integrated_player_id()) {
		return integrated_client;
	}

	return clients[id.value];
}

std::vector<std::string> server_setup::get_all_nicknames() const {
	std::vector<std::string> nicknames;

	auto add_nick = [&nicknames](const auto, const auto& c) {
		nicknames.push_back(c.get_nickname());
	};

	for_each_id_and_client(add_nick, connected_and_integrated_v);

	return nicknames;
}

const server_name_type& server_setup::get_server_name() const {
	return vars.server_name;
}

std::string server_setup::get_current_arena_name() const {
	return vars.arena;
}

server_step_entropy server_setup::unpack(const compact_server_step_entropy& n) const {
	auto mode_id_to_entity_id = [&](const mode_player_id& mode_id) {
		return get_arena_handle().on_mode(
			[&](const auto& typed_mode) {
				return typed_mode.lookup(mode_id);
			}
		);
	};

	auto get_settings_for = [&](const mode_player_id& mode_id) {
		return get_client_state(mode_id).settings.public_settings.character_input;
	};

	return n.unpack(mode_id_to_entity_id, get_settings_for);
}

augs::path_type server_setup::get_unofficial_content_dir() const {
	return current_arena_folder;
}

bool safe_equal(const decltype(requested_client_settings::rcon_password)& candidate_password, const std::string& actual_password) {
	const bool rcon_is_disabled = actual_password.empty();

	if (rcon_is_disabled) {
		return false;
	}

	int matches = 0;

	const auto candidate_n = static_cast<std::size_t>(candidate_password.size());
	const auto actual_n = actual_password.size();

	for (std::size_t i = 0; i < std::min(candidate_n, actual_n); ++i) {
		if (candidate_password.data()[i] == actual_password[i]) {
			matches++;
		}
		else {
			matches--;
		}
	}

	return matches == static_cast<int>(actual_password.size());
}

netcode_address_t to_netcode_addr(const yojimbo::Address& t);
bool is_internal(const netcode_address_t& address);

rcon_level_type server_setup::get_rcon_level(const client_id_type& id) const { 
	if (is_integrated()) {
		if (id == get_integrated_client_id()) {
			return rcon_level_type::MASTER;
		}

		return rcon_level_type::INTEGRATED_ONLY;
	}

	const auto& c = clients[id];

	if (vars.auto_authorize_loopback_for_rcon) {
		if (server->get_client_address(id).IsLoopback()) {
			LOG("Auto-authorizing the loopback client for master rcon.");
			return rcon_level_type::MASTER;
		}
	}

	if (vars.auto_authorize_internal_for_rcon) {
		if (is_internal(to_netcode_addr(server->get_client_address(id)))) {
			LOG("Auto-authorizing the internal network client %x for master rcon.", ::ToString(server->get_client_address(id)));
			return rcon_level_type::MASTER;
		}
	}

	if (::safe_equal(c.settings.rcon_password, private_vars.master_rcon_password)) {
		LOG("Authorized the remote client for master rcon.");
		return rcon_level_type::MASTER;
	}

	if (::safe_equal(c.settings.rcon_password, private_vars.rcon_password)) {
		if (private_vars.master_rcon_password.empty()) {
			LOG("Authorized the remote client for master rcon.");
			return rcon_level_type::MASTER;
		}

		LOG("Authorized the remote client for basic rcon.");
		return rcon_level_type::BASIC;
	}

	LOG("RCON disabled for this client.");

	return rcon_level_type::DENIED;
}

void server_setup::broadcast(const ::server_broadcasted_chat& payload, const std::optional<client_id_type> except) {
	std::string sender_player_nickname;
	auto sender_player_faction = faction_type::SPECTATOR;

	get_arena_handle().on_mode(
		[&](const auto& typed_mode) {
			if (const auto entry = typed_mode.find(payload.author)) {
				sender_player_faction = entry->get_faction();
				sender_player_nickname = entry->get_nickname();
			}
		}
	);

	bool integrated_received = false;

	const auto new_entry = chat_gui_entry::from(
		payload,
		get_current_time(),
		sender_player_nickname,
		sender_player_faction
	);

	auto send_it = [&](const auto recipient_client_id, auto&) {
		if (except.has_value() && *except == recipient_client_id) {
			return;
		}

		if (payload.target == chat_target_type::TEAM_ONLY) {
			const auto recipient_player_faction = get_arena_handle().on_mode(
				[&](const auto& typed_mode) {
					if (const auto entry = typed_mode.find(to_mode_player_id(recipient_client_id))) {
						return entry->get_faction();
					}

					return faction_type::SPECTATOR;
				}
			);

			if (sender_player_faction != recipient_player_faction) {
				return;
			}
		}

		if (to_mode_player_id(recipient_client_id) == get_integrated_player_id()) {
			integrated_received = true;
		}
		else {
			server->send_payload(
				recipient_client_id,
				game_channel_type::RELIABLE_MESSAGES,

				payload
			);
		}
	};

	for_each_id_and_client(send_it, connected_and_integrated_v);

	if (integrated_received) {
		integrated_client_gui.chat.add_entry(std::move(new_entry));
	}

	if (is_dedicated() || integrated_received) {
		LOG("Server: %x", new_entry.operator std::string());
	}
}

message_handler_result server_setup::abort_or_kick_if_debug(const client_id_type& id, const std::string& reason) {
#if IS_PRODUCTION_BUILD
	LOG(find_client_nickname(id) + " was forcefully disconnected the server. Reason: %x", reason);
	return message_handler_result::ABORT_AND_DISCONNECT;
#else
	kick(id, reason);
	return message_handler_result::CONTINUE;
#endif
}

void server_setup::kick(const client_id_type& kicked_id, const std::string& reason) {
	auto& c = clients[kicked_id];

	LOG_NVPS(reason);

	if (!c.is_set()) {
		return;
	}

	if (c.when_kicked.has_value()) {
		return;
	}


	c.when_kicked = server_time;

	server_broadcasted_chat message;
	message.message = reason;
	message.target = chat_target_type::KICK;

	if (const auto session_id = find_session_id(kicked_id)) {
		message.author = *session_id;
	}

	const auto except = kicked_id;
	broadcast(message, except);

	message.recipient_effect = recipient_effect_type::DISCONNECT;

	server->send_payload(
		kicked_id,
		game_channel_type::RELIABLE_MESSAGES,

		message
	);
}

void server_setup::ban(const client_id_type& id, const std::string& reason) {
	// TODO!!!
	(void)id;
	(void)reason;
}

std::optional<arena_player_metas> server_setup::get_new_player_metas() {
	if (rebuild_player_meta_viewables) {
		auto& metas = integrated_player_metas;
		
		auto make_meta = [&](const auto client_id, const auto& cc) {
			metas[client_id].avatar.image_bytes = cc.meta.avatar.image_bytes;
		};

		for_each_id_and_client(make_meta, connected_and_integrated_v);

		rebuild_player_meta_viewables = false;
		return metas;
	}

	return std::nullopt;
}

const arena_player_metas* server_setup::find_player_metas() const {
	return std::addressof(integrated_player_metas);
}
	
bool server_setup::handle_input_before_game(
	const handle_input_before_game_input in
) {
	ensure(is_integrated());

	if (arena_gui_base::handle_input_before_game(in)) {
		return true;
	}

	if (integrated_client_gui.control(in)) {
		return true;
	}

	return false;
}

void server_setup::draw_custom_gui(const draw_setup_gui_input& in) const {
	ensure(is_integrated());

	using namespace augs::gui::text;

	integrated_client_gui.chat.draw_recent_messages(
		in.get_drawer(),
		integrated_client_vars.client_chat,
		in.config.faction_view,
		in.gui_fonts.gui,
		get_current_time()
	);

	arena_gui_base::draw_custom_gui(in);
}

bool server_setup::is_integrated() const {
	return dedicated == std::nullopt;
}

bool server_setup::is_dedicated() const {
	return dedicated.has_value();
}

void server_setup::handle_new_session(const add_player_input&) {
	rebuild_player_meta_viewables = true;
}

void server_setup::log_performance() {
	if (is_dedicated()) {
		const auto s = vars.log_performance_once_every_secs;

		if (s > 0) {
			const auto once_every = std::max(s, 0.5f);

			if (server_time - last_logged_at >= once_every) {
				profiler.prepare_summary_info();

				const auto summary = typesafe_sprintf(
					"S: %3f, SS: %3f, AA: %3f, ACS: %3f, SE: %3f, SP: %3f",
					1000 * profiler.step.get_summary_info().value,
					1000 * profiler.solve_simulation.get_summary_info().value,
					1000 * profiler.advance_adapter.get_summary_info().value,
					1000 * profiler.advance_clients_state.get_summary_info().value,
					1000 * profiler.send_entropies.get_summary_info().value,
					1000 * profiler.send_packets.get_summary_info().value
				);

				last_logged_at = server_time;
				LOG(summary);
			}
		}
	}
}

bool server_setup::requires_cursor() const {
	return arena_gui_base::requires_cursor() || integrated_client_gui.requires_cursor();
}

bool server_setup::player_added_to_mode(const mode_player_id mode_id) const {
	return found_in(get_arena_handle(), mode_id);
}

const netcode_socket_t* server_setup::find_underlying_socket() const {
	return server->find_underlying_socket();
}

void server_setup::refresh_runtime_info_for_rcon() {
	auto& out_entries = runtime_info.arenas_on_disk;
	out_entries.clear();

	auto add_from = [&](const auto& root) {
		try {
			augs::for_each_in_directory(
				root,
				[&](const auto& p) {
					out_entries.push_back({ std::filesystem::relative(p, root).string() });
					return callback_result::CONTINUE;
				},
				[](const auto&) { return callback_result::CONTINUE; }
			);
		}
		catch (...) {

		}
	};

	add_from(OFFICIAL_ARENAS_DIR);
	add_from(DOWNLOADED_ARENAS_DIR);
	add_from(EDITOR_PROJECTS_DIR);

	auto broadcast_new_info_to_rcons = [&](const auto recipient_id, auto&) {
		const auto rcon_level = get_rcon_level(recipient_id);

		if (rcon_level >= rcon_level_type::BASIC) {
			server->send_payload(
				recipient_id,
				game_channel_type::RELIABLE_MESSAGES,

				runtime_info
			);
		}
	};

	LOG("Refreshed runtime info. Arenas on disk: %x", out_entries.size());

	for_each_id_and_client(broadcast_new_info_to_rcons, only_connected_v);
}

void server_setup::set_client_is_downloading_files(const client_id_type client_id, server_client_state& c, const downloading_type type) {
	if (type > c.downloading_status) {
		server_broadcasted_chat message;

		message.target = 
			type == downloading_type::DIRECTLY ? 
			chat_target_type::DOWNLOADING_FILES_DIRECTLY : 
			chat_target_type::DOWNLOADING_FILES
		;

		if (const auto session_id = find_session_id(client_id)) {
			message.author = *session_id;

			const auto except = client_id;
			broadcast(message, except);
		}
	}

	c.downloading_status = type;
}


#include "augs/readwrite/to_bytes.h"

// TODO: rewrite unit tests to use streams since we're no longer using preserialized_message 

#undef BUILD_UNIT_TESTS
#if BUILD_UNIT_TESTS
#include <Catch/single_include/catch2/catch.hpp>
#include "augs/misc/lua/lua_utils.h"
#include <sol/sol.hpp>
#include "augs/readwrite/lua_file.h"

TEST_CASE("NetSerialization EmptyEntropies") {
	{
		net_messages::server_step_entropy ss;
		ss.Release();

		{
			REQUIRE(ss.bytes.size() == 0);

			networked_server_step_entropy sent;
			ss.write_payload(sent);

			/* One byte for num of entropies accepted, second byte for signifying empty entropy */
			REQUIRE(ss.bytes.size() == 2);
		}
	}

	{
		net_messages::server_step_entropy ss;
		ss.Release();

		{
			REQUIRE(ss.bytes.size() == 0);

			networked_server_step_entropy sent;
			sent.meta.state_hash = 0xdeadbeef;
			ss.write_payload(sent);

			/* One byte for num of entropies accepted, second byte for signifying empty entropy and existent hash, additional bytes for hash */
			REQUIRE(ss.bytes.size() == 2 + sizeof(decltype(sent.meta.state_hash)::value_type));
		}
	}

	{
		net_messages::client_entropy ss;
		ss.Release();

		{
			REQUIRE(ss.bytes.size() == 0);

			total_client_entropy sent;
			ss.write_payload(sent);

			REQUIRE(ss.bytes.size() == 1);
		}
	}
}

TEST_CASE("NetSerialization ClientEntropy") {
	net_messages::client_entropy ss;
	ss.Release();

	total_client_entropy sent;

	const auto naive_bytes = [&]() {
		sent.mode = mode_commands::team_choice { faction_type::RESISTANCE };
		sent.mode = mode_commands::spell_purchase { spell_id::of<haste>() };

		sent.cosmic.cast_spell.set<ultimate_wrath_of_the_aeons>();
		sent.cosmic.motions[game_motion_type::MOVE_CROSSHAIR] = { -2047, 2048 };
		sent.cosmic.intents.push_back({ game_intent_type::INTERACT, intent_change::PRESSED });
		sent.cosmic.intents.push_back({ game_intent_type::MOVE_FORWARD, intent_change::RELEASED });

		ss.write_payload(sent);

		return augs::to_bytes(sent);	
	}();

	total_client_entropy received;
	REQUIRE(ss.read_payload(received));

	const auto naively_received = augs::from_bytes<total_client_entropy>(naive_bytes);

	if (!(received == sent) || !(received == naively_received)) {
		auto lua = augs::create_lua_state();

		augs::save_as_lua_table(lua, sent, "sent.lua");
		augs::save_as_lua_table(lua, received, "received.lua");
	}

	REQUIRE(received.cosmic.motions.at(game_motion_type::MOVE_CROSSHAIR) == naively_received.cosmic.motions.at(game_motion_type::MOVE_CROSSHAIR));
	REQUIRE(received == naively_received);
	REQUIRE(received == sent);

	const auto naive_bytes_of_received = augs::to_bytes(received);
	REQUIRE(naive_bytes_of_received == naive_bytes);
}

TEST_CASE("NetSerialization ServerEntropy") {
	net_messages::server_step_entropy ss;
	ss.Release();
	
	networked_server_step_entropy sent;

	const auto naive_bytes = [&]() {
		total_mode_player_entropy t;
		t.mode = mode_commands::team_choice { faction_type::METROPOLIS };
		t.mode = mode_commands::spell_purchase { spell_id::of<fury_of_the_aeons>() };

		total_mode_player_entropy tt;
		tt.cosmic.cast_spell.set<ultimate_wrath_of_the_aeons>();
		tt.cosmic.motions[game_motion_type::MOVE_CROSSHAIR] = { 1, 1 };
		tt.cosmic.intents.push_back({ game_intent_type::INTERACT, intent_change::PRESSED });
		tt.cosmic.intents.push_back({ game_intent_type::MOVE_FORWARD, intent_change::RELEASED });

		auto second = mode_player_id::first();
		second.value++;

		auto third = second;
		third.value++;

		sent.payload.players.push_back({ mode_player_id::machine_admin(), t });
		sent.payload.players.push_back({ second, {} });
		sent.payload.players.push_back({ third, tt });
		sent.payload.players.push_back({ mode_player_id::first(), {} });

		REQUIRE(ss.write_payload(sent));

		return augs::to_bytes(sent);	
	}();

	networked_server_step_entropy received;
	const auto naively_received = augs::from_bytes<networked_server_step_entropy>(naive_bytes);

	REQUIRE(ss.read_payload(received));

	if (!(received == sent) || !(received == naively_received)) {
		auto lua = augs::create_lua_state();

		augs::save_as_lua_table(lua, sent, "sent.lua");
		augs::save_as_lua_table(lua, received, "received.lua");
	}

	REQUIRE(received == naively_received);
	REQUIRE(received == sent);

	const auto naive_bytes_of_received = augs::to_bytes(received);
	REQUIRE(naive_bytes_of_received == naive_bytes);
}

TEST_CASE("NetSerialization ServerEntropySecond") {
	net_messages::server_step_entropy ss;
	ss.Release();

	networked_server_step_entropy sent;
	sent.meta.state_hash = 0xdeadbeef;
	sent.meta.reinference_necessary = true;

	const auto naive_bytes = [&]() {
		total_mode_player_entropy t;

		item_flavour_id bought_item;
		bought_item.type_id.set<shootable_weapon>();
		bought_item.raw.indirection_index = 11;
		bought_item.raw.version  = 11;

		t.mode = bought_item;

		total_mode_player_entropy tt;
		tt.cosmic.cast_spell.set<ultimate_wrath_of_the_aeons>();
		tt.cosmic.motions[game_motion_type::MOVE_CROSSHAIR] = { -127, 128 };
		tt.cosmic.intents.push_back({ game_intent_type::DROP, intent_change::RELEASED });
		tt.cosmic.intents.push_back({ game_intent_type::DROP, intent_change::RELEASED });
		tt.cosmic.intents.push_back({ game_intent_type::DROP, intent_change::RELEASED });

		auto second = mode_player_id::first();
		second.value++;

		auto third = second;
		third.value++;

		sent.payload.players.push_back({ mode_player_id::machine_admin(), {} });
		sent.payload.players.push_back({ second, tt });
		sent.payload.players.push_back({ mode_player_id::first(), tt });
		sent.payload.players.push_back({ third, {} });

		sent.payload.general.added_player.id = mode_player_id::machine_admin();
		sent.payload.general.added_player.name = "proplayerrrrrrrrrr";
		sent.payload.general.added_player.faction = faction_type::DEFAULT;
		sent.payload.general.removed_player = third;
		sent.payload.general.special_command = match_command::RESTART_MATCH;

		REQUIRE(ss.write_payload(sent));

		return augs::to_bytes(sent);	
	}();

	networked_server_step_entropy received;
	const auto naively_received = augs::from_bytes<networked_server_step_entropy>(naive_bytes);

	REQUIRE(ss.read_payload(received));

	if (!(received == sent) || !(received == naively_received)) {
		auto lua = augs::create_lua_state();

		augs::save_as_lua_table(lua, sent, "sent.lua");
		augs::save_as_lua_table(lua, received, "received.lua");
	}

	REQUIRE(received == naively_received);
	REQUIRE(received == sent);
}

#endif
