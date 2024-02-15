#pragma once

server_assigned_teams::server_assigned_teams(const augs::path_type& assign_teams_json) {
	try {
		auto json = augs::from_json_file<server_assigned_teams_json>(assign_teams_json);
		
		auto read_faction = [&](const auto& entries, const faction_type faction) {
			for (auto& e : entries) {
				id_to_faction[e] = faction;
			}
		};

		read_faction(json.metropolis, faction_type::METROPOLIS);
		read_faction(json.resistance, faction_type::RESISTANCE);
		read_faction(json.atlantis, faction_type::ATLANTIS);
		read_faction(json.spectators, faction_type::SPECTATOR);
	}
	catch (...) {

	}
}

