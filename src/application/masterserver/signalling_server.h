#pragma once
#include <cstdint>
#include <nlohmann/json.hpp>
#include "rtc/rtc.hpp"

#include "augs/string/path_sanitization.h"
#include "augs/templates/bit_cast.h"

void yojimbo_random_bytes( uint8_t * data, int bytes );

class signalling_server {
public:
	struct signalling_peer {
		std::shared_ptr<rtc::WebSocket> ws;
		netcode_address_t ip_informational = {};

		double time_hosted = 0.0;
		double time_last_heartbeat = -1;
		double time_last_correct_message = -1;
		server_heartbeat last_heartbeat;

		// TODO_SECURITY: use proper secure channels
		std::unordered_set<uint64_t> recvd_messages;

		bool is_server() const {
			return time_last_heartbeat != -1.0;
		}
	};

	using peer_map = std::unordered_map<
		std::string,
		signalling_peer
	>;

private:
	std::mutex peers_mutex;
	std::mutex active_connections_mutex;
	std::mutex aliases_mutex;

	double peer_timeout;
	rtc::WebSocketServer server;

	peer_map peers_unsafe;

	template <class F>
	decltype(auto) on_peers(F callback) {
		std::scoped_lock lk(peers_mutex);
		return callback(peers_unsafe);
	}

	template <class F>
	decltype(auto) on_peers(F callback) const {
		std::scoped_lock lk(peers_mutex);
		return callback(peers_unsafe);
	}

	/*
		active_connections is kept to prevent sockets going out of scope until assigned an id.
	*/

    std::unordered_set<std::shared_ptr<rtc::WebSocket>> active_connections;
	std::unordered_map<std::string, std::string> webrtc_id_aliases;

	std::atomic<bool> dirty = false;

	struct pending_signalling_packet {
		uint64_t guid = 0;
		std::string message;
		int times = 5;
		netcode_address_t target;
	};

	double when_last_processed_timeouts = -1;
	double when_last_sent_packets = -1;
	std::vector<pending_signalling_packet> pending_packets;

	void setup_server_callbacks() {
		server.onClient([this](std::shared_ptr<rtc::WebSocket> client) {
			LOG("New client connected from address: %x", client->remoteAddress());

			{
				std::scoped_lock lk(active_connections_mutex);
				active_connections.emplace(client);
			}

			client->onOpen([this, wclient = std::weak_ptr(client)]() {
				if(auto client = wclient.lock()) {
					on_peers([&](auto& peers) {
						register_web_peer(
							peers,
							client,
							client->path(),
							client->remoteAddress()
						);
					});
				}
			});

			client->onClosed([this, wclient = std::weak_ptr(client)]() {
				if (auto client = wclient.lock()) {
					LOG("(onClosed) Address: %x", client->remoteAddress());

					on_peers([&](auto& peers) {
						remove_web_peer(peers, client);
					});
				}
			});

			client->onError([this, wclient = std::weak_ptr(client)](const std::string& err) {
				LOG("Client WebSocket error: %x", err);

				if (auto client = wclient.lock()) {
					LOG("(onError) Address: %x", client->remoteAddress());

					on_peers([&](auto& peers) {
						remove_web_peer(peers, client);
					});
				}
			});

			client->onMessage([this, wclient = std::weak_ptr(client)](std::variant<rtc::binary, std::string> data) {
				if(auto client = wclient.lock()) {
					if (std::holds_alternative<std::string>(data)) {
						try {
							const auto str = std::get<std::string>(data);
							auto json_message = nlohmann::json::parse(str);

							on_peers(
								[&](auto& peers) {
									relay_message(peers, json_message, get_webrtc_id(peers, client));
								}
							);
						}
						catch (...) {

						}
					}
					else if (std::holds_alternative<rtc::binary>(data)) {
						try {
							const auto bytes = std::get<rtc::binary>(data);
							const auto hb = augs::from_bytes<server_heartbeat>(bytes);

							on_peers(
								[&](auto& peers) {
									const auto webrtc_id = get_webrtc_id(peers, client);

									if (const auto peer = mapped_or_nullptr(peers, webrtc_id)) {
										const auto unix_time = augs::date_time::secs_since_epoch();

										LOG("Received heartbeat from Web peer: %x", webrtc_id);

										if (!peer->is_server()) {
											peer->time_hosted = unix_time;
										}

										peer->last_heartbeat = hb;
										peer->time_last_heartbeat = unix_time;
										peer->time_last_correct_message = augs::high_precision_secs();

										dirty = true;
									}
									else {
										LOG("Wrong peer id: %x", webrtc_id);
									}
								}
							);
						}
						catch (...) {

						}
					}
				}
			});
		});
	}

