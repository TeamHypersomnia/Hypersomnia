#pragma once
#include "3rdparty/cpp-httplib/httplib.h"
#include "augs/string/parse_url.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

inline std::string bytes_to_string(const std::vector<std::byte>& bytes) {
	std::string result;
	result.resize(bytes.size());
	std::memcpy(result.data(), bytes.data(), bytes.size());

	return result;
}

namespace discord_webhooks {
	inline std::string find_attachment_url(const std::string& response) {
		rapidjson::Document d;

		if (d.Parse(response.c_str()).HasParseError()) {
			LOG("Couldn't parse JSON response.");
		}
		else {
			if (
				d.IsObject() 
				&& d.HasMember("attachments") 
				&& d["attachments"].IsArray()
				&& d["attachments"].Size() == 1
				&& d["attachments"][0].HasMember("url")
				&& d["attachments"][0]["url"].IsString()
			) {
				const auto& url = d["attachments"][0]["url"].GetString();

				LOG("Parsed JSON response successfully.");

				return url;
			}
			else {
				LOG("Couldn't access the url value.");
			}
		}

		return "";
	}

	inline std::string escaped_nick(const std::string& n) {
		std::string result;

		for (auto c : n) {
			switch(c) {
				case '\\':
				case '*':
				case '_':
				case '~':
				case '>':
				case '`':
					result += '\\';
					break;
				default:
					break;

			}

			result += c;
		}

		return result;
	}

	inline httplib::MultipartFormDataItems form_duel_of_honor(
		const std::string& hook_username,
		const std::string& first_player,
		const std::string& second_player,
		const std::string& duel_picture_url
	) {
		const auto payload = [&]()
		{
			using namespace rapidjson;

			const auto duel_notice = typesafe_sprintf(
				"**%x** and **%x** have agreed to a duel of honor.",
				escaped_nick(first_player),
				escaped_nick(second_player)
			);

			StringBuffer s;
			Writer<StringBuffer> writer(s);

			writer.StartObject();
			writer.Key("username");
			writer.String(hook_username.c_str());
			writer.Key("embeds");
			writer.StartArray();
			{
				writer.StartObject();
				{
					writer.Key("image");
					writer.StartObject();
					{
						writer.Key("url");
						writer.String(duel_picture_url.c_str());
					}
					writer.EndObject();

					writer.Key("description");
					writer.String(duel_notice.c_str());
				}
				writer.EndObject();
			}
			writer.EndArray();
			writer.EndObject();

			return std::string(s.GetString());
		}();

		LOG("Generated payload: %x", payload);

		return {
			{ "payload_json", payload, "", "" }
		};
	}

	inline httplib::MultipartFormDataItems form_player_connected(
		const std::vector<std::byte> avatar,
		const std::string& hook_username,
		const std::string& connected_player,
		std::vector<std::string> other_players,
		const std::string& current_map
	) {
		erase_element(other_players, connected_player);

		const auto payload = [&]()
		{
			using namespace rapidjson;

			const auto connected_notice = typesafe_sprintf("%x connected.", escaped_nick(connected_player));
			const auto embed_color = 52224;
			const auto num_others = other_players.size();

			const auto now_playing_notice = [&]() {
				if (num_others == 0) {
					return typesafe_sprintf("Now playing ``%x``.", current_map);
				}

				if (num_others == 1) {
					return typesafe_sprintf("Now playing ``%x`` with **%x**.", current_map, escaped_nick(other_players[0]));
				}

				return typesafe_sprintf("Now playing ``%x`` with:", current_map);
			}();
				
			std::string footer_content;

			if (num_others > 1) {
				for (const auto& p : other_players) {
					footer_content += p + "\n";
				}

				if (footer_content.size() > 0) {
					footer_content.pop_back();
				}
			}

			StringBuffer s;
			Writer<StringBuffer> writer(s);

			writer.StartObject();
			writer.Key("username");
			writer.String(hook_username.c_str());
			writer.Key("embeds");
			writer.StartArray();
			{
				writer.StartObject();
				{
					writer.Key("color");
					writer.Uint(embed_color);

					writer.Key("title");
					writer.String(connected_notice.c_str());

					writer.Key("description");
					writer.String(now_playing_notice.c_str());

					if (!footer_content.empty()) {
						writer.Key("footer");
						writer.StartObject();
						{
							writer.Key("text");
							writer.String(footer_content.c_str());
						}
						writer.EndObject();
					}
				}
				writer.EndObject();
			}
			writer.EndArray();
			writer.EndObject();

			return std::string(s.GetString());
		}();

		LOG("Generated payload: %x", payload);

		if (avatar.empty()) {
			return {
				{ "payload_json", payload, "", "" }
			};
		}

		return {
			{ "payload_json", payload, "", "" },
			{ "file1", bytes_to_string(avatar), "av.png", "application/octet-stream" }
		};
	}

}