	static std::string formatHex(int number, int width) {
		std::stringstream ss;
		ss << std::setw(width) << std::setfill('0') << std::hex << number;
		return ss.str();
	}

	static webrtc_id_type find_free_random_guid(const peer_map& list) {
		std::array<uint8_t, 32> guid;
		std::string str;

		do {
			yojimbo_random_bytes(reinterpret_cast<uint8_t*>(&guid), sizeof(guid));
			str = augs::to_hex_format(guid);
		}
		while (found_in(list, str));

		return str;
	}

	static webrtc_id_type find_free_id(const peer_map& list) {
		for (int i = 0; i <= 0xFF; ++i) {
			webrtc_id_type id = formatHex(i, 2);
			if (list.find(id) == list.end()) {
				return id;
			}
		}

		for (int i = 0x100; i <= 0xFFFF; ++i) {
			webrtc_id_type id = formatHex(i, 4);
			if (list.find(id) == list.end()) {
				return id;
			}
		}

		return "";
	}

	webrtc_id_type get_webrtc_id(
		const peer_map& peers,
		const std::shared_ptr<rtc::WebSocket>& ws
	) {
		for (auto& s : peers) {
			if (s.second.ws == ws) {
				return s.first;
			}
		}

		return "";
	}

	void release_socket(const std::shared_ptr<rtc::WebSocket>& ws) {
		std::scoped_lock lk(active_connections_mutex);
		active_connections.erase(ws);
	}

	bool remove_web_peer(
		peer_map& peers,
		const webrtc_id_type& id
	) {
		if (const auto p = mapped_or_nullptr(peers, id)) {
			LOG("Erasing peer: %x", id);

			release_socket(p->ws);

			peers.erase(id);
			dirty = true;

			return true;
		}

		LOG("remove_web_peer: Peer '%x' was not found in the map.", id);
		return false;
	}

	void remove_web_peer(
		peer_map& peers,
		const std::shared_ptr<rtc::WebSocket>& ws
	) {
		if (!remove_web_peer(peers, get_webrtc_id(peers, ws))) {
			release_socket(ws);
		}
	}

	webrtc_id_type choose_webrtc_id(
		peer_map& peers,
		std::optional<std::string> maybe_requested
	) {
		if (maybe_requested == std::nullopt) {
			return find_free_id(peers);
		}

		auto requested = *maybe_requested;

		if (requested.size() > 0 && requested[0] == '/') {
			requested.erase(requested.begin());
		}

		if (requested == "random") {
			return find_free_random_guid(peers);
		}

		/*
			Remove : to not overshadow ip:port addresses
		*/

		erase_if(requested, [](const auto c) { return c == ':'; });

		if (requested.empty()) {
			return find_free_id(peers);
		}

		if (found_in(peers, webrtc_id_type(requested))) {
			return find_free_id(peers);
		}

		return requested;
	}

	void register_web_peer(
		peer_map& peers,
		std::shared_ptr<rtc::WebSocket> ws,
		std::optional<std::string> path,
		std::optional<std::string> addr
	) {
		dirty = true;

		const auto chosen_webrtc_id = choose_webrtc_id(peers, path);

		auto& p = peers[chosen_webrtc_id];
		p.ws = ws;
		p.time_last_correct_message = augs::high_precision_secs();

		if (addr) {
			LOG("Web peer connected from %x. Chosen WebRTC id: %x", *addr, chosen_webrtc_id);

			netcode_parse_address(addr->c_str(), &p.ip_informational);

			/* 
				Port is irrelevant.
				Use 0 to not overshadow native clients coming from the same network.
			*/

			p.ip_informational.port = 0;
		}
		else {
			LOG("Web peer connected from unknown address. Chosen WebRTC id: %x", chosen_webrtc_id);
		}

		nlohmann::json message = {
			{"your_id", std::string(chosen_webrtc_id) }
		};

		ws->send(message.dump());
	}

	void relay_message(peer_map& peers, nlohmann::json message, webrtc_id_type source_id) {
		if (message.contains("id") && message["id"].is_string()) {
			std::string dest_id = message["id"];

			{
				std::scoped_lock lk(aliases_mutex);

				if (const auto target = mapped_or_nullptr(webrtc_id_aliases, dest_id)) {
					LOG("Destination Alias '%x' resolved to '%x'", dest_id, *target);
					dest_id = *target;
				}

				if (const auto target = key_or_default(webrtc_id_aliases, source_id); target != "") {
					LOG("Source '%x' aliased to '%x'", source_id, target);
					message["alias"] = target;
				}
			}

			message["id"] = source_id;

			netcode_address_t maybe_native;

			if (NETCODE_OK == netcode_parse_address(dest_id.c_str(), &maybe_native)) {
				/* WebRTC id is IP address */

				LOG("Relaying to native endpoint: %x", dest_id);

				pending_signalling_packet packet;
				yojimbo_random_bytes(reinterpret_cast<uint8_t*>(&packet.guid), sizeof(packet.guid));

				packet.message = message.dump();
				packet.target = maybe_native;

				pending_packets.emplace_back(std::move(packet));
			}
			else {
				LOG("Relaying to peer with id %x", dest_id);

				if (const auto target = mapped_or_nullptr(peers, dest_id)) {
					ensure(target->ws != nullptr);

					target->ws->send(message.dump());
				}
				else {
					LOG("No peer like that!");
				}
			}
		}
	}

	auto make_config(
		const std::string& bind_address,
		const int port,
		const augs::path_type& cert,
		const augs::path_type& pem
	) {
		rtc::WebSocketServer::Configuration config;

		config.port = port;
		config.enableTls = !cert.empty() && !pem.empty();

		if (config.enableTls) {
			LOG("Starting signalling server with SSL."); 
			config.certificatePemFile = augs::string_windows_friendly(cert);
			config.keyPemFile = augs::string_windows_friendly(pem);
		}
		else {
			LOG("Starting signalling server WITHOUT SSL."); 
		}

		config.bindAddress = bind_address; 
		config.maxMessageSize = 1000;

		return config;
	}

	void process_timeouts(double now) {
		std::vector<webrtc_id_type> to_erase;

		on_peers([&](auto& peers) {
			for (auto& p : peers) {
				if (now - p.second.time_last_correct_message >= peer_timeout) {
					to_erase.push_back(p.first);
				}
			}

			for (const auto& t : to_erase) {
				LOG("Timing out peer: %x", t);
				remove_web_peer(peers, t);
			}
		});
	}

	bool try_receive_guid(peer_map& peers, const std::string& dest_id, const uint64_t guid) {
		if (const auto target = mapped_or_nullptr(peers, dest_id)) {
			auto& recvd_messages = target->recvd_messages;

			if (!found_in(recvd_messages, guid)) {
				recvd_messages.emplace(guid);

				return true;
			}
		}

		return false;
	}

public:
	auto get_new_peers_map() {
		peer_map cp;

		{
			on_peers([&](const auto& peers) { 
				cp = peers; 
				dirty = false;
			});
		}

		return cp;
	}

	void relay_message_from_native(
		const masterserver_out::webrtc_signalling_payload& p,
		const std::string& source_webrtc_id
	) {
		const auto guid = augs::bit_cast<uint64_t>(p.message_guid);

		try {
			auto json_message = nlohmann::json::parse(p.message);

			if (json_message.contains("id") && json_message["id"].is_string()) {
				on_peers([&](auto& peers) {
					if (try_receive_guid(peers, std::string(json_message["id"]), guid)) {
						relay_message(peers, json_message, source_webrtc_id);
					}
				});
			}
		}
		catch (...) {

		}
	}

	bool should_reserialize() {
		std::scoped_lock lk(peers_mutex);
		return dirty;
	}

	void process_timeouts_if_its_time() {
		const auto current_time = augs::high_precision_secs();

		if (current_time - when_last_processed_timeouts > 0.1) {
			when_last_processed_timeouts = current_time;

			process_timeouts(current_time);
		}
	}

	template <class P>
	void send_signalling_packets(P send) {
		const auto current_time = augs::high_precision_secs();

		if (current_time - when_last_sent_packets > 0.1) {
			when_last_sent_packets = current_time;

			for (auto& p : pending_packets) {
				masterserver_out::webrtc_signalling_payload payload;
				payload.message = p.message;
				LOG_NVPS(payload.message, p.guid, ::ToString(p.target));
				payload.message_guid = augs::bit_cast<int64_t>(p.guid);

				send(payload, p.target);

				p.times--;
			}
		}

		erase_if(pending_packets, [](const auto& p) { return p.times <= 0; });
	}

	void set_webrtc_id_alias(
		const std::string& from, const std::string& to
	) {
		std::scoped_lock lk(aliases_mutex);
		webrtc_id_aliases[from] = to;
	}

	template <class... Args>
	signalling_server(
		const double peer_timeout,
		Args&&... args
	) :
		peer_timeout(peer_timeout),
		server(make_config(std::forward<Args>(args)...))
	{
		setup_server_callbacks();
	}
};
